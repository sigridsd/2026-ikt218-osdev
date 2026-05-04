bits 32
extern isr_handler

global dummy_isr
global isr0
global isr1
global isr2

; Empty ISR used to initialize every IDT entry
dummy_isr:
    iretd

; ISR 0 - Division By Zero
isr0:
    cli
    push dword 0
    push dword 0
    jmp isr_common_stub

; ISR 1 - Debug
isr1:
    cli
    push dword 0
    push dword 1
    jmp isr_common_stub

; ISR 2 - Non-Maskable Interrupt
isr2:
    cli
    push dword 0
    push dword 2
    jmp isr_common_stub

; Common stub shared by all ISRs
isr_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    sti
    iretd



extern irq_handler

%macro irq_stub 1
irq_stub_%+%1:
    cli
    push dword %1       ; pass IRQ number to irq_handler
    call irq_handler
    add esp, 4
    mov al, 0x20        ; EOI to master PIC
    out 0x20, al
    iretd
%endmacro

irq_stub 0
irq_stub 1
irq_stub 2
irq_stub 3
irq_stub 4
irq_stub 5
irq_stub 6
irq_stub 7
irq_stub 8
irq_stub 9
irq_stub 10
irq_stub 11
irq_stub 12
irq_stub 13
irq_stub 14
irq_stub 15

global irq_stub_table
irq_stub_table:
%assign i 0
%rep 16
    dd irq_stub_%+i
%assign i i+1
%endrep