#include "../include/gdt.h"

#define GDT_ENTRIES 3

static gdt_entry_t gdt[GDT_ENTRIES];
static gdt_ptr_t gdt_ptr;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags) {
    // Lagre de nedre 16 bitene av limit og base
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].base_low = base & 0xFFFF;
    // Midterste byte av base
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].access = access;
    // Øvre 4 bits er flags, nedre 4 bits er øvre del av limit
    gdt[index].limit_and_flags = (flags << 4) | ((limit >> 16) & 0x0F);
    // Øverste byte av base
    gdt[index].base_high = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    // Entry 0: alltid null
    gdt_set_entry(0, 0, 0, 0, 0);
    // Entry 1: kodesegment, dekker hele minnet (0 til 4GB)
    // access 0x9A = present, ring 0, code, readable
    // flags 0xC = 32-bit, page granularity
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);
    // Entry 2: datasegment, dekker hele minnet
    // access 0x92 = present, ring 0, data, writable
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdt_ptr);
}