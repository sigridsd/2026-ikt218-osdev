#include "../include/gdt.h"

#define GDT_ENTRIES 3

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdt_ptr;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags) {
    // Store the lower 16 bits of limit and base
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].base_low = base & 0xFFFF;
    // Middle byte of base
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].access = access;
    // Upper 4 bits are flags, lower 4 bits are the high part of limit
    gdt[index].limit_and_flags = (flags << 4) | ((limit >> 16) & 0x0F);
    // Top byte of base
    gdt[index].base_high = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    // Entry 0: always null
    gdt_set_entry(0, 0, 0, 0, 0);
    // Entry 1: code segment, covers all memory (0 to 4GB)
    // access 0x9A = present, ring 0, code, readable
    // flags 0xC = 32-bit, page granularity
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);
    // Entry 2: data segment, covers all memory
    // access 0x92 = present, ring 0, data, writable
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdt_ptr);
}