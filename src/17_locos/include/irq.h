/*
Name: irq.h
Project: LocOS
Description: This file contains the definitions and function declarations for handling hardware interrupts (IRQs)
*/

#ifndef IRQ_H//Start include guard
#define IRQ_H  // Define include guard macro

#include <libc/stdint.h>//Integer types

void irq_init(void);// Initialize PIC controller
void irq_handler_c(uint32_t irq_no); // Route IRQ to handler

#endif                          