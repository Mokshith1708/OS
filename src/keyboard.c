/**
 * Part of the PS
 */
#include <stdint.h>
#include <stdbool.h>
#include "keyboard.h"

#define SC_BREAK_CODE 0xF0

// Pointer to memory location to write ASCII characters
volatile char *key=(volatile unsigned int*)0x1180AA;

// Modifier key state
static bool caps_lock = false;
static bool shift = false;
static bool break_code_received = false;

// Unshifted ASCII mapping
static const char scancode_to_ascii[128] = {
    [0x1C] = 'a',
    [0x32] = 'b',
    [0x21] = 'c',
    [0x23] = 'd',
    [0x24] = 'e',
    [0x2B] = 'f',
    [0x34] = 'g',
    [0x33] = 'h',
    [0x43] = 'i',
    [0x3B] = 'j',
    [0x42] = 'k',
    [0x4B] = 'l',
    [0x3A] = 'm',
    [0x31] = 'n',
    [0x44] = 'o',
    [0x4D] = 'p',
    [0x15] = 'q',
    [0x2D] = 'r',
    [0x1B] = 's',
    [0x2C] = 't',
    [0x3C] = 'u',
    [0x2A] = 'v',
    [0x1D] = 'w',
    [0x22] = 'x',
    [0x35] = 'y',
    [0x1A] = 'z',
    [0x45] = '0',
    [0x16] = '1',
    [0x1E] = '2',
    [0x26] = '3',
    [0x25] = '4',
    [0x2E] = '5',
    [0x36] = '6',
    [0x3D] = '7',
    [0x3E] = '8',
    [0x46] = '9',
    [0x0E] = '`',
    [0x4E] = '-',
    [0x55] = '=',
    [0x5D] = '\\',
    [0x29] = ' ',
    [0x66] = '\b',      // Backspace
    [0x0D] = '\t',      // Tab
    [0x5A] = '\n',      // Enter
    [0x54] = '[',
    [0x5B] = ']',
    [0x4C] = ';',
    [0x52] = '\'',
    [0x41] = ',',
    [0x49] = '.',
    [0x4A] = '/',
};

// Shifted ASCII mapping
static const char scancode_to_ascii_shift[128] = {
    [0x1C] = 'A',
    [0x32] = 'B',
    [0x21] = 'C',
    [0x23] = 'D',
    [0x24] = 'E',
    [0x2B] = 'F',
    [0x34] = 'G',
    [0x33] = 'H',
    [0x43] = 'I',
    [0x3B] = 'J',
    [0x42] = 'K',
    [0x4B] = 'L',
    [0x3A] = 'M',
    [0x31] = 'N',
    [0x44] = 'O',
    [0x4D] = 'P',
    [0x15] = 'Q',
    [0x2D] = 'R',
    [0x1B] = 'S',
    [0x2C] = 'T',
    [0x3C] = 'U',
    [0x2A] = 'V',
    [0x1D] = 'W',
    [0x22] = 'X',
    [0x35] = 'Y',
    [0x1A] = 'Z',
    [0x45] = ')',
    [0x16] = '!',
    [0x1E] = '@',
    [0x26] = '#',
    [0x25] = '$',
    [0x2E] = '%',
    [0x36] = '^',
    [0x3D] = '&',
    [0x3E] = '*',
    [0x46] = '(',
    [0x0E] = '~',
    [0x4E] = '_',
    [0x55] = '+',
    [0x5D] = '|',
    [0x29] = ' ',
    [0x66] = '\b',      // Backspace
    [0x0D] = '\t',      // Tab
    [0x5A] = '\n',      // Enter
    [0x54] = '{',
    [0x5B] = '}',
    [0x4C] = ':',
    [0x52] = '"',
    [0x41] = '<',
    [0x49] = '>',
    [0x4A] = '?',
};


static bool left_shift = false;
static bool right_shift = false;

void PS2_ScanCodeHandler(uint8_t scancode)
{
    if (scancode == SC_BREAK_CODE) {
        break_code_received = true;
        return;
    }

    if (break_code_received) {
        // Key released, update modifier states accordingly
        if (scancode == 0x12) left_shift = false;  // Left Shift released
        if (scancode == 0x59) right_shift = false; // Right Shift released
        break_code_received = false;
        return;
    }

    // Key pressed
    if (scancode == 0x12) left_shift = true;  // Left Shift pressed
    else if (scancode == 0x59) right_shift = true; // Right Shift pressed
    else if (scancode == 0x58) {                 // Caps Lock pressed (make code)
        caps_lock = !caps_lock;
        return; // No char to output
    }

    bool is_shifted = left_shift || right_shift;
    char ascii_char = 0;

    if (is_shifted) {
        ascii_char = scancode_to_ascii_shift[scancode];
    } else {
        ascii_char = scancode_to_ascii[scancode];
    }

    // Handle caps lock for letters: toggle case if caps_lock is on and not shifted
    if (ascii_char >= 'a' && ascii_char <= 'z') {
        if (caps_lock && !is_shifted) {
            ascii_char = ascii_char - 'a' + 'A';
        }
    }

    if (ascii_char != 0) {
        // Store in memory (assuming keyboard points to proper buffer)
        *key = ascii_char;
    }
}
