#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/irq.h"
#include "../include/keyboard.h"
#include "../include/libc/stdbool.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"
#include "../include/multiboot2.h"
#include "libc/stddef.h"

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr) {
  gdt_init();
  idt_init();
  isr_init();
  irq_init();
  keyboard_init();

  // Enable interrupts — the CPU starts with interrupts disabled (cli in _start).
  // Only safe to do after GDT, IDT, ISRs and IRQs are all set up.
  asm volatile("sti");

  // Test: trigger interrupt 1 (Debug) manually
  asm volatile("int $0x1");

  printf("hello world");
  // Call cpp kernel_main (defined in kernel.cpp)
  return kernel_main();
}
