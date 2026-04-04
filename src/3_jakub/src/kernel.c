#include "libc/libs.h"
#include <multiboot2.h>
#include "gdt.h"
#include "kernel/heap.h"
#include "kernel/paging.h"
#include "kernel/pit.h"
#include "interrupts/idt.h"
#include "interrupts/isr.h"
#include "interrupts/keyboard.h"

extern uint32_t end;

struct multiboot_info
{
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr)
{
    terminal_initialize();

    (void)mb_info_addr;

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printf("Invalid multiboot magic\n");
        return 1;
    }

    init_gdt();
    
    // Initialize IDT
    init_idt();
    
    // Initialize Interrupts and IRQs
    init_isr();
    init_irq();

    init_kernel_memory(&end);
    init_paging();

    // Initialize the keyboard logger
    init_keyboard();
    init_pit();

    // Enable CPU Interrupts
    asm volatile("sti");

    terminal_print_logo();
    keyboard_print_prompt();

    // Call cpp kernel_main (defined in kernel.cpp)
    return kernel_main();
}
