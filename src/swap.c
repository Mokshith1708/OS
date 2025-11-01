#include <stdint.h>
#include "include/proc.h"
#include "include/fs.h"
#include "include/hal_console.h"
#include <string.h>

/* App RAM window (from linker) */
extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];
#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

extern uint8_t __ramdisk_start[];

int swap_in(inode_t* inode, uint32_t *entry, uint32_t *sp, uint32_t *img_size)
{
    proc_img_hdr_t* hdr = (proc_img_hdr_t*)(__ramdisk_start + (inode->direct_blocks[0] * BLOCK_SIZE));

    if (hdr->ram_size > APP_SIZE)
    {
        hal_console_puts("swap_in: too big\r\n");
        return -3;
    }

    uint8_t* app_data = (uint8_t*)hdr + sizeof(proc_img_hdr_t);
    memcpy((void*)APP_BASE, app_data, hdr->ram_size);

    if (entry)
    {
        *entry = hdr->entry_pc;
    }
    if (sp)
    {
        *sp = hdr->initial_sp ? hdr->initial_sp : (uint32_t)APP_END;
    }
    if (img_size)
    {
        *img_size = hdr->ram_size;
    }

    hal_console_puts("swap_in: success\r\n");
    return 0;
}
