#ifndef IRQ_H
#define IRQ_H

#include "isr.h" // reuse registers_t

// Remaps the PIC and installs IRQ stubs into IDT slots 32-47
void irq_init(void);

// Called from irq_common_stub — dispatches to registered handlers and sends EOI
void irq_handler(registers_t *regs);

// Register a C handler for a specific IRQ line (0-15)
void irq_register_handler(int irq, void (*handler)(registers_t *));

// Assembly stubs — one per IRQ line (IRQ 0-15)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

#endif
