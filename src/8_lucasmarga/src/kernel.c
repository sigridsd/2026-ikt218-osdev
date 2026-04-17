#include "libc/stdint.h"
#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "memory.h"

extern uint32_t end;

void kmain(uint32_t magic, void* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    gdt_setup();
    terminal_initialize();

    pic_remap();
    idt_setup();

    terminal_print_string("Hello World\n");

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();

    void* test1 = malloc(1000);
    void* test2 = malloc(2000);
    (void)test1;
    (void)test2;

    terminal_print_string("Keyboard logger ready:\n");

    __asm__ __volatile__("sti");

    __asm__ __volatile__("int $0");
    __asm__ __volatile__("int $1");
    __asm__ __volatile__("int $2");


    while (1) {
        __asm__ __volatile__("hlt");
    }
}