#include "../include/isr.h"
#include "../include/idt.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"

// Names of the three exceptions we handle
static const char *exception_messages[] = {
    "Division By Zero",       // ISR 0
    "Debug",                  // ISR 1
    "Non-Maskable Interrupt"  // ISR 2
};

// Register ISR handlers in the IDT
void isr_init(void) {
    // 0x08 = code segment in the GDT
    // 0x8E = 32-bit interrupt gate, ring 0, present
    idt_set_entry(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_entry(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_entry(2, (uint32_t)isr2, 0x08, 0x8E);
}

// Called from isr_common_stub in isr.asm with a pointer to the register state
void isr_handler(registers_t *regs) {
    printf("ISR fired: interrupt %d: %s\n", regs->int_no,
           exception_messages[regs->int_no]);
}