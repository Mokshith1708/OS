#include "include/stdio.h"
#include "include/string.h"
#include <stdarg.h>

// Syscall prototypes
int _write(int file, char *ptr, int len);
int _read(int file, char *ptr, int len);

int putchar(int c) {
    char ch = (char)c;
    return _write(1, &ch, 1);
}

int puts(const char *s) {
    int len = strlen(s);
    _write(1, (char*)s, len);
    return _write(1, "\n", 1);
}

int getchar(void) {
    char c;
    int ret = _read(0, &c, 1);
    if (ret < 1) return EOF;
    return (int)c;
}

// Simple integer to string conversion
static void itoa(int n, char *s, int base) {
    int i = 0;
    int is_negative = 0;
    if (n == 0) {
        s[i++] = '0';
        s[i] = '\0';
        return;
    }
    if (n < 0 && base == 10) {
        is_negative = 1;
        n = -n;
    }
    while (n != 0) {
        int rem = n % base;
        s[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        n = n / base;
    }
    if (is_negative) s[i++] = '-';
    s[i] = '\0';
    // Reverse the string
    int start = 0, end = i - 1;
    while (start < end) {
        char temp = s[start];
        s[start] = s[end];
        s[end] = temp;
        start++;
        end--;
    }
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[32];

    for (const char *p = format; *p != '\0'; p++) {
        if (*p != '%') {
            putchar(*p);
            continue;
        }
        p++; // Move past '%'
        switch (*p) {
            case 'c':
                putchar(va_arg(args, int));
                break;
            case 's':
                puts(va_arg(args, char *));
                break;
            case 'd':
                itoa(va_arg(args, int), buffer, 10);
                puts(buffer);
                break;
            case 'x':
                itoa(va_arg(args, int), buffer, 16);
                puts(buffer);
                break;
            case '%':
                putchar('%');
                break;
            default:
                putchar('%');
                putchar(*p);
                break;
        }
    }
    va_end(args);
    return 0;
}
