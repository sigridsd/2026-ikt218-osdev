#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// One entry in the GDT (8 bytes).
typedef struct gdt_entry {
    uint16_t limit_low;      // Limit bits 0-15
    uint16_t base_low;       // Base bits 0-15
    uint8_t  base_middle;    // Base bits 16-23
    uint8_t  access;         // Access byte
    uint8_t  limit_and_flags;// Upper 4 bits: flags, Lower 4 bits: limit bits 16-19
    uint8_t  base_high;      // Base bits 24-31
} __attribute__((packed)) gdt_entry_t;

// The GDT pointer loaded with the `lgdt` instruction.
typedef struct {
    uint16_t limit; // Size of GDT in bytes, minus 1
    uint32_t base;  // Address of the first gdt_entry_t
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);

// Defined in the assembly file — loads the GDT pointer with lgdt
extern void gdt_flush(uint32_t gdt_ptr_address);

#endif
