#include "../include/idt.h"

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

// Fyller inn en IDT-entry med handler-adresse, segment og type
void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr) {
    // Nedre 16 bits av handler-adressen
    idt[index].offset_low = handler & 0xFFFF;
    // Øvre 16 bits av handler-adressen
    idt[index].offset_high = (handler >> 16) & 0xFFFF;
    // Kodesegment i GDT (0x08 = entry 1)
    idt[index].selector = selector;
    // Alltid 0
    idt[index].zero = 0;
    // Gate type og privilegenivå
    idt[index].type_attr = type_attr;
}

extern void dummy_isr(void);

void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint32_t)&idt;

    // Fyll alle 256 entries med en tom ISR som bare returnerer
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_entry(i, (uint32_t)dummy_isr, 0x08, 0x8E);
    }

    // Last IDT inn i CPU via lidt-instruksjonen
    idt_flush((uint32_t)&idt_ptr);
}