#include "driver.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h> // for memcpy

// cursor tracking
static int cursor_x = 0;
static int cursor_y = 0;

// framebuffer pointer (set by init)
static uint8_t *framebuffer = 0;

// initialize console
void console_init(uint8_t *fb) {
    framebuffer = fb;
    cursor_x = 0;
    cursor_y = 0;

    // Clear screen
    memset(framebuffer, 0x00, SCREEN_H_PIXELS * SCREEN_V_PIXELS * 3); // Each pixel is 3 bytes (RGB); Each colour takes 1 byte (0-255)
}

// scroll the framebuffer up by one line (CHAR_H pixels)
static void console_scroll(void) {
    if (!framebuffer) return;

    int row_bytes = SCREEN_H_PIXELS * CHAR_H * 3;
    int scroll_bytes = (SCREEN_V_PIXELS - CHAR_H) * SCREEN_H_PIXELS * 3;

    // Move framebuffer up by CHAR_H rows
    memmove(framebuffer, framebuffer + row_bytes, scroll_bytes);

    // Clear the new bottom line
    memset(framebuffer + scroll_bytes, 0x00, row_bytes);

    cursor_y = ROWS - 1; // set cursor to last row
}

// draw a character at the current cursor
void console_putc(char c) {
    if (!framebuffer) return;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        if (c < 32 || c > 126) return; // ignore non-printable

        int ch = c - 32;
        int row_offset = cursor_y * CHAR_H;
        int col_offset = cursor_x * CHAR_W;

        for (int row = 0; row < 8; row++) { // raw font is always 8 rows
            uint8_t bits = font8x8_basic[ch][row];
            for (int col = 0; col < 8; col++) { // raw font is always 8 cols
                // uint8_t val = (bits & (1 << (7 - col))) ? 0xFF : 0x00; // bit 0 => leftmost pixel
                uint8_t val = (bits & (1 << col)) ? 0xFF : 0x00; // // bit 7 => leftmost pixel; check this once

                // expand each font pixel into SCALE×SCALE block
                for (int sy = 0; sy < SCALE; sy++) {
                    for (int sx = 0; sx < SCALE; sx++) {
                        int pixel_x = col_offset + col * SCALE + sx;
                        int pixel_y = row_offset + row * SCALE + sy;

                        if (pixel_x >= SCREEN_H_PIXELS || pixel_y >= SCREEN_V_PIXELS)
                            continue;

                        int idx = (pixel_y * SCREEN_H_PIXELS + pixel_x) * 3;
                        framebuffer[idx + 0] = val;
                        framebuffer[idx + 1] = val;
                        framebuffer[idx + 2] = val;
                    }
                }
            }
        }

        cursor_x++;
        if (cursor_x >= COLS) { // COLS must already be SCREEN_H_PIXELS / CHAR_W
            cursor_x = 0; 
            cursor_y++;
        }
    }

    if (cursor_y >= ROWS) { // ROWS must already be SCREEN_V_PIXELS / CHAR_H
        console_scroll();
    }

    // Optional: flush cache if using VDMA
    // Xil_DCacheFlushRange((INTPTR)framebuffer, SCREEN_H_PIXELS * SCREEN_V_PIXELS * 3);
}

// print string
void console_puts(const char *s) {
    while (*s) console_putc(*s++);
}

// print integer decimal
void console_put_int(int n) {
    char buf[12];
    int i = 0, neg = 0;
    if (n < 0) { neg = 1; n = -n; }
    do { buf[i++] = '0' + (n % 10); n /= 10; } while(n);
    if (neg) buf[i++] = '-';
    while (i--) console_putc(buf[i]);
}

// print integer hex
void console_put_hex(uint32_t n) {
    char buf[9];
    for (int i = 0; i < 8; i++) {
        int val = (n >> ((7-i)*4)) & 0xF;
        buf[i] = val < 10 ? '0'+val : 'A'+val-10;
    }
    buf[8] = 0;
    console_puts(buf);
}

// blocking getchar (placeholder)
int console_getchar(void) {
    return 0;
}