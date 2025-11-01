#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "unistd.h"

#define MAX_ARGS 8

// NOTE: This is a user-space application. It cannot call kernel functions
// or access hardware directly. It must use system calls.

// These function prototypes would normally be in a C library header.
int getchar(void);

static int parse_args(char *line, char **argv) {
    int argc = 0;
    char *next_arg = line;

    while (argc < MAX_ARGS) {
        // Find the beginning of the next argument
        while (*next_arg == ' ') next_arg++;
        if (*next_arg == '\0') break;

        // This is the next argument
        argv[argc++] = next_arg;

        // Find the end of this argument
        while (*next_arg != ' ' && *next_arg != '\0') {
            next_arg++;
        }

        // If we are not at the end of the line, null-terminate the argument
        if (*next_arg != '\0') {
            *next_arg++ = '\0';
        }
    }
    return argc;
}

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
  printf("  run <app> [args...]  - run an application with arguments\r\n");
  printf("  exit                 - exit the shell\r\n");
}

int main(int argc, char *argv[]) {
  char line[128];
  char *args[MAX_ARGS];

  printf("\r\n[MokshithOS Shell v4.0 - User Mode]\r\n");

  // Print own arguments (for debugging)
  printf("Shell started with %d arguments:\r\n", argc);
  for (int i = 0; i < argc; i++) {
      printf("  argv[%d]: %s\r\n", i, argv[i]);
  }

  for (;;) {
    printf("app> ");
    int n = readline(line, sizeof(line));
    if (n <= 0) continue;

    int n_args = parse_args(line, args);
    if (n_args == 0) continue;

    if (strcmp(args[0], "help") == 0) {
      cmd_help();
    } else if (strcmp(args[0], "exit") == 0) {
      printf("Shell exiting.\r\n");
      break;
    } else if (strcmp(args[0], "run") == 0) {
      if (n_args < 2) {
        printf("Usage: run <app> [args...]\r\n");
        continue;
      }
      char *app_name = args[1];
      char path[128];
      strcpy(path, "apps/");
      strcat(path, app_name);
      strcat(path, "/build/");
      strcat(path, app_name);
      strcat(path, ".proc");
      
      printf("Executing %s...\r\n", path);
      int ret = exec(path, n_args - 1, &args[1]);
      // exec only returns if it fails
      printf("Error: Failed to execute '%s' (ret=%d)\r\n", path, ret);
    } else {
      printf("Unknown command: %s\r\n", args[0]);
    }
  }
  return 0;
}
