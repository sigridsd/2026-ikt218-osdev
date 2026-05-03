#ifndef IRQ_H
#define IRQ_H

#include "libc/stdint.h"
extern uint32_t tick;
extern int suppress_keyboard_print;

void irq_init(void);
void irq_handler(uint8_t irq);
uint8_t get_last_scancode(void);

extern void* irq_stub_table[];


#endif