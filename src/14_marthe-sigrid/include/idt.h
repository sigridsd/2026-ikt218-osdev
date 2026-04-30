#ifndef IDT_H
#define IDT_H

#include "libc/stdint.h"

// En entry i Interrupt Descriptor Table (8 bytes)
typedef struct {
    uint16_t offset_low;  // Nedre 16 bits av handler-adressen
    uint16_t selector;    // Kodesegment i GDT (0x08 = entry 1)
    uint8_t  zero;        // Alltid 0, reservert
    uint8_t  type_attr;   // Gate type, DPL og present bit
    uint16_t offset_high; // Øvre 16 bits av handler-adressen
} __attribute__((packed)) idt_entry_t;

// IDT-peker som lastes med lidt-instruksjonen
typedef struct {
    uint16_t limit; // Størrelsen på IDT minus 1
    uint32_t base;  // Minneadresse til starten av IDT
} __attribute__((packed)) idt_ptr_t;

void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector,
                   uint8_t type_attr);
void idt_init(void);
extern void idt_flush(uint32_t idt_ptr_address);

#endif