#pragma once

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

void _exit(int status);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);
void yield(void);
