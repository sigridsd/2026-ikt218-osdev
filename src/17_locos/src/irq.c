/*
Name: irq.c
Project: LocOS
Description: This file contains the implementation of the IRQ (Interrupt Request) handling for LocOS project
*/

#include "irq.h" //IRQ setup function
#include "terminal.h"   // Print to the screen

#define PIC1_COMMAND 0x20 //Master PIC command port
#define PIC1_DATA    0x21  // Master PIC data port
#define PIC2_COMMAND 0xA0  // Slave PIC command port
#define PIC2_DATA    0xA1     // Slave PIC data port.

#define PIC_EOI      0x20  // End of interrupt signal
#define ICW1_INIT    0x10  // Initialization command word 1
#define ICW1_ICW4    0x01 //ICW4 needed flag
#define ICW4_8086    0x01 // 8086 mode flag
#define KBD_DATA     0x60  // Keyboard data port

#define KBD_BUF_SIZE 128  // Keyboard buffer size

static uint8_t kbd_scancode_buffer[KBD_BUF_SIZE]; // Store raw scancodes.
static uint32_t kbd_buf_head = 0;  // Buffer write position.
/* Single-writer demo buffer: wraps when full (old bytes are overwritten). */

/* PS/2 Set 1 scancode -> ASCII (minimal, no shift/caps support). */
static const char scancode_set1_ascii[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p', [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = ';', [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',',
    [0x34] = '.', [0x35] = '/',
    [0x39] = ' '
};

static inline void outb(uint16_t port, uint8_t value) { // Write byte to I/O port.
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {   // Read byte from I/O port.
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait(void) { // Small delay for PIC timing.
    outb(0x80, 0);
}

void irq_init(void) {   // Initialize PIC controller.
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, 0x20);  /* Master offset 0x20 (IRQ0..IRQ7) */
    io_wait();
    outb(PIC2_DATA, 0x28);  /* Slave  offset 0x28 (IRQ8..IRQ15) */
    io_wait();

    outb(PIC1_DATA, 4);     /* Tell Master about Slave at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 2);     /* Tell Slave its cascade identity */
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Keep it simple: unmask only IRQ1 (keyboard), mask everything else. */
    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);
}

static void keyboard_handle_irq1(void) { // Process keyboard input
    uint8_t scancode = inb(KBD_DATA);     //Read scancode from keyboard
    /* Store raw scancode for debugging/report requirements. */
    kbd_scancode_buffer[kbd_buf_head % KBD_BUF_SIZE] = scancode;
    kbd_buf_head++;

    /* Ignore key release events for this basic logger. */
    if (scancode & 0x80) {    //Check if key released.
        return;
    }

    char ch = scancode_set1_ascii[scancode];  // Look up ASCII value.
    if (ch == '\0') {
        return;
    }

    char out[2] = { ch, '\0' };// Create string.
    terminal_write(out);  // Print to screen.
}

void irq_handler_c(uint32_t irq_no) {  // Route IRQ to handler.
    if (irq_no == 1) {  // Keyboard IRQ.
        keyboard_handle_irq1();
    } else {    // Other IRQs.
        terminal_printf("IRQ %u triggered\n", (unsigned int)irq_no);
    }

    if (irq_no >= 8) {    // Slave PIC IRQ
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);  //Signal master PIC done.
}
