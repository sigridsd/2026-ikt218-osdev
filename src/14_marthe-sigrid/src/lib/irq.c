#include "irq.h"
#include "idt.h"
#include "io.h"
#include "libc/stdio.h"

// Scancode til ASCII lookup table
// Index = scancode, verdi = ASCII tegn
static char scancode_table[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',  '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,    '*',  0,  ' '
};

static char keyboard_buffer[256];
static int buffer_index = 0;

static void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);  // les scancode fra keyboard port

    // ignorer key release events (bit 7 satt)
    if (scancode & 0x80)
        return;

    char c = scancode_table[scancode];
    if (c == 0)
        return;

    // lagre i buffer
    keyboard_buffer[buffer_index % 256] = c;
    buffer_index++;

    // print tegnet til skjermen
    char str[2] = {c, '\0'};
    printf(str);
}

void irq_handler(uint8_t irq) {
    if (irq == 1)
        keyboard_handler();
}
void irq_init(void) {
    // Remap PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    for (uint8_t i = 0; i < 16; i++) {
        idt_set_entry(32 + i, (uint32_t)irq_stub_table[i], 0x08, 0x8E);
    }
}
