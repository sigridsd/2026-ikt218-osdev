#include "terminal.h"
#include <libc/stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

static uint16_t *video_memory = (uint16_t *)0xB8000;
static int terminal_column = 0;
static int terminal_row = 0;

static void terminal_newline(void)
{
    terminal_column = 0;
    terminal_row++;

    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
    }
}

void terminal_write(const char *text)
{
    int i = 0;

    while (text[i] != '\0') {
        if (text[i] == '\n') {
            terminal_newline();
            i++;
            continue;
        }

        int index = terminal_row * VGA_WIDTH + terminal_column;
        video_memory[index] = (uint16_t)text[i] | (uint16_t)(VGA_COLOR_WHITE_ON_BLACK << 8);

        terminal_column++;

        if (terminal_column >= VGA_WIDTH) {
            terminal_newline();
        }

        i++;
    }
}
