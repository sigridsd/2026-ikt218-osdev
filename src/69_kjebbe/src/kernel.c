#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/irq.h"
#include "../include/isr.h"
#include "../include/kernel/memory.h"
#include "../include/kernel/pit.h"
#include "../include/keyboard.h"
#include "../include/libc/stdbool.h"
#include "../include/libc/stddef.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"
#include "../include/matrix.h"
#include "../include/menu.h"
#include "../include/multiboot2.h"
#include "../include/piano.h"
#include "../include/program.h"
#include "lib/song/song.h"

extern uint32_t
    end; // Defined in arch/i386/linker.ld — marks the end of the kernel image

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr) {
  gdt_init();
  idt_init();
  isr_init();
  irq_init();
  keyboard_init();

  // Initialize kernel heap starting just past the kernel image.
  init_kernel_memory(&end);

  // Set up basic identity-mapped paging.
  init_paging();

  // Display heap/paging layout.
  print_memory_layout();

  // Initialise the PIT at 1000 Hz (1 tick per ms).
  init_pit();

  // Enable interrupts the CPU starts with interrupts disabled (cli in
  // _start). Only safe to do after GDT, IDT, ISRs, IRQs and PIT are all set up.
  asm volatile("sti");

  while (true) {
    int entry = kb_dequeue(&kb);
    if (entry != -1) {
      keyboard_handler(entry);
    }
    sleep_interrupt(1);
  }

  return kernel_main();
}
