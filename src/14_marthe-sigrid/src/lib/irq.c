#include "irq.h"
#include "idt.h"
#include "io.h"
#include "libc/stdio.h"

uint32_t tick = 0;
// scancode -> ASCII lookup. index in = scancode, value out = the char
static char scancode_table[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',  '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0,    '*',  0,  ' '
};

static char keyboard_buffer[256];
static int buffer_index = 0;
static uint8_t last_scancode = 0;

uint8_t get_last_scancode(void) {  
    uint8_t sc = last_scancode;
    last_scancode = 0;
    return sc;
}

int suppress_keyboard_print = 0;
static void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);  // grab the scancode from the keyboard port

    // top bit set means key released, we don't care about those
    if (scancode & 0x80)
        return;

    last_scancode = scancode;

    char c = scancode_table[scancode];
    if (c == 0)
        return;

    // toss it in the buffer
    keyboard_buffer[buffer_index % 256] = c;
    buffer_index++;

    // print it back to the screen, unless something (snake/music) wants quiet
    if (!suppress_keyboard_print) {
    char str[2] = {c, '\0'};
    printf(str);
}
}

void irq_handler(uint8_t irq) {
    if (irq == 0)      // IRQ0 is the PIT timer, just count ticks
        tick++;
    if (irq == 1)      // IRQ1 is the keyboard
        keyboard_handler();
}
void irq_init(void) {
    // remap the PIC so its IRQs land at 0x20-0x2F instead of clashing with CPU exceptions

    // start init sequence on both PICs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // tell them their new vector offsets (master -> 0x20, slave -> 0x28)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    // tell master the slave is wired on IRQ2, and tell slave its cascade id
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    // 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // unmask everything
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    // wire the asm IRQ stubs into IDT slots 32-47
    for (uint8_t i = 0; i < 16; i++) {
        idt_set_entry(32 + i, (uint32_t)irq_entry_table[i], 0x08, 0x8E);
    }
}
