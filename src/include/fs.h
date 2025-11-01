#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_MAGIC 0x4D4F4B53 // "MOKS"
#define MAX_FILENAME_LEN 28
#define MAX_INODES 64
#define BLOCK_SIZE 512

// File open flags
#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR      0x0004
#define O_CREAT     0x0008
#define O_TRUNC     0x0010
#define O_APPEND    0x0020

// Filesystem Superblock
// Contains metadata about the entire filesystem.
// There is only one, located at the start of the ramdisk.
typedef struct {
    uint32_t magic;          // Magic number to identify the filesystem
    uint32_t total_blocks;
    uint32_t inode_blocks;   // Number of blocks used by the inode table
    uint32_t data_block_start; // First block number for data
} superblock_t;

// Inode (Index Node)
// Contains metadata about a single file.
typedef struct {
    uint32_t inode_num;      // Inode number
    uint16_t type;           // 0 = unused, 1 = file, 2 = directory
    uint16_t permissions;    // (not used yet)
    uint32_t size;           // Size of the file in bytes
    uint32_t direct_blocks[12]; // Pointers to data blocks
    // Indirect block pointers would go here for larger files
} inode_t;

// Directory Entry
// A directory is just a file containing a list of these entries.
typedef struct {
    uint32_t inode_num;                // Inode number for this entry
    char name[MAX_FILENAME_LEN]; // Filename
} dirent_t;

typedef struct {
    inode_t* inode;    // Pointer to the inode of the open file
    uint32_t offset;   // Current read/write offset
    uint32_t flags;    // Open flags (e.g., O_RDONLY, O_WRONLY)
} file_t;

void fs_init(void);
inode_t* fs_find_file(const char* name);
inode_t* fs_lookup(const char* path);

#endif // FS_H
