#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "libc/stdint.h"
#include "libc/stdio.h"
#include "kernel/memory.h"
#include "kernel/pit.h"
extern uint32_t end;

void main(uint32_t magic, uint32_t mb_info_addr)
{
    gdt_init();
    printf("Hello World\n"); //hello world test
    idt_init();
    isr_init();
    irq_init();

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

int counter = 0;
while(true){
    printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
    sleep_busy(1000);
    printf("[%d]: Slept using busy-waiting.\n", counter++);

    printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
    sleep_interrupt(1000);
    printf("[%d]: Slept using interrupts.\n", counter++);
}
}