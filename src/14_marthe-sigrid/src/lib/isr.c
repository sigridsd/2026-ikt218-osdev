#include "../include/isr.h"
#include "../include/idt.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"

// Navn på de tre eksepsjonene vi håndterer
static const char *exception_messages[] = {
    "Division By Zero",       // ISR 0
    "Debug",                  // ISR 1
    "Non-Maskable Interrupt"  // ISR 2
};

// Registrerer ISR-handlere i IDT
void isr_init(void) {
    // 0x08 = kodesegment i GDT
    // 0x8E = 32-bit interrupt gate, ring 0, present
    idt_set_entry(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
}

// Kalles fra isr_common_stub i isr.asm med en peker til registertilstanden
void isr_handler(registers_t *regs) {
    printf("ISR fired: interrupt %d: %s\n", regs->int_no,
           exception_messages[regs->int_no]);
}