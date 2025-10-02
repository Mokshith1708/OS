#include "driver.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h> // for memcpy

// cursor tracking
static int cursor_x = 0;
static int cursor_y = 0;

// framebuffer pointer (set by init)
static volatile uint8_t *framebuffer = 0;

// initialize console
void console_init(uint8_t *fb) {
    framebuffer = fb;
    cursor_x = 0;
    cursor_y = 0;

    // Clear screen
    memset(framebuffer, 0x00, SCREEN_H_PIXELS * SCREEN_V_PIXELS * 3);
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
    } if (c == '\b') {
    	// clear :)
        cursor_x--;
        int row_offset = cursor_y * CHAR_H;
        int col_offset = cursor_x * CHAR_W;
    	for (int row = 0; row < CHAR_H; row++){
        	for (int col = 0;col < CHAR_W; col++){
        		int pixel_x = col_offset + col;
        		int pixel_y = row_offset + row;
        		if (pixel_x >= SCREEN_H_PIXELS || pixel_y >= SCREEN_V_PIXELS)
        			continue;
        		int idx = (pixel_y * SCREEN_H_PIXELS + pixel_x) * 3;
        		framebuffer[idx + 0] = 0;
        		framebuffer[idx + 1] = 0;
        		framebuffer[idx + 2] = 0;
        	}
        }
    } else {
        if (c < 32 || c > 126) return; // ignore non-printable

        int ch = c;
        int row_offset = cursor_y * CHAR_H;
        int col_offset = cursor_x * CHAR_W;

        for (int row = 0; row < CHAR_H; row++) {
            uint8_t bits = font8x16[ch][row];
            for (int col = 0; col < CHAR_W; col++) {
                uint8_t val = (bits & (0x80 >> col)) ? 0xFF : 0x00;


                int pixel_x = col_offset + col;
                int pixel_y = row_offset + row;

                if (pixel_x >= SCREEN_H_PIXELS || pixel_y >= SCREEN_V_PIXELS)
                    continue;

                int idx = (pixel_y * SCREEN_H_PIXELS + pixel_x) * 3;
                framebuffer[idx + 0] = val;
                framebuffer[idx + 1] = val;
                framebuffer[idx + 2] = val;
            }
        }

        cursor_x++;
        if (cursor_x >= COLS) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= ROWS) {
        console_scroll();
    }

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
