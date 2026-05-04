#include "../include/gdt.h"

#define GDT_ENTRY_COUNT 3

static gdt_entry_t gdt_table[GDT_ENTRY_COUNT];
static gdt_ptr_t   gdt_descriptor;

// Stuffs base/limit/access/flags into the weird GDT layout where the
// base is split across 3 fields and the limit across 2. Using a
// designated initializer just so it's easy to see what each byte is.
static void gdt_set_entry(int index, uint32_t base, uint32_t limit,
                          uint8_t access, uint8_t flags) {
    gdt_table[index] = (gdt_entry_t){
        .limit_low       = (uint16_t)(limit & 0xFFFF),
        .base_low        = (uint16_t)(base & 0xFFFF),
        .base_middle     = (uint8_t)((base >> 16) & 0xFF),
        .access          = access,
        .limit_and_flags = (uint8_t)((flags << 4) | ((limit >> 16) & 0x0F)),
        .base_high       = (uint8_t)((base >> 24) & 0xFF),
    };
}

void gdt_init(void) {
    // entry 0: the null descriptor, CPU requires it
    gdt_set_entry(0, 0, 0, 0, 0);
    // entry 1: ring-0 code segment covering all 4 GiB (0x9A=code, 0xC=32-bit + page granularity)
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);
    // entry 2: same thing but for data (0x92 access)
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

    gdt_descriptor = (gdt_ptr_t){
        .limit = (uint16_t)(sizeof(gdt_table) - 1),
        .base  = (uint32_t)&gdt_table,
    };
    gdt_flush((uint32_t)&gdt_descriptor);
}