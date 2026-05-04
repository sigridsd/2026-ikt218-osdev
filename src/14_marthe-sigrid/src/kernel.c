#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "libc/stdint.h"
#include "libc/stdio.h"
#include "kernel/memory.h"
#include "kernel/pit.h"
#include "snake.h"
extern uint32_t end;

void play_music();

// little colored start menu
void show_menu() {
    printf_color("\n  ========================\n", 0x0B); // cyan
    printf_color("  Marthe & Sigrid's menu\n", 0x0F);     // white
    printf_color("  ========================\n", 0x0B);
    printf_color("  1 - Play Music\n", 0x0D);             // magenta
    printf_color("  2 - Play Snake\n", 0x0A);             // green
    printf_color("  ========================\n", 0x0B);
    printf_color("  Press 1 or 2\n", 0x07);
}

// kernel entry point, jumped to from multiboot2.asm
void main(uint32_t magic, uint32_t mb_info_addr)
{
    // get the CPU tables and interrupts up
    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    // heap, paging, timer
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    // poke the heap a bit just to check it works
    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

    // menu loop, just wait for 1 or 2
    while (1) {
        terminal_clear();
        show_menu();

        uint8_t sc = 0;
        while (sc != 0x02 && sc != 0x03) {  // 0x02 = '1', 0x03 = '2'
            sc = get_last_scancode();
            sleep_busy(10);
        }

        if (sc == 0x02) {
            play_music();
        } else if (sc == 0x03) {
            snake_game();
        }
    }
}
