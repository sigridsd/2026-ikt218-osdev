#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"

static volatile uint16_t* VGA_BUFFER = (uint16_t*) 0xB8000;
static uint8_t VGA_WIDTH = 80;
static uint8_t VGA_HEIGHT = 25;
static uint8_t VGA_COLOR = 0x0F; // White on black

static size_t terminal_row = 0;
static size_t terminal_column = 0;

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

static void putc(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, VGA_COLOR);
    
    terminal_column++;
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
    }
}

void terminal_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    VGA_COLOR = 0x0F; // White on black

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', VGA_COLOR);
        }
    }
}

void terminal_write(const char* s) {
    for (size_t i = 0; s[i] != '\0'; i++) {
        putc(s[i]);
    }
}