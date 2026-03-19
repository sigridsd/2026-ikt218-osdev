#include "../../include/gdt.h"

static gdt_entry_t gdt[3];
static gdt_ptr_t   gdt_pointer;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].limit_low       = limit & 0xFFFF;
    gdt[index].base_low        = base & 0xFFFF;
    gdt[index].base_middle     = (base >> 16) & 0xFF;
    gdt[index].access          = access;
    gdt[index].limit_and_flags = (flags << 4) | ((limit >> 16) & 0x0F);
    gdt[index].base_high       = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    // Null descriptor — required as the first entry, everything zero
    gdt_set_entry(0, 0, 0, 0, 0);

    // Code segment — base 0, full 4 GiB, access 0x9A, flags 0xC
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);

    // Data segment — base 0, full 4 GiB, access 0x92, flags 0xC
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

    gdt_pointer.limit = sizeof(gdt) - 1;
    gdt_pointer.base  = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdt_pointer);
}
