#ifndef ISR_H
#define ISR_H

#include "libc/stdint.h"

// Snapshot av alle registre når et interrupt inntreffer
// Rekkefølgen må matche nøyaktig hva isr_common_stub pusher på stacken
typedef struct {
    uint32_t ds;                                    // Datasegment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushet av pusha
    uint32_t int_no, err_code;                      // Interrupt nummer og error code
    uint32_t eip, cs, eflags, useresp, ss;          // Pushet automatisk av CPU
} registers_t;

void isr_init(void);
void isr_handler(registers_t *regs);

extern void isr0(void); // Division By Zero
extern void isr1(void); // Debug
extern void isr2(void); // Non-Maskable Interrupt

#endif