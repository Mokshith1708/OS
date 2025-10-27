#pragma once
#include <stdint.h>

void display_init(uint8_t* fb_base_addr);
void display_clear_screen(uint32_t color);
void display_put_char(char c);
void display_puts(const char* s);
void display_put_int(int n);
void display_put_hex(uint32_t n);
void display_set_color(uint32_t fg_color, uint32_t bg_color);
void display_set_cursor(int col, int row);