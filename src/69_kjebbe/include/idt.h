#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

// One entry in the IDT (8 bytes).
// The CPU uses this to find the handler function when an interrupt fires.
typedef struct {
    uint16_t offset_low;  // Lower 16 bits of the handler function's address
    uint16_t selector;    // Code segment selector (0x08 = our kernel code segment)
    uint8_t  zero;        // Always 0, reserved by Intel
    uint8_t  type_attr;   // Gate type + privilege level + present bit
                          //   0x8E = 10001110b:
                          //     bit 7   (1)    = present (entry is valid)
                          //     bits 6-5 (00)  = ring 0 (kernel privilege)
                          //     bit 4   (0)    = interrupt/trap gate (not a task gate)
                          //     bits 3-0 (1110)= 32-bit interrupt gate
    uint16_t offset_high; // Upper 16 bits of the handler function's address
} __attribute__((packed)) idt_entry_t;

// The IDT pointer loaded with the `lidt` instruction.
// Same concept as the GDT pointer loaded with `lgdt`.
typedef struct {
    uint16_t limit; // Size of the IDT in bytes, minus 1
    uint32_t base;  // Address of the first idt_entry_t
} __attribute__((packed)) idt_ptr_t;

// Sets one IDT entry (called for each interrupt handler we register)
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector, uint8_t type_attr);

// Initialises the IDT and loads it with lidt
void idt_init(void);

// Defined in the assembly file — loads the IDT pointer with lidt
extern void idt_flush(uint32_t idt_ptr_address);

#endif
