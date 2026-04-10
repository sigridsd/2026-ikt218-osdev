#include <libc/stdio.h>
#include <libc/stdint.h>
#include <libc/limits.h>
#include <libc/stddef.h>

const size_t VGA_HEIGHT = 25;
const size_t VGA_WIDTH = 80;

static uint16_t* terminalBuffer = (uint16_t*) 0xB8000;



void terminalEntryAt(char c, uint8_t colour, size_t x, size_t y){
    size_t index = y * VGA_WIDTH + x;
    terminalBuffer[index] = vgaEntry(c, colour);
}