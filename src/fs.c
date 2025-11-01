#include "include/fs.h"
#include "include/hal_console.h"
#include <string.h>
#include <stddef.h>

extern uint8_t __ramdisk_start[];
extern uint8_t __ramdisk_end[];

static superblock_t* fs_superblock = NULL;

void fs_init() {
    hal_console_puts("Initializing filesystem...\n");

    if (&__ramdisk_start[0] == &__ramdisk_end[0]) {
        hal_console_puts("Ramdisk not found.\n");
        return;
    }

    fs_superblock = (superblock_t*)__ramdisk_start;

    if (fs_superblock->magic != FS_MAGIC) {
        hal_console_puts("Invalid filesystem magic.\n");
        fs_superblock = NULL;
        return;
    }

    hal_console_puts("Filesystem mounted successfully.\n");
}

inode_t* fs_lookup(const char* path) {
    if (!fs_superblock) {
        return NULL;
    }

    // Start from root directory
    inode_t* current_dir_inode = (inode_t*)(__ramdisk_start + BLOCK_SIZE + (sizeof(inode_t) * 1)); // Inode 1 is root

    // Skip leading slash if present
    if (*path == '/') {
        path++;
    }

    char path_copy[MAX_FILENAME_LEN + 1];
    char* token;

    strcpy(path_copy, path);

    token = strtok(path_copy, "/");

    while (token != NULL) {
        if (current_dir_inode->type != 2) { // Not a directory
            return NULL;
        }

        dirent_t* dir_entries = (dirent_t*)(__ramdisk_start + (current_dir_inode->direct_blocks[0] * BLOCK_SIZE));
        int num_entries = current_dir_inode->size / sizeof(dirent_t);

        inode_t* found_inode = NULL;
        for (int i = 0; i < num_entries; i++) {
            if (strcmp(dir_entries[i].name, token) == 0) {
                found_inode = (inode_t*)(__ramdisk_start + BLOCK_SIZE + (sizeof(inode_t) * dir_entries[i].inode_num));
                break;
            }
        }

        if (found_inode == NULL) {
            return NULL; // Not found
        }

        current_dir_inode = found_inode;
        token = strtok(NULL, "/");
    }

    return current_dir_inode;
}

inode_t* fs_find_file(const char* name) {
    return fs_lookup(name);
}

