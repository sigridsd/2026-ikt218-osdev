#ifndef GDT_H
#define GDT_H

#include "libc/stdint.h"

// one slot in the GDT (8 bytes). layout dictated by the CPU,
// base + limit are sliced into multiple fields, hence the awkward names.
typedef struct {
    uint16_t limit_low;       // low 16 bits of the segment size
    uint16_t base_low;        // low 16 bits of the base address
    uint8_t  base_middle;     // middle 8 bits of the base
    uint8_t  access;          // access byte (ring level, type, present, etc.)
    uint8_t  limit_and_flags; // top 4 bits = flags, bottom 4 = high nibble of limit
    uint8_t  base_high;       // top 8 bits of the base
} __attribute__((packed)) gdt_entry_t;

// what we hand to lgdt: how big the table is and where it lives
typedef struct {
    uint16_t limit; // size of the table - 1
    uint32_t base;  // address of the first entry
} __attribute__((packed)) gdt_ptr_t;

void gdt_init(void);
extern void gdt_flush(uint32_t gdt_ptr_address);

#endif