#include <kernel/pit.h>
#include <kernel/interrupt.h>
#include <kernel/io.h>
#include <kernel/terminal.h>

static volatile uint32_t pit_ticks = 0; // Encasulated to pit.c to avoid accidental overwrite

uint32_t GetCurrentTick(void){
    return pit_ticks;
}

static void PitIrqHandler(struct Registers* regs){
    (void)regs; // Required by interface, not needed in this implementation
    pit_ticks++;
}

void PitInitialize(void){
    uint16_t divisor = DIVIDER;

    RegisterInterruptHandler(IRQ0, PitIrqHandler);

    OutPortByte(PIT_CMD_PORT, 0x36);
    OutPortByte(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    OutPortByte(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

//More efficient way of sleeping compared to SleepBusy()
void SleepInterrupt(uint32_t ticks_to_wait){
    uint32_t start_tick = GetCurrentTick();
    uint32_t end_tick = start_tick + ticks_to_wait;

    while(GetCurrentTick() < end_tick){
        __asm__ volatile ("sti; hlt");
    }
}

/* "Consumes" 100% CPU power to effectively put entire system into sleep
for (ticks_to_wait) amount of ticks / milliseconds */
void SleepBusy(uint32_t milliseconds){ 
    uint32_t start_tick = GetCurrentTick();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;

    while((GetCurrentTick() - start_tick) < ticks_to_wait){
        /* Occupy the CPU by with handling a whole lot of nothing.
        Also known as busy waiting */
    }
}

void SleepTest(){
    while (GetCurrentTick() < 15000) {
      TerminalWriteString("Sleeping with busy-waiting (HIGH CPU).\n");
      SleepBusy(1000);
      TerminalWriteString("Slept using busy-waiting.\n");

      TerminalWriteString("Sleeping with interrupts (LOW CPU).\n");
      SleepInterrupt(1000);
      TerminalWriteString("Slept using interrupts.\n");
  }
}
