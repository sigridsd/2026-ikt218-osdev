#include "../include/idt.h"

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

// Fill in an IDT entry with handler address, segment and type
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr) {
    // Lower 16 bits of the handler address
    idt[index].offset_low = handler & 0xFFFF;
    // Upper 16 bits of the handler address
    idt[index].offset_high = (handler >> 16) & 0xFFFF;
    // Code segment in the GDT (0x08 = entry 1)
    idt[index].selector = selector;
    // Always 0
    idt[index].zero = 0;
    // Gate type and privilege level
    idt[index].type_attr = type_attr;
}

extern void dummy_isr(void);

void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Fill all 256 entries with an empty ISR that just returns
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_entry(i, (uint32_t)dummy_isr, 0x08, 0x8E);
    }

    // Load the IDT into the CPU via the lidt instruction
    idt_flush((uint32_t)&idt_ptr);
}