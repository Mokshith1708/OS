#include <stdint.h>
#include "hal/hal_console.h"
#include "proc.h"

extern char __ram_start__, __ram_end__;
#define RAM_BASE ((uintptr_t)&__ram_start__)
#define RAM_END  ((uintptr_t)&__ram_end__)
#define RAM_SIZE (RAM_END - RAM_BASE)

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

static void cmd_help(void) {
  hal_console_puts("Commands:\r\n");
  hal_console_puts("  help                 - show this help\r\n");
  hal_console_puts("  meminfo              - RAM range and size\r\n");
  hal_console_puts("  run <path>           - load & jump to image\r\n");
  hal_console_puts("  save <path>          - snapshot RAM to file\r\n");
  hal_console_puts("  load <path>          - alias of run\r\n");
}

static void cmd_meminfo(void) {
  char buf[96];
  // Simple print without sprintf
  hal_console_puts("RAM_BASE=0x20000000\r\n"); // matches lm3s linker map
  hal_console_puts("RAM_SIZE=65536 bytes\r\n");
  (void)buf;
}

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
