#include <stdio.h>
#include <unistd.h>

// The main function. It does NOT initialize any hardware.
int main(int argc, char *argv[]) {
    printf("Test Write App Started!\n");

    printf("Received %d arguments:\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d]: %s\n", i, argv[i]);
    }

    const char *msg = "This is a test message written to stdout.\n";
    printf("Now, writing a message with the write() syscall:\n%s", msg);
    write(1, msg, 43); // Manually provide length

    printf("Test Write App Finished.\n");

    return 0;
}