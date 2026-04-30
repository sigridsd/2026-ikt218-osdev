#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// En entry i Global Descriptor Table (8 bytes)
typedef struct {
    uint16_t limit_low;       // Nedre 16 bits av segmentstørrelsen
    uint16_t base_low;        // Nedre 16 bits av startadressen
    uint8_t  base_middle;     // Midterste 8 bits av startadressen
    uint8_t  access;          // Tilgangsrettigheter og privilegenivå
    uint8_t  limit_and_flags; // Øvre 4 bits: flags, nedre 4 bits: limit
    uint8_t  base_high;       // Øverste 8 bits av startadressen
} __attribute__((packed)) gdt_entry_t;

// GDT-peker som lastes med lgdt-instruksjonen
typedef struct {
    uint16_t limit; // Størrelsen på GDT minus 1
    uint32_t base;  // Minneadresse til starten av GDT
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);
extern void gdt_flush(uint32_t gdt_ptr_address);

#endif