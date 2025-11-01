#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

// It's a host tool, so it includes the definition from the OS source
#include "../src/include/fs.h"

#define RAMDISK_SIZE (1024 * 1024) // 1MB Ramdisk

static uint8_t ramdisk[RAMDISK_SIZE];

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <output_image> <file1> [file2] ...\n", argv[0]);
        return 1;
    }

    const char *output_image = argv[1];
    int num_files = argc - 2;

    if (num_files + 1 > MAX_INODES) { // +1 for root dir
        fprintf(stderr, "Error: Too many files for available inodes.\n");
        return 1;
    }

    // 1. Setup Superblock
    superblock_t *sb = (superblock_t *)ramdisk;
    sb->magic = FS_MAGIC;
    sb->total_blocks = RAMDISK_SIZE / BLOCK_SIZE;
    sb->inode_blocks = (sizeof(inode_t) * MAX_INODES) / BLOCK_SIZE;
    sb->data_block_start = 1 + sb->inode_blocks; // Superblock is block 0

    // 2. Setup Inode Table
    inode_t *inodes = (inode_t *)(ramdisk + BLOCK_SIZE);
    memset(inodes, 0, sizeof(inode_t) * MAX_INODES);

    // Inode 0 is reserved. Inode 1 is the root directory.
    inodes[1].type = 2; // Directory
    inodes[1].size = 0; // Will be updated as we add entries
    inodes[1].direct_blocks[0] = sb->data_block_start; // First data block

    // 3. Setup Root Directory Data Block
    dirent_t *root_dir = (dirent_t *)(ramdisk + (sb->data_block_start * BLOCK_SIZE));
    memset(root_dir, 0, BLOCK_SIZE);

    // Add '.' and '..' entries
    strcpy(root_dir[0].name, ".");
    root_dir[0].inode_num = 1;
    strcpy(root_dir[1].name, "..");
    root_dir[1].inode_num = 1;
    inodes[1].size = sizeof(dirent_t) * 2;

    int current_inode = 2;
    int current_block = sb->data_block_start + 1;
    int root_dir_entries = 2;

    // 4. Add user files
    for (int i = 0; i < num_files; i++) {
        const char *filepath = argv[i + 2];
        FILE *f = fopen(filepath, "rb");
        if (!f) {
            perror("Failed to open file");
            return 1;
        }

        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (file_size > (12 * BLOCK_SIZE)) {
            fprintf(stderr, "Error: File %s is too large (max 12 blocks).\n", filepath);
            fclose(f);
            return 1;
        }

        // Create inode for this file
        inodes[current_inode].inode_num = current_inode;
        inodes[current_inode].type = 1; // File
        inodes[current_inode].size = file_size;

        // Read file and write to data blocks
        char *file_buffer = (char *)malloc(file_size);
        fread(file_buffer, file_size, 1, f);
        fclose(f);

        int blocks_needed = (file_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        for (int j = 0; j < blocks_needed; j++) {
            memcpy(ramdisk + (current_block * BLOCK_SIZE), file_buffer + (j * BLOCK_SIZE), BLOCK_SIZE);
            inodes[current_inode].direct_blocks[j] = current_block;
            current_block++;
        }
        free(file_buffer);

        // Add directory entry in root
        dirent_t *dirent = &root_dir[root_dir_entries];
        strncpy(dirent->name, strrchr(filepath, '/') ? strrchr(filepath, '/') + 1 : filepath, MAX_FILENAME_LEN - 1);
        dirent->inode_num = current_inode;
        inodes[1].size += sizeof(dirent_t);

        printf("Added '%s', size %ld, inode %d, blocks %d\n", dirent->name, file_size, current_inode, blocks_needed);

        root_dir_entries++;
        current_inode++;
    }

    // 5. Write the whole ramdisk to the output file
    FILE *out_f = fopen(output_image, "wb");
    if (!out_f) {
        perror("Failed to create output image");
        return 1;
    }
    fwrite(ramdisk, RAMDISK_SIZE, 1, out_f);
    fclose(out_f);

    printf("\nSuccessfully created ramdisk image '%s'\n", output_image);

    return 0;
}
