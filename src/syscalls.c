// src/syscalls.c
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "include/proc.h" // For schedule()
#include "include/hal_console.h"
#include <stdlib.h>
#include "include/systick.h"
#include <string.h>

extern pcb_t *current_process;
extern uint8_t __ramdisk_start[];
extern pcb_t pcb_table[MAX_PROCESSES];

// Syscall numbers - MUST match apps/libuser/user_syscalls.c
#define SYS_EXIT  1
#define SYS_YIELD 2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_EXEC  5
#define SYS_EXEC  5
#define SYS_SBRK  6
#define SYS_OPEN  7
#define SYS_CLOSE 8
#define SYS_LSEEK 9
#define SYS_FSTAT 10
#define SYS_SLEEP 11
#define SYS_GETTIMEOFDAY 12
#define SYS_SEND_MSG 13
#define SYS_RECEIVE_MSG 14

#undef errno
extern int errno;

// Forward declarations for kernel-internal syscall implementations
int sys_write(int file, char *ptr, int len);
void* sys_sbrk(intptr_t incr);
int sys_open(const char *name, int flags, int mode);

// sys_exit is called when a program (or the kernel) calls exit().
void sys_exit(int status) {
    hal_console_puts("\n--- Process exited. Cleaning up and reloading shell. ---\n");

    // 1. Free the current process's PCB
    if (current_process) {
        // Close all open files
        for (int i = 0; i < MAX_OPEN_FILES; i++) {
            if (current_process->fd_table[i] != NULL) {
                free(current_process->fd_table[i]);
                current_process->fd_table[i] = NULL;
            }
        }

        current_process->state = PROC_STATE_UNUSED;
        current_process = NULL;
    }

    // 2. Start a new shell process
    start_process("apps/shell/build/shell_app.proc");

    // 3. Schedule the new shell to run. The old context is abandoned.
    schedule();

    // This point should be unreachable
    while(1);
}

int sys_open(const char *name, int flags, int mode) {
    (void)mode; // Mode is not yet implemented

    if (!current_process) {
        errno = EPERM;
        return -1;
    }

    // Find an empty file descriptor slot
    int fd;
    for (fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (current_process->fd_table[fd] == NULL) {
            break;
        }
    }

    if (fd == MAX_OPEN_FILES) {
        errno = EMFILE; // Too many open files
        return -1;
    }

    inode_t* inode = fs_lookup(name);
    if (inode == NULL) {
        // If O_CREAT is set, create the file (not implemented yet)
        errno = ENOENT;
        return -1;
    }

    // Allocate a new file_t structure
    file_t* file = (file_t*)malloc(sizeof(file_t)); // Need a kernel malloc
    if (file == NULL) {
        errno = ENOMEM;
        return -1;
    }

    file->inode = inode;
    file->offset = 0;
    file->flags = flags;

    current_process->fd_table[fd] = file;

    return fd;
}

// The rest of these are stubs for file and process operations.
// We return error codes because our OS does not yet support them.
int sys_close(int file) {
    if (!current_process || file < 0 || file >= MAX_OPEN_FILES || current_process->fd_table[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    // Free the allocated file_t structure
    free(current_process->fd_table[file]);
    current_process->fd_table[file] = NULL;

    return 0;
}
int sys_fstat(int file, struct stat *st) {
    if (!current_process || file < 0 || file >= MAX_OPEN_FILES || current_process->fd_table[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    file_t* open_file = current_process->fd_table[file];
    inode_t* inode = open_file->inode;

    if (inode == NULL) {
        errno = EBADF;
        return -1;
    }

    st->st_mode = (inode->type == 1) ? S_IFREG : S_IFDIR; // 1 for file, 2 for directory
    st->st_size = inode->size;
    st->st_ino = inode->inode_num; // Assuming inode_num is stored in inode_t

    return 0;
}

int sys_sleep(uint32_t milliseconds) {
    if (!current_process) {
        errno = EPERM;
        return -1;
    }

    current_process->state = PROC_STATE_SLEEPING;
    current_process->sleep_ticks = (milliseconds * SYSTICK_HZ) / 1000; // Convert ms to ticks
    schedule();

    return 0;
}
int sys_isatty(int file) { (void)file; return 1; }
int sys_lseek(int file, int ptr, int dir) {
    if (!current_process || file < 0 || file >= MAX_OPEN_FILES || current_process->fd_table[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    file_t* open_file = current_process->fd_table[file];
    inode_t* inode = open_file->inode;

    int new_offset;
    switch (dir) {
        case SEEK_SET:
            new_offset = ptr;
            break;
        case SEEK_CUR:
            new_offset = open_file->offset + ptr;
            break;
        case SEEK_END:
            new_offset = inode->size + ptr;
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    if (new_offset < 0 || new_offset > inode->size) {
        errno = EINVAL;
        return -1;
    }

    open_file->offset = new_offset;
    return new_offset;
}

int sys_read(int file, char *ptr, int len) {
    // We only handle stdin (file descriptor 0) for now
    if (file == 0) {
        int i;
        for (i = 0; i < len; i++) {
            // This is a non-blocking read. If no input, block the process.
            while (!hal_console_input_available()) {
                current_process->state = PROC_STATE_BLOCKED;
                schedule();
            }
            ptr[i] = hal_console_try_getchar();
        }
        return len;
    }

    if (!current_process || file < 0 || file >= MAX_OPEN_FILES || current_process->fd_table[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    file_t* open_file = current_process->fd_table[file];
    if (!(open_file->flags & O_RDONLY)) {
        errno = EACCES; // File not open for reading
        return -1;
    }

    inode_t* inode = open_file->inode;
    if (inode == NULL) {
        errno = EBADF;
        return -1;
    }

    uint32_t bytes_to_read = len;
    if (open_file->offset + bytes_to_read > inode->size) {
        bytes_to_read = inode->size - open_file->offset;
    }

    if (bytes_to_read == 0) {
        return 0; // End of file or nothing to read
    }

    // Read data block by block
    uint32_t bytes_read = 0;
    while (bytes_read < bytes_to_read) {
        uint32_t current_block_idx = (open_file->offset + bytes_read) / BLOCK_SIZE;
        uint32_t block_offset = (open_file->offset + bytes_read) % BLOCK_SIZE;
        uint32_t block_num = inode->direct_blocks[current_block_idx];

        uint8_t* block_ptr = __ramdisk_start + (block_num * BLOCK_SIZE);

        uint32_t copy_len = BLOCK_SIZE - block_offset;
        if (copy_len > (bytes_to_read - bytes_read)) {
            copy_len = bytes_to_read - bytes_read;
        }

        memcpy(ptr + bytes_read, block_ptr + block_offset, copy_len);
        bytes_read += copy_len;
    }

    open_file->offset += bytes_read;
    return bytes_read;
}

int sys_write(int file, char *ptr, int len) {
    // We only handle stdout (file descriptor 1) for now
    if (file == 1) {
        int i;
        for (i = 0; i < len; i++) {
            hal_console_putc(ptr[i]);
        }
        return len;
    }

    if (!current_process || file < 0 || file >= MAX_OPEN_FILES || current_process->fd_table[file] == NULL) {
        errno = EBADF;
        return -1;
    }

    file_t* open_file = current_process->fd_table[file];
    if (!(open_file->flags & (O_WRONLY | O_RDWR))) {
        errno = EACCES; // File not open for writing
        return -1;
    }

    inode_t* inode = open_file->inode;
    if (inode == NULL) {
        errno = EBADF;
        return -1;
    }

    // For simplicity, we only support writing to existing files and not extending them.
    // Also, we don't handle O_APPEND yet.
    uint32_t bytes_to_write = len;
    if (open_file->offset + bytes_to_write > inode->size) {
        bytes_to_write = inode->size - open_file->offset;
    }

    if (bytes_to_write == 0) {
        return 0; // Nothing to write or end of file
    }

    // Write data block by block
    uint32_t bytes_written = 0;
    while (bytes_written < bytes_to_write) {
        uint32_t current_block_idx = (open_file->offset + bytes_written) / BLOCK_SIZE;
        uint32_t block_offset = (open_file->offset + bytes_written) % BLOCK_SIZE;
        uint32_t block_num = inode->direct_blocks[current_block_idx];

        uint8_t* block_ptr = __ramdisk_start + (block_num * BLOCK_SIZE);

        uint32_t copy_len = BLOCK_SIZE - block_offset;
        if (copy_len > (bytes_to_write - bytes_written)) {
            copy_len = bytes_to_write - bytes_written;
        }

        memcpy(block_ptr + block_offset, ptr + bytes_written, copy_len);
        bytes_written += copy_len;
    }

    open_file->offset += bytes_written;
    return bytes_written;
}


int sys_kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int sys_getpid(void) { return 1; }



extern volatile uint32_t tick_count;

int sys_gettimeofday(struct timeval *tv, void *tz) {
    (void)tz; // Timezone is not implemented

    if (tv) {
        tv->tv_sec = tick_count / SYSTICK_HZ;
        tv->tv_usec = (tick_count % SYSTICK_HZ) * (1000000 / SYSTICK_HZ);
    }

    return 0;
}

int sys_send_msg(int pid, char* msg) {
    if (pid < 1 || pid > MAX_PROCESSES) {
        errno = ESRCH; // No such process
        return -1;
    }

    pcb_t* target_pcb = &pcb_table[pid - 1];

    if (target_pcb->state == PROC_STATE_UNUSED) {
        errno = ESRCH;
        return -1;
    }

    if (target_pcb->msg_count >= MAX_MESSAGES) {
        errno = EMSGSIZE; // Message queue full
        return -1;
    }

    // Copy the message into the target process's queue
    // For simplicity, we are just storing the pointer. In a real OS, we would copy the message content.
    target_pcb->msg_queue[target_pcb->msg_write_idx] = msg;
    target_pcb->msg_write_idx = (target_pcb->msg_write_idx + 1) % MAX_MESSAGES;
    target_pcb->msg_count++;

    // If the target process is blocked waiting for a message, wake it up
    if (target_pcb->state == PROC_STATE_BLOCKED) {
        target_pcb->state = PROC_STATE_READY;
    }

    return 0;
}

char* sys_receive_msg(void) {
    if (!current_process) {
        errno = EPERM;
        return NULL;
    }

    if (current_process->msg_count == 0) {
        // Block the process until a message arrives
        current_process->state = PROC_STATE_BLOCKED;
        schedule();

        // After waking up, recheck if a message is available
        if (current_process->msg_count == 0) {
            // This should not happen if the sender correctly woke us up
            errno = EAGAIN; // No message available
            return NULL;
        }
    }

    char* msg = current_process->msg_queue[current_process->msg_read_idx];
    current_process->msg_queue[current_process->msg_read_idx] = NULL; // Clear the slot
    current_process->msg_read_idx = (current_process->msg_read_idx + 1) % MAX_MESSAGES;
    current_process->msg_count--;

    return msg;
}


void svc_handler_c(uint32_t *stack) {
    // 1. Get syscall number from R0, which we set in the wrapper
    int svc_number = stack[0];

    // 2. Get arguments from the process stack
    int r1 = stack[1];
    int r2 = stack[2];
    int r3 = stack[3];

    int ret = -1; // Default return value

    switch (svc_number) {
        case SYS_EXIT: // _exit
            sys_exit(r1);
            break; // sys_exit never returns

        case SYS_YIELD: // yield
            schedule(); // Call the scheduler to switch processes
            ret = 0;
            break;

        case SYS_READ: // _read
            ret = sys_read(r1, (char *)r2, r3);
            break;

        case SYS_WRITE: // _write
            ret = sys_write(r1, (char *)r2, r3);
            break;

        case SYS_EXEC: // exec
            ret = sys_exec((const char *)r1, r2, (char *const *)r3);
            break;

        case SYS_SBRK: // sbrk
            ret = (int)_sbrk(r1);
            break;

        case SYS_OPEN: // open
            ret = sys_open((const char *)r1, r2, r3);
            break;

        case SYS_SLEEP: // sleep
            ret = sys_sleep(r1);
            break;

        case SYS_GETTIMEOFDAY: // gettimeofday
            ret = sys_gettimeofday((struct timeval *)r1, (void *)r2);
            break;

        case SYS_SEND_MSG: // send_msg
            ret = sys_send_msg(r1, (char *)r2);
            break;

        case SYS_RECEIVE_MSG: // receive_msg
            ret = (int)sys_receive_msg();
            break;

        default:
            hal_console_puts("Unknown syscall number: ");
            hal_console_put_int(svc_number);
            hal_console_puts("\n");
            break;
    }

    // 4. Store return value in stack frame
    stack[0] = ret;
}

__attribute__((naked))
void SVC_Handler(void) {
    __asm__ volatile (
        // Determine which stack pointer (MSP or PSP) was active
        "mrs r0, psp\n" // Assume PSP is active
        "mov r1, sp\n"  // Get MSP
        "cmp r0, r1\n"  // Compare PSP and MSP
        "bne .L_psp_is_active\n" // If not equal, PSP was active
        "mrs r0, msp\n" // If equal, MSP was active
    ".L_psp_is_active:\n"
        "b svc_handler_c\n"
    );
}