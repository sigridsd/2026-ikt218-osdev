#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// Snapshot of all registers when an interrupt fires.
// The order must match exactly what isr_common_stub pushes onto the stack.
typedef struct {
    uint32_t ds;                                    // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;                      // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;          // Pushed automatically by the CPU
} registers_t;

void isr_init(void);
void isr_handler(registers_t *regs);

extern void isr0(void); // Division By Zero
extern void isr1(void); // Debug
extern void isr2(void); // Non-Maskable Interrupt

#endif