#include <unistd.h>

size_t strlen(const char *s) {
    size_t i = 0;
    while (s[i]) i++;
    return i;
}

// The main function. It does NOT initialize any hardware.
int main(void) {
    const char *msg = "Hello from test_write!\n";
    write(1, msg, strlen(msg));

    // Loop forever so we know the app didn't crash
    for (;;) {}
    return 0;
}