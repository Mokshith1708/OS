#include "include/kmalloc.h"
#include <stddef.h>
#include <stdint.h> // For intptr_t
#include <string.h> // For memcpy, memset

// Define a simple block header for our allocator
typedef struct block_header {
    size_t size;
    struct block_header *next;
    int free;
} block_header_t;

// Pointer to the first block in our heap list
static block_header_t *head = NULL;

// Helper function to request more memory from the system
static block_header_t *request_more_memory(size_t size) {
    block_header_t *block;
    // Request memory from _sbrk, adding space for the header
    block = (block_header_t *)_sbrk(size + sizeof(block_header_t));
    if (block == (void *)-1) {
        return NULL; // _sbrk failed
    }
    block->size = size;
    block->next = NULL;
    block->free = 1;
    return block;
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    block_header_t *current;
    block_header_t *last = NULL;

    // Align size to a multiple of 4 bytes for Cortex-M0
    size = (size + 3) & ~3;

    if (head == NULL) {
        // First allocation
        head = request_more_memory(size);
        if (head == NULL) {
            return NULL;
        }
        head->free = 0;
        return (void *)(head + 1); // Return pointer past the header
    }

    current = head;
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Found a free block that is large enough
            if (current->size > size + sizeof(block_header_t)) {
                // Split the block
                block_header_t *new_block = (block_header_t *)((char *)(current + 1) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->next = current->next;
                new_block->free = 1;

                current->size = size;
                current->next = new_block;
            }
            current->free = 0;
            return (void *)(current + 1);
        }
        last = current;
        current = current->next;
    }

    // No suitable block found, request more memory
    block_header_t *new_block = request_more_memory(size);
    if (new_block == NULL) {
        return NULL;
    }
    new_block->free = 0;
    last->next = new_block;
    return (void *)(new_block + 1);
}

void kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    // Get the block header from the pointer
    block_header_t *block = (block_header_t *)ptr - 1;
    block->free = 1;

    // TODO: Implement merging of free blocks for better memory utilization
    // For now, just mark as free.
}
