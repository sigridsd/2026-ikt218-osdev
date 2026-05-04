#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

// One entry in the Interrupt Descriptor Table (8 bytes)
typedef struct {
    uint16_t offset_low;  // Lower 16 bits of the handler address
    uint16_t selector;    // Code segment in the GDT (0x08 = entry 1)
    uint8_t  zero;        // Always 0, reserved
    uint8_t  type_attr;   // Gate type, DPL and present bit
    uint16_t offset_high; // Upper 16 bits of the handler address
} __attribute__((packed)) idt_entry_t;

// IDT pointer loaded by the lidt instruction
typedef struct {
    uint16_t limit; // Size of the IDT minus 1
    uint32_t base;  // Memory address of the start of the IDT
} __attribute__((packed)) idt_ptr_t;

void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr);
void idt_init(void);
extern void idt_flush(uint32_t idt_ptr_address);

#endif