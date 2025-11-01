// A simple malloc implementation based on K&R C.
#include <stdint.h>
#include "include/unistd.h"

// Block header
typedef long Align;

union header {
    struct {
        union header *ptr; // Next free block
        uint32_t size;     // Size of this block
    } s;
    Align x; // Force alignment of blocks
};

typedef union header Header;

static Header base;          // Empty list to get started
static Header *freep = NULL; // Start of free list

#define MIN_ALLOC_SIZE 1024 // Minimum units to request from sbrk

// Add a block to the free list
void free(void *ap) {
    Header *bp, *p;

    if (ap == NULL) return;

    bp = (Header *)ap - 1; // Point to block header

    // Find correct place in free list to insert the block
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
            break; // Freed block at start or end of arena
        }
    }

    // Coalesce with next block if adjacent
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    // Coalesce with previous block if adjacent
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }

    freep = p;
}

// Get more memory from the kernel
static Header *morecore(uint32_t nu) {
    char *cp;
    Header *up;

    if (nu < MIN_ALLOC_SIZE) {
        nu = MIN_ALLOC_SIZE;
    }

    cp = _sbrk(nu * sizeof(Header));
    if (cp == (char *)-1) { // No space at all
        return NULL;
    }

    up = (Header *)cp;
    up->s.size = nu;
    free((void *)(up + 1));
    return freep;
}

void *malloc(uint32_t nbytes) {
    Header *p, *prevp;
    uint32_t nunits;

    if (nbytes == 0) return NULL;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    if ((prevp = freep) == NULL) { // No free list yet
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { // Big enough
            if (p->s.size == nunits) { // Exactly
                prevp->s.ptr = p->s.ptr;
            } else { // Allocate tail end
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1);
        }
        if (p == freep) { // Wrapped around free list
            if ((p = morecore(nunits)) == NULL) {
                return NULL; // None left
            }
        }
    }
}
