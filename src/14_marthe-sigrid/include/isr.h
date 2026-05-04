#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// snapshot of the CPU at the moment of an interrupt.
// the field order has to mirror what exception_dispatch pushes on the
// stack, otherwise everything will read out scrambled.
typedef struct {
    uint32_t ds;                                     // saved data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pushed by `pusha`
    uint32_t int_no, err_code;                       // we push these in the asm stubs
    uint32_t eip, cs, eflags, useresp, ss;           // the CPU pushes these for us
} registers_t;

void isr_init(void);
void isr_handler(registers_t *regs);

extern void isr_divzero(void); // #DE  divide error
extern void isr_debug(void);   // #DB  debug
extern void isr_nmi(void);     //      non-maskable interrupt

#endif