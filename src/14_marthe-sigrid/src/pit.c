#include "kernel/pit.h"
#include "io.h"
#include "libc/stdio.h"

extern uint32_t tick;  // shared with irq.c (incremented in IRQ0 handler)

uint32_t get_current_tick() {
    return tick;
}

void init_pit() {
    uint32_t divisor = DIVIDER;

    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

    printf("PIT initialized at %d Hz\n", TARGET_FREQUENCY);
}

void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        asm volatile("sti");  // make sure interrupts are enabled
        while (get_current_tick() == start_tick + elapsed_ticks) {
            // busy wait until the next PIT tick
        }
        elapsed_ticks++;
    }
}

void sleep_interrupt(uint32_t milliseconds) {
    uint32_t current_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t end_ticks = current_tick + ticks_to_wait;

    while (current_tick < end_ticks) {
        asm volatile("sti");
        asm volatile("hlt");
        current_tick = get_current_tick();
    }
}