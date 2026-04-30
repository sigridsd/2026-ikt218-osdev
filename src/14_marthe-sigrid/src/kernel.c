#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "libc/stdint.h"

void main(uint32_t magic, uint32_t mb_info_addr)
{
    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    asm volatile("int $0x00");
    asm volatile("int $0x01");
    asm volatile("int $0x02");

    asm volatile("sti");

    while (1) {}
}