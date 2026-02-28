#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"
#include "gdt.h"

void main(void) {
    terminal_init();
    terminal_write("Hello, OS World!\n");

    gdt_init();
    terminal_write("GDT initialized successfully.\n");

    terminal_write("Kernel initialization complete.\n");

    for(;;) {
        // Halt the CPU to save power until the next interrupt
        __asm__ volatile ("hlt");
    }
}