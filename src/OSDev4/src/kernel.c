#include <stdint.h>
#include <stddef.h>
#include "gdt.h"

/*
 * VGA text mode
 *
 * The BIOS maps an 80x25 character grid at physical address 0xB8000.
 * Each cell is 2 bytes:  [attribute byte][character byte]
 *   attribute high nibble = background colour
 *   attribute low  nibble = foreground colour
 */
#define VGA_BUFFER  ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_COLOR   0x0F   /* white text on black background */

static size_t terminal_col = 0;
static size_t terminal_row = 0;

/* Clear the entire screen with spaces */
static void terminal_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_BUFFER[y * VGA_WIDTH + x] = ' ' | ((uint16_t)VGA_COLOR << 8);
}

/* Write a single character, handling newlines and line wrap */
static void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_col] =
        (uint16_t)(unsigned char)c | ((uint16_t)VGA_COLOR << 8);

    if (++terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
}

/* Write a null-terminated string to the terminal */
void terminal_write(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
        terminal_putchar(str[i]);
}

/*
 * Kernel entry point.
 *
 * Called from multiboot2.asm after the bootloader hands control to _start.
 * Parameters pushed by the bootloader trampoline:
 *   magic      - should equal 0x36D76289 for Multiboot2
 *   mboot_info - pointer to the Multiboot2 information structure
 */
int main(uint32_t magic, void* mboot_info)
{
    (void)magic;
    (void)mboot_info;

    terminal_clear();

    /* Set up the Global Descriptor Table */
    gdt_init();

    terminal_write("Hello World\n");
    terminal_write("GDT initialized: NULL / Code / Data descriptors loaded.\n");

    /* Halt the CPU - nothing more to do */
    for (;;)
        __asm__ volatile ("hlt");

    return 0;
}
