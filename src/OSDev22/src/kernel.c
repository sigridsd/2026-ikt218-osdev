#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "libc/stdint.h"
#include "gdt.h"
#include "terminal.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    /* Sett opp GDT - må gjøres først for riktig segmentering */
    gdt_init();

    /* Klargjør skjermen */
    terminal_initialize();

    /* Sett opp interrupt-systemet */
    idt_init();     /* Opprett tom IDT-tabell */
    isr_init();     /* Registrer CPU-exceptions (0-31) */
    irq_init();     /* Remap PIC + registrer hardware IRQ-er (32-47) + sti */

    /* Start tastatur-driver */
    keyboard_init();

    printf("Hello World\n");
    printf("Interrupt-system aktivert!\n");
    printf("Write something on keybords:\n");

    /*
     * Hovedloop - holder kernelen i live.
     * hlt setter CPU-en i lavstrøm-modus til neste interrupt.
     * Uten denne loopen ville main() returnert og kræsjet.
     */
    for (;;) {
        asm volatile("hlt");
    }

    return 0;
}