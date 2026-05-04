#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// One entry in the Global Descriptor Table (8 bytes)
typedef struct {
    uint16_t limit_low;       // Lower 16 bits of the segment size
    uint16_t base_low;        // Lower 16 bits of the base address
    uint8_t  base_middle;     // Middle 8 bits of the base address
    uint8_t  access;          // Access rights and privilege level
    uint8_t  limit_and_flags; // Upper 4 bits: flags, lower 4 bits: limit
    uint8_t  base_high;       // Top 8 bits of the base address
} __attribute__((packed)) gdt_entry_t;

// GDT pointer loaded by the lgdt instruction
typedef struct {
    uint16_t limit; // Size of the GDT minus 1
    uint32_t base;  // Memory address of the start of the GDT
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);
extern void gdt_flush(uint32_t gdt_ptr_address);

#endif