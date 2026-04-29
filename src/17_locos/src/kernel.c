/*
Name: kernel.c
Project: LocOS
Description: Main kernel entry file. It starts the system and runs simple tests.
             This is a school project about operating system development.
*/

#include "gdt.h"       // GDT setup.
#include "idt.h"       // IDT setup.
#include "terminal.h"  // Screen output.

void kmain(void) {      // Main kernel start function.
    gdt_init();         // Load the GDT first.
    terminal_init();    // Turn on text output.
    idt_init();         // Load the IDT and IRQ setup.

    terminal_printf("Hello World!\n");                 // Print a simple line.
    terminal_printf("LocOS\n");                        // Print the OS name.
    terminal_write("\n Hello World! (write)\n");      // Test terminal_write.
    terminal_printf("\n Hello World! (printf)\n");    // Test terminal_printf.
    terminal_printf("IDT loaded. Triggering test interrupts...\n"); // Start interrupt tests.

    __asm__ volatile ("int $0x0");   // Test interrupt 0.
    __asm__ volatile ("int $0x1");   // Test interrupt 1.
    __asm__ volatile ("int $0x2");   // Test interrupt 2.
    __asm__ volatile ("int $0x30");  // Test software interrupt 0x30.

    terminal_printf("Triggering test IRQ vector...\n"); // Start IRQ test.
    __asm__ volatile ("int $0x20");                    // Test IRQ 0 vector.

    terminal_printf("Keyboard logger ready. Type on keyboard:\n"); // Keyboard test.
    __asm__ volatile ("sti");                                   // Enable interrupts.

    for (;;) {                 // Idle loop.
        __asm__ volatile ("hlt"); // Sleep until next interrupt.
    }
}


   