#include "kernel/pit.h"
#include "io.h"
#include "libc/stdio.h"

extern uint32_t tick;  // bumped by the IRQ0 handler in irq.c

uint32_t get_current_tick() {
    return tick;
}

void init_pit() {
    // PIT clock is 1193180 Hz, divide it down to whatever rate we want
    uint32_t divisor = DIVIDER;

    // 0x36 = channel 0, lo/hi byte, mode 3 (square wave), binary
    outb(PIT_CMD_PORT, 0x36);
    // divisor is 16 bits but the port is 8, so send low byte then high
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xFF);

    printf("PIT initialized at %d Hz\n", TARGET_FREQUENCY);
}

// just spin until enough ticks have passed
void sleep_busy(uint32_t milliseconds) {
    uint32_t start_tick = get_current_tick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    uint32_t elapsed_ticks = 0;

    while (elapsed_ticks < ticks_to_wait) {
        asm volatile("sti");  // make sure IRQs are on
        while (get_current_tick() == start_tick + elapsed_ticks) {
            // chill until the next tick
        }
        elapsed_ticks++;
    }
}

// hlt-based sleep, nicer to the CPU
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