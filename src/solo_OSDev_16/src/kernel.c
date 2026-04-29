#include "gdt.h" 
#include "terminal.h"
#include "idt.h"
#include "irq.h"
#include "pic.h"


void kmain(void) {
gdt_initialize();
terminal_initialize();
idt_init();
pic_remap();
__asm__ volatile("sti"); 

while(1){}
}