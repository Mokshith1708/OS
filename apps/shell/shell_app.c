#include <stdio.h>
#include <string.h>

// NOTE: This is a user-space application. It cannot call kernel functions
// or access hardware directly. It must use system calls.

// These function prototypes would normally be in a C library header.
int getchar(void);


static int readline(char *buf, int max) {
  int n = 0;
  while (n < max - 1) {
    int c = getchar();
    if (c < 0) break; // End of stream

    if (c == '\r' || c == '\n') {
      printf("\r\n");
      break;
    } else if (c == 127 || c == 8) { // backspace
      if (n > 0) {
          n--;
          printf("\b \b");
      }
    } else if (c >= 32 && c < 127) {
      buf[n++] = (char)c;
      putchar(c);
    }
  }
  buf[n] = 0;
  return n;
}

static void cmd_help(void) {
  printf("Commands:\r\n");
  printf("  help                 - show this help\r\n");
  printf("  exit                 - exit the shell\r\n");
}

int main(void) {
  char line[128];

  printf("\r\n[MokshithOS Shell v2.0 - User Mode]\r\n");
  for (;;) {
    printf("app> ");
    int n = readline(line, sizeof(line));
    if (n <= 0) continue;

    if (strcmp(line, "help") == 0) {
      cmd_help();
    } else if (strcmp(line, "exit") == 0) {
      printf("Shell exiting.\r\n");
      break;
    } else {
      printf("Unknown command: %s\r\n", line);
    }
  }
  return 0;
}
