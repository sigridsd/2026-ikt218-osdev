#include "../../include/irq.h"
#include "../../include/idt.h"

// Write a byte to an I/O port (used to talk to the PIC)
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("out %1, %0" : : "dN"(port), "a"(value));
}

// ─── PIC remapping ────────────────────────────────────────────────────────────
//
// The 8259A PIC has two chips: master (IRQ 0-7) and slave (IRQ 8-15).
// Each has a command port and a data port:
//   Master: command = 0x20, data = 0x21
//   Slave:  command = 0xA0, data = 0xA1
//
// We send four Initialization Command Words (ICWs) to each chip to remap them:
//   ICW1 (0x11): start init sequence, cascade mode, ICW4 required
//   ICW2:        new base interrupt number (32 for master, 40 for slave)
//   ICW3 master: which IRQ line the slave is connected to (IRQ 2 = bit 2 = 0x04)
//   ICW3 slave:  its own cascade identity (0x02)
//   ICW4 (0x01): 8086 mode

static void pic_remap(void) {
    outb(0x20, 0x11); // ICW1: init master PIC
    outb(0xA0, 0x11); // ICW1: init slave PIC

    outb(0x21, 0x20); // ICW2: master maps IRQ 0-7  to interrupts 32-39
    outb(0xA1, 0x28); // ICW2: slave  maps IRQ 8-15 to interrupts 40-47

    outb(0x21, 0x04); // ICW3: slave is on IRQ 2 of master
    outb(0xA1, 0x02); // ICW3: slave's cascade identity

    outb(0x21, 0x01); // ICW4: 8086 mode
    outb(0xA1, 0x01); // ICW4: 8086 mode

    outb(0x21, 0x0);  // unmask all interrupts on master
    outb(0xA1, 0x0);  // unmask all interrupts on slave
}

// ─── IRQ init ─────────────────────────────────────────────────────────────────

void irq_init(void) {
    pic_remap();

    // Install the 16 stubs into IDT slots 32-47
    idt_set_entry(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_entry(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_entry(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_entry(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_entry(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_entry(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_entry(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_entry(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_entry(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_entry(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_entry(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_entry(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_entry(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_entry(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_entry(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_entry(47, (uint32_t)irq15, 0x08, 0x8E);
}

// ─── IRQ handler table ────────────────────────────────────────────────────────

static void (*irq_handlers[16])(registers_t *) = {0};

void irq_register_handler(int irq, void (*handler)(registers_t *)) {
    irq_handlers[irq] = handler;
}

// ─── IRQ handler ──────────────────────────────────────────────────────────────

void irq_handler(registers_t *regs) {
    int irq = regs->int_no - 32;

    if (irq_handlers[irq]) {
        irq_handlers[irq](regs);
    }

    // Send End Of Interrupt (EOI) to tell the PIC we're done.
    // IRQ 8-15 come from the slave PIC (interrupts 40-47), so the slave
    // also needs an EOI before the master does.
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20); // EOI to slave PIC
    }
    outb(0x20, 0x20);     // EOI to master PIC (always)
}
