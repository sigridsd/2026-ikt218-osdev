extern irq_handler

global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; ─── IRQ stubs ────────────────────────────────────────────────────────────────
;
; Same idea as ISR stubs. No CPU error code for IRQs, so we always push 0.
; We push the remapped interrupt number (IRQ 0 = interrupt 32, etc.) so the
; C handler can tell which IRQ fired from regs->int_no.

irq0:  cli
    push dword 0
    push dword 32
    jmp irq_common_stub

irq1:  cli
    push dword 0
    push dword 33
    jmp irq_common_stub

irq2:  cli
    push dword 0
    push dword 34
    jmp irq_common_stub

irq3:  cli
    push dword 0
    push dword 35
    jmp irq_common_stub

irq4:  cli
    push dword 0
    push dword 36
    jmp irq_common_stub

irq5:  cli
    push dword 0
    push dword 37
    jmp irq_common_stub

irq6:  cli
    push dword 0
    push dword 38
    jmp irq_common_stub

irq7:  cli
    push dword 0
    push dword 39
    jmp irq_common_stub

irq8:  cli
    push dword 0
    push dword 40
    jmp irq_common_stub

irq9:  cli
    push dword 0
    push dword 41
    jmp irq_common_stub

irq10: cli
    push dword 0
    push dword 42
    jmp irq_common_stub

irq11: cli
    push dword 0
    push dword 43
    jmp irq_common_stub

irq12: cli
    push dword 0
    push dword 44
    jmp irq_common_stub

irq13: cli
    push dword 0
    push dword 45
    jmp irq_common_stub

irq14: cli
    push dword 0
    push dword 46
    jmp irq_common_stub

irq15: cli
    push dword 0
    push dword 47
    jmp irq_common_stub

; ─── Common stub ──────────────────────────────────────────────────────────────
; Identical structure to isr_common_stub — save state, call C, restore, iret.

irq_common_stub:
    pusha

    mov ax, ds
    push eax

    mov ax, 0x10        ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pointer to registers_t
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8          ; remove int_no and err_code
    sti
    iret
