#include <stdint.h>
#include "hal/hal_console.h"
#include "proc.h"

// Symbols from linker script
extern char __ram_start__, __ram_end__;
extern char __app_ram_start__, __app_ram_end__;

// Calculate regions
#define RAM_BASE      ((uintptr_t)&__ram_start__)
#define RAM_END       ((uintptr_t)&__ram_end__)
#define RAM_SIZE      (RAM_END - RAM_BASE)

#define USER_RAM_BASE ((uintptr_t)&__app_ram_start__)
#define USER_RAM_END  ((uintptr_t)&__app_ram_end__)
#define USER_RAM_SIZE (USER_RAM_END - USER_RAM_BASE)

// --- Small helpers for printing numbers without libc ---

static void print_hex(uintptr_t val) {
  char buf[11];
  const char hexchars[] = "0123456789ABCDEF";
  buf[0] = '0';
  buf[1] = 'x';
  for (int i = 0; i < 8; i++) {
    buf[9 - i] = hexchars[val & 0xF];
    val >>= 4;
  }
  buf[10] = 0;
  hal_console_puts(buf);
}

static void print_dec(uintptr_t val) {
  char buf[20];
  int i = 19;
  buf[i--] = 0;
  if (val == 0) buf[i--] = '0';
  while (val > 0 && i >= 0) {
    buf[i--] = '0' + (val % 10);
    val /= 10;
  }
  hal_console_puts(&buf[i+1]);
}

// --- Shell helpers ---

static int readline(char *buf, int max) {
  int n = 0;
  while (n < max - 1) {
    int c = hal_console_getchar();
    if (c == '\r' || c == '\n') {
      hal_console_puts("\r\n");
      break;
    } else if (c == 127 || c == 8) { // backspace
      if (n > 0) { n--; hal_console_puts("\b \b"); }
    } else if (c >= 32 && c < 127) {
      buf[n++] = (char)c;
      hal_console_putc((char)c);
    }
  }
  buf[n] = 0;
  return n;
}

static int streq(const char *a, const char *b) {
  while (*a && *a == *b) { a++; b++; }
  return (*a == 0 && *b == 0);
}

static int starts(const char *s, const char *pfx) {
  while (*pfx) { if (*s++ != *pfx++) return 0; }
  return 1;
}

// --- Commands ---

static void cmd_help(void) {
  hal_console_puts("Commands:\r\n");
  hal_console_puts("  help                 - show this help\r\n");
  hal_console_puts("  meminfo              - RAM range and size\r\n");
  hal_console_puts("  run <path>           - load & jump to image\r\n");
  hal_console_puts("  save <path>          - snapshot RAM to file\r\n");
  hal_console_puts("  load <path>          - alias of run\r\n");
  hal_console_puts("  exit                 - exit shell\r\n");
}

static void cmd_meminfo(void) {
  hal_console_puts("Kernel + User RAM Info:\r\n");
  hal_console_puts("  RAM_BASE = "); print_hex(RAM_BASE); hal_console_puts("\r\n");
  hal_console_puts("  RAM_END  = "); print_hex(RAM_END);  hal_console_puts("\r\n");
  hal_console_puts("  RAM_SIZE = "); print_dec(RAM_SIZE); hal_console_puts(" bytes\r\n");

  hal_console_puts("User Space:\r\n");
  hal_console_puts("  USER_RAM_BASE = "); print_hex(USER_RAM_BASE); hal_console_puts("\r\n");
  hal_console_puts("  USER_RAM_END  = "); print_hex(USER_RAM_END);  hal_console_puts("\r\n");
  hal_console_puts("  USER_RAM_SIZE = "); print_dec(USER_RAM_SIZE); hal_console_puts(" bytes\r\n");
}

// --- Main shell loop ---

void shell_run(void) {
  char line[128];

  hal_console_puts("\r\n[OS] UART shell ready. Type 'help'.\r\n");
  for (;;) {
    hal_console_puts("os> ");
    int n = readline(line, sizeof(line));
    if (n <= 0) continue;

    if (streq(line, "help")) {
      cmd_help();
    } else if (streq(line, "meminfo")) {
      cmd_meminfo();
    } else if (starts(line, "run ")) {
      const char *path = line + 4;
      hal_console_puts("Loading...\r\n");
      start_process(path); // never returns on success
      hal_console_puts("Load failed.\r\n");
    } else if (starts(line, "load ")) {
      const char *path = line + 5;
      hal_console_puts("Loading...\r\n");
      start_process(path);
      hal_console_puts("Load failed.\r\n");
    } else if (starts(line, "save ")) {
      const char *path = line + 5;
      if (swap_out(path) == 0) hal_console_puts("Saved.\r\n");
      else hal_console_puts("Save failed.\r\n");
    } else if (streq(line, "exit")) {
      hal_console_puts("Bye.\r\n");
      break;
    } else {
      hal_console_puts("Unknown. Try 'help'.\r\n");
    }
  }
}
