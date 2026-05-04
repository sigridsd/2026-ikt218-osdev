#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

// one slot in the IDT (8 bytes). the handler address is sliced in two
// because the CPU's gate descriptor format demands it.
typedef struct {
    uint16_t offset_low;  // low 16 bits of the handler address
    uint16_t selector;    // GDT selector to use (0x08 = our kernel code seg)
    uint8_t  zero;        // reserved, must be 0
    uint8_t  type_attr;   // gate type + DPL + present bit
    uint16_t offset_high; // top 16 bits of the handler address
} __attribute__((packed)) idt_entry_t;

// what we hand to lidt: table size and where it lives
typedef struct {
    uint16_t limit; // size of the table - 1
    uint32_t base;  // address of the first entry
} __attribute__((packed)) idt_ptr_t;

void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr);
void idt_init(void);
extern void idt_flush(uint32_t idt_ptr_address);

#endif