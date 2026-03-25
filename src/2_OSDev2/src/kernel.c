#include <libc/stddef.h>
#include <libc/stdint.h>

#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"

void main(void) {
    terminal_init();
    terminal_write("Hello, OS World!\n");

    gdt_init();
    terminal_write("GDT initialized successfully.\n");

    idt_init();
    terminal_write("IDT initialized successfully.\n");

    irq_init();
    __asm__ volatile ("sti"); // Enable interrupts
    terminal_write("IRQ initialized successfully.\n");

    keyboard_init();
    terminal_write("Keyboard initialized successfully.\n");

    terminal_write("Kernel initialization complete.\n");
    terminal_write("Type on the keyboard to see output:\n");

    for(;;) {
        // Halt the CPU to save power until the next interrupt
        __asm__ volatile ("hlt");
    }
}