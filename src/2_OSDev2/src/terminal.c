#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"

static volatile uint16_t* VGA_BUFFER = (uint16_t*) 0xB8000; // VGA text mode buffer address
static uint8_t VGA_WIDTH = 80;  // VGA text mode width in characters
static uint8_t VGA_HEIGHT = 25; // VGA text mode height in characters
static uint8_t VGA_COLOR = 0x0F; // White on black

static size_t terminal_row = 0; // Current row in the terminal
static size_t terminal_column = 0;  // Current column in the terminal

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {  // Combine character and color into a single 16-bit value
    return (uint16_t) c | (uint16_t) color << 8;    // Character is in the lower byte, color is in the upper byte
}

static void putc(char c) {  // Handle newline character
    if (c == '\n') {    // Move to the beginning of the next line
        terminal_column = 0;
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, VGA_COLOR);   // Move to the next column after writing the character
    
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
    }
}

void terminal_init(void) {  // Initialize terminal state and clear the screen
    terminal_row = 0;
    terminal_column = 0;
    VGA_COLOR = 0x0F; // White on black

    for (size_t y = 0; y < VGA_HEIGHT; y++) {   // Clear the screen by filling it with spaces
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR);
        }
    }
}

void terminal_write(const char* s) {    // Write a null-terminated string to the terminal
    for (size_t i = 0; s[i] != '\0'; i++) {
        putc(s[i]);
    }
}