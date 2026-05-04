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

// Print the start menu with colored entries
void show_menu() {
    printf_color("\n  ========================\n", 0x0B); // cyan
    printf_color("  Marthe & Sigrid's menu\n", 0x0F);     // white
    printf_color("  ========================\n", 0x0B);   // cyan
    printf_color("  1 - Play Music\n", 0x0D);             // magenta
    printf_color("  2 - Play Snake\n", 0x0A);             // green
    printf_color("  ========================\n", 0x0B);   // cyan
    printf_color("  Press 1 or 2\n", 0x07);            // light gray
}

// Kernel entry point. Called from multiboot2.asm after the bootloader
// has loaded us. magic and mb_info_addr come from the multiboot2 spec.
void main(uint32_t magic, uint32_t mb_info_addr)
{
    // Set up CPU descriptor tables and interrupt handlers
    gdt_init();
    idt_init();
    isr_init();
    irq_init();

    // Set up the kernel heap, paging, and the timer
    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    // Smoke-test the heap with a few allocations
    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

    // Main menu loop: clear screen, show menu, wait for 1 or 2
    while (1) {
        terminal_clear();
        show_menu();

        uint8_t sc = 0;
        while (sc != 0x02 && sc != 0x03) {  // 0x02 = scancode for '1', 0x03 = '2'
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
