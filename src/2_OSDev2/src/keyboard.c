#include <libc/stdint.h>
#include <libc/stddef.h>
#include "keyboard.h"
#include "io.h"
#include "terminal.h"

#define KBD_DATA_PORT 0x60

static uint8_t buf[256]; // Buffer to store keystrokes
static uint8_t head = 0; // Points to the next position to write in the buffer
static uint8_t tail = 0; // Points to the next position to read from the buffer

static void push_scancode(uint8_t scancode) {
    buf[head] = scancode; // Store scancode in buffer
    head = (uint8_t)(head + 1) % 256; // Move head forward, wrap around if necessary
    if (head == tail) { // Buffer overflow, move tail forward to discard oldest scancode
        tail = (uint8_t)(tail + 1) % 256;
    }
}

static const char scancode_set1[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // F1-F10 placeholders
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_init(void) {
    // No specific initialization needed for the keyboard in this simple implementation
}

void keyboard_on_irq1(void) {
    uint8_t scancode = inb(KBD_DATA_PORT); // Read scancode from keyboard data port
    push_scancode(scancode); // Store scancode in buffer

    // Ignore key releases (scancodes with the high bit set)
    if (scancode & 0x80) return;

    if (scancode < 128) {
        char c = scancode_set1[scancode]; // Translate scancode to ASCII
        if (c) {
            char s[2] = {c, '\0'}; // Create a string for the character
            terminal_write(s); 
        }
    }
}