#include "../../include/keyboard.h"
#include "../../include/irq.h"
#include "../../include/libc/stdint.h"
#include "../../include/libc/stdio.h"

// Read a byte from an I/O port (used to read from the PS/2 keyboard data port)
static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("in %1, %0" : "=a"(ret) : "dN"(port));
  return ret;
}

// ─── Scancode set 1 → ASCII lookup table
// ──────────────────────────────────────
//
// Index is the PS/2 scancode (make code only, key press).
// Value is the ASCII character to print. 0 means no printable character.
// This is a US QWERTY layout.

static const char scancode_ascii[128] = {
    0,    0,   '1', '2',  '3',  '4', '5',  '6',  // 0x00-0x07
    '7',  '8', '9', '0',  '-',  '=', '\b', '\t', // 0x08-0x0F
    'q',  'w', 'e', 'r',  't',  'y', 'u',  'i',  // 0x10-0x17
    'o',  'p', '[', ']',  '\n', 0,   'a',  's',  // 0x18-0x1F
    'd',  'f', 'g', 'h',  'j',  'k', 'l',  ';',  // 0x20-0x27
    '\'', '`', 0,   '\\', 'z',  'x', 'c',  'v',  // 0x28-0x2F
    'b',  'n', 'm', ',',  '.',  '/', 0,    '*',  // 0x30-0x37
    0,    ' ', 0,   0,    0,    0,   0,    0,    // 0x38-0x3F
    0,    0,   0,   0,    0,    0,   0,    '7',  // 0x40-0x47
    '8',  '9', '-', '4',  '5',  '6', '+',  '1',  // 0x48-0x4F
    '2',  '3', '0', '.',  0,    0,   0,    0,    // 0x50-0x57
    0,    0,   0,   0,    0,    0,   0,    0,    // 0x58-0x5F
    0,    0,   0,   0,    0,    0,   0,    0,    // 0x60-0x67
    0,    0,   0,   0,    0,    0,   0,    0,    // 0x68-0x6F
    0,    0,   0,   0,    0,    0,   0,    0,    // 0x70-0x77
    0,    0,   0,   0,    0,    0,   0,    0,    // 0x78-0x7F
};

// ─── IRQ1 handler
// ─────────────────────────────────────────────────────────────

static void keyboard_handler(registers_t *regs) {
  (void)regs;

  uint8_t scancode = inb(0x60); // read scancode from PS/2 data port

  // Bit 7 set means key release — ignore those, only handle key press
  if (scancode & 0x80)
    return;

  char c = scancode_ascii[scancode];
  if (c) {
    printf("%c", c);
  }
}

void keyboard_init(void) { irq_register_handler(1, keyboard_handler); }
