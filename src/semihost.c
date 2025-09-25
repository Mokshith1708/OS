#include "ff.h"  // FatFs header
#include "semihost.h"

// Maintain a simple file descriptor table mapping int fd to FIL objects for demo
#define MAX_FILES 10
static FIL file_table[MAX_FILES];
static int file_open_flags[MAX_FILES] = {0};

static int alloc_fd() {
    for(int i = 0; i < MAX_FILES; i++) {
        if (!file_open_flags[i]) {
            file_open_flags[i] = 1;
            return i;
        }
    }
    return -1;  // No free slot
}

static void free_fd(int fd) {
    if(fd >= 0 && fd < MAX_FILES) {
        file_open_flags[fd] = 0;
    }
}

int sh_open(const char *path, uint32_t mode) {
    FIL *fil;
    int fd = alloc_fd();
    if (fd == -1)
        return -1;

    fil = &file_table[fd];
    BYTE fat_mode = 0;
    if (mode & 1) fat_mode |= FA_READ;
    if (mode & 2) fat_mode |= FA_WRITE;

    FRESULT res = f_open(fil, path, fat_mode);
    if (res != FR_OK) {
        free_fd(fd);
        xil_printf("File does not exist");
        return -1;
    }
    return fd;
}

int sh_close(int fd) {
    if(fd < 0 || fd >= MAX_FILES || !file_open_flags[fd])
        return -1;
    f_close(&file_table[fd]);
    free_fd(fd);
    return 0;
}

int sh_read(int fd, void *buf, uint32_t len) {
    if(fd < 0 || fd >= MAX_FILES || !file_open_flags[fd])
        return -1;
    UINT bytesRead;
    FRESULT res = f_read(&file_table[fd], buf, len, &bytesRead);
    if (res != FR_OK)
        return -1;
    return bytesRead;  // Returns number of bytes read
}

int sh_write(int fd, const void *buf, uint32_t len) {
    if(fd < 0 || fd >= MAX_FILES || !file_open_flags[fd])
        return -1;
    UINT bytesWritten;
    FRESULT res = f_write(&file_table[fd], buf, len, &bytesWritten);
    if (res != FR_OK)
        return -1;
    return bytesWritten;  // Number of bytes written
}

int sh_flen(int fd) {
    if(fd < 0 || fd >= MAX_FILES || !file_open_flags[fd])
        return -1;
    FSIZE_t sz = f_size(&file_table[fd]);
    return (int)sz;
}

void sh_write0(const char *s) {
    // Simple debug output to UART
    while (*s) {
        xil_printf("%c", *s++);
    }
}

void sh_exit(int code) {
    xil_printf("Exit with code %d\n", code);
    while (1); // Hang or reset
}
