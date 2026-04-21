#include "libc/stdint.h"
#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "memory.h"
#include "pit.h"

extern uint32_t end;

static void print_int(int n) {
    if (n == 0) { terminal_print_string("0"); return; }
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    while (n > 0) { buf[--i] = '0' + (n % 10); n /= 10; }
    terminal_print_string(&buf[i]);
}

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

    init_pit();

    void* some_memory = malloc(12345);
    void* memory2     = malloc(54321);
    void* memory3     = malloc(13331);
    (void)some_memory;
    (void)memory2;
    (void)memory3;

    __asm__ __volatile__("int $0");
    __asm__ __volatile__("int $1");
    __asm__ __volatile__("int $2");

    terminal_print_string("Keyboard logger ready:\n");

    __asm__ __volatile__("sti");

    int counter = 0;
    while (1) {
        terminal_print_string("[");
        print_int(counter);
        terminal_print_string("]: Sleeping with busy-waiting (HIGH CPU).\n");
        sleep_busy(1000);
        terminal_print_string("[");
        print_int(counter++);
        terminal_print_string("]: Slept using busy-waiting.\n");

        terminal_print_string("[");
        print_int(counter);
        terminal_print_string("]: Sleeping with interrupts (LOW CPU).\n");
        sleep_interrupt(1000);
        terminal_print_string("[");
        print_int(counter++);
        terminal_print_string("]: Slept using interrupts.\n");
    }
}