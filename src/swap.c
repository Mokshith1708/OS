#include <stdint.h>
#include "proc.h"
#include "semihost.h"
#include "hal/hal_console.h"

/* App RAM window (from linker) */
extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];
#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

int swap_out(const char *path)
{
    proc_img_hdr_t hdr;
    hdr.ram_size = (uint32_t)APP_SIZE;
    hdr.entry_pc = 0;
    hdr.initial_sp = (uint32_t)APP_END; /* default */
    hdr.flags = 0;

    int fd = sh_open(path, 4);
    if (fd < 0)
    {
        hal_console_puts("swap_out: open fail\r\n");
        return -1;
    }

    if (sh_write(fd, &hdr, sizeof(hdr)) != 0)
    {
        sh_close(fd);
        return -2;
    }
    if (sh_write(fd, (void *)APP_BASE, hdr.ram_size) != 0)
    {
        sh_close(fd);
        return -3;
    }

    sh_close(fd);
    return 0;
}

int swap_in(const char *path, uint32_t *entry, uint32_t *sp)
{
    proc_img_hdr_t hdr;

    hal_console_puts("swap_in: opening '");
    hal_console_puts(path);
    hal_console_puts("'\r\n");
    int fd = sh_open(path, 0);
    if (fd < 0)
    {
        hal_console_puts("swap_in: sh_open FAILED\r\n");
        return -1;
    }
    hal_console_puts("swap_in: fd=");
    hal_console_put_int(fd);
    hal_console_puts("\r\n");

    int flen = sh_flen(fd);
    hal_console_puts("swap_in: flen=");
    hal_console_put_int(flen);
    hal_console_puts("\r\n");

    int remain = sh_read(fd, &hdr, sizeof(hdr));
    hal_console_puts("swap_in: header remain=");
    hal_console_put_int(remain);
    hal_console_puts("\r\n");
    if (remain)
    {
        sh_close(fd);
        return -2;
    }

    hal_console_puts("hdr.ram_size=");
    hal_console_put_int(hdr.ram_size);
    hal_console_puts(" entry=");
    hal_console_put_hex(hdr.entry_pc);
    hal_console_puts(" sp=");
    hal_console_put_hex(hdr.initial_sp);
    hal_console_puts("\r\n");

    if (hdr.ram_size > APP_SIZE)
    {
        sh_close(fd);
        hal_console_puts("swap_in: too big\r\n");
        return -3;
    }

    remain = sh_read(fd, (void *)APP_BASE, hdr.ram_size);
    hal_console_puts("swap_in: body remain=");
    hal_console_put_int(remain);
    hal_console_puts("\r\n");
    sh_close(fd);
    if (remain)
        return -4;

    if (entry)
    {
        *entry = hdr.entry_pc;
    }
    if (sp)
    {
        *sp = hdr.initial_sp ? hdr.initial_sp : (uint32_t)APP_END;
    }

    hal_console_puts("swap_in: success\r\n");
    return 0;
}
