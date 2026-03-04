#include "libc/libs.h"

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0F; // hvit tekst på svart bakgrunn

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

int putchar(int ic) {
    unsigned char c = (unsigned char) ic;

    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        return ic;
    }

    const size_t index = terminal_row * 80 + terminal_column;
    VGA_MEMORY[index] = vga_entry(c, terminal_color);

    terminal_column++;
    if (terminal_column == 80) {
        terminal_column = 0;
        terminal_row++;
    }

    return ic;
}

bool print(const char* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        putchar(data[i]);
    }
    return true;
}

static void print_number(int value) {
    char buffer[16];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }

    if (value < 0) {
        putchar('-');
        value = -value;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        putchar(buffer[--i]);
    }
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    int written = 0;

    for (size_t i = 0; format[i] != '\0'; i++) {

        if (format[i] == '%') {
            i++;

            if (format[i] == 'd') {
                int value = va_arg(args, int);
                print_number(value);
            }
            else if (format[i] == 's') {
                char* str = va_arg(args, char*);
                while (*str) {
                    putchar(*str++);
                }
            }
            else if (format[i] == 'c') {
                char c = (char)va_arg(args, int);
                putchar(c);
            }
        }
        else {
            putchar(format[i]);
        }

        written++;
    }

    va_end(args);
    return written;
}