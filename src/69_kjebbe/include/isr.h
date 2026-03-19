#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// Snapshot of all registers at the moment an interrupt fires.
// The layout must match exactly what isr_common_stub pushes onto the stack.
typedef struct {
    uint32_t ds;                                      // data segment (saved manually)
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha (edi first on stack)
    uint32_t int_no, err_code;                        // pushed by our stub
    uint32_t eip, cs, eflags, useresp, ss;            // pushed automatically by the CPU
} registers_t;

// Initialises ISRs: installs the stubs into the IDT
void isr_init(void);

// Called from isr_common_stub — prints which interrupt fired
void isr_handler(registers_t *regs);

// Assembly stubs — one per interrupt we handle
extern void isr0(void); // Division By Zero
extern void isr1(void); // Debug
extern void isr2(void); // Non-Maskable Interrupt

#endif
