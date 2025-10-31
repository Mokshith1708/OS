#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// The main function. It does NOT initialize any hardware.
int main(int argc, char *argv[]) {
    printf("Hello App Started!\n");

    printf("Received %d arguments:\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d]: %s\n", i, argv[i]);
    }

    printf("\n--- Malloc Test ---\n");
    char *buf = malloc(128);
    if (buf) {
        printf("malloc(128) returned: 0x%x\n", (uint32_t)buf);
        strcpy(buf, "This string is on the heap!");
        printf("Buffer content: %s\n", buf);
        free(buf);
        printf("Memory freed.\n");
    } else {
        printf("malloc failed!\n");
    }

    printf("\nHello App Finished.\n");
    return 0;
}