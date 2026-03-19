#include "../../include/idt.h"

static idt_entry_t idt[256];
static idt_ptr_t   idt_pointer;

void idt_set_entry(uint8_t index, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[index].offset_low  = handler & 0xFFFF;
    idt[index].selector    = selector;
    idt[index].zero        = 0;
    idt[index].type_attr   = type_attr;
    idt[index].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_init(void) {
    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base  = (uint32_t)&idt;

    // Zero out all entries — real handlers will be installed in Task 2
    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, 0, 0x08, 0x8E);
    }

    idt_flush((uint32_t)&idt_pointer);
}
