#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"

// turn an exception number into a name. we only hook the first three,
// so anything else falling in here means something weird happened.
static const char *exception_label(uint32_t vector) {
    switch (vector) {
        case 0: return "(Divide Error)";
        case 1: return "(Debug)";
        case 2: return "NMI (Non-Maskable Interrupt)";
    }
    return "<unknown vector>";
}

// hook the three exception stubs into the IDT.
// 0x08 = kernel code segment in the GDT, 0x8E = 32-bit interrupt gate, ring 0, present.
void isr_init(void) {
    void (*const stubs[3])(void) = { isr_divzero, isr_debug, isr_nmi };
    for (uint8_t vec = 0; vec < 3; vec++) {
        idt_set_entry(vec, (uint32_t)stubs[vec], 0x08, 0x8E);
    }
}

// called from the asm trampoline once it's stashed all the registers into *regs
void isr_handler(registers_t *regs) {
    printf("[exception] vector=%d err=0x%x -> %s\n",
           regs->int_no, regs->err_code, exception_label(regs->int_no));
}