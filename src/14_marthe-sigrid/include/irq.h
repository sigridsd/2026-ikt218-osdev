#ifndef IRQ_H
#define IRQ_H

#include "libc/stdint.h"
extern uint32_t tick;

void irq_init(void);
void irq_handler(uint8_t irq);

extern void* irq_stub_table[];

#endif