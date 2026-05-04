#include "../include/idt.h"

#define IDT_ENTRIES 256

static idt_entry_t idt_table[IDT_ENTRIES];
static idt_ptr_t   idt_descriptor;

extern void idt_default_gate(void);

// Stuffs a handler address + gate info into one IDT slot. The struct
// layout is dictated by the CPU, so we use a designated initializer
// to make it obvious which field is which.
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr) {
    idt_table[index] = (idt_entry_t){
        .offset_low  = (uint16_t)(handler & 0xFFFF),
        .selector    = selector,
        .zero        = 0,
        .type_attr   = type_attr,
        .offset_high = (uint16_t)((handler >> 16) & 0xFFFF),
    };
}

void idt_init(void) {
    // point every slot at the safe stub first, real handlers overwrite later
    for (uint16_t vec = 0; vec < IDT_ENTRIES; vec++) {
        idt_set_entry((uint8_t)vec, (uint32_t)idt_default_gate, 0x08, 0x8E);
    }

    idt_descriptor = (idt_ptr_t){
        .limit = (uint16_t)(sizeof(idt_table) - 1),
        .base  = (uint32_t)&idt_table,
    };
    idt_flush((uint32_t)&idt_descriptor);
}