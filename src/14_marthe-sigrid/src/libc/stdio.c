#include "../include/libc/stdio.h"
#include "../include/libc/stdarg.h"
#include "../include/libc/stddef.h"
#include "../include/libc/stdint.h"

// VGA-skjermen er 80 tegn bred og 25 linjer høy
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
// VGA-minnet starter på denne adressen
#define VGA_MEMORY 0xB8000

// Peker til VGA-minnebufferet
static uint16_t *terminal_buffer = (uint16_t *)VGA_MEMORY;
static int terminal_row = 0;
static int terminal_col = 0;
// Farge: lys grå tekst på svart bakgrunn
static uint8_t terminal_color = 0x07;

// Scroller skjermen opp én linje
static void terminal_scroll(void) {
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            terminal_buffer[(row - 1) * VGA_WIDTH + col] =
                terminal_buffer[row * VGA_WIDTH + col];
        }
    }
    // Tøm siste linje
    for (int col = 0; col < VGA_WIDTH; col++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] =
            (terminal_color << 8) | ' ';
    }
    terminal_row = VGA_HEIGHT - 1;
}

// Skriver ett tegn til skjermen
void terminal_putchar(char c, uint8_t color) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else {
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_col] =
            (color << 8) | c;
        terminal_col++;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            terminal_row++;
        }
    }
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
}

// Skriver en streng til skjermen
static void terminal_write(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i], terminal_color);
    }
}

// Skriver et heltall til skjermen
static void print_int(int value) {
    if (value < 0) {
        terminal_putchar('-', terminal_color);
        value = -value;
    }
    if (value == 0) {
        terminal_putchar('0', terminal_color);
        return;
    }
    char buf[12];
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0) {
        terminal_putchar(buf[--i], terminal_color);
    }
}

// Skriver et heksadesimalt tall til skjermen
static void print_hex(uint32_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    terminal_write("0x");
    if (value == 0) {
        terminal_putchar('0', terminal_color);
        return;
    }
    char buf[8];
    int i = 0;
    while (value > 0) {
        buf[i++] = hex_chars[value & 0xF];
        value >>= 4;
    }
    while (i > 0) {
        terminal_putchar(buf[--i], terminal_color);
    }
}

// Intern printf-funksjon med fargestøtte
static void vprintf_color(const char *format, va_list args, uint8_t color) {
    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++;
            switch (format[i]) {
                case 's': terminal_write(va_arg(args, const char *)); break;
                case 'd': print_int(va_arg(args, int)); break;
                case 'x': print_hex(va_arg(args, uint32_t)); break;
                case 'c': terminal_putchar((char)va_arg(args, int), color); break;
                case '%': terminal_putchar('%', color); break;
            }
        } else {
            terminal_putchar(format[i], color);
        }
    }
}

// Hovedfunksjonen for å skrive formatert tekst til skjermen
int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf_color(format, args, terminal_color);
    va_end(args);
    return 0;
}