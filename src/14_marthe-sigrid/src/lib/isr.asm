bits 32
extern isr_handler

global idt_default_gate
global isr_divzero
global isr_debug
global isr_nmi

; default stub for IDT slots we haven't filled in yet, just returns
idt_default_gate:
    iretd

; #DE doesn't push an error code, so we push a zero ourselves so the
; stack layout matches the others
isr_divzero:
    cli
    push dword 0
    push dword 0
    jmp exception_dispatch

; same deal, no error code from CPU
isr_debug:
    cli
    push dword 0
    push dword 1
    jmp exception_dispatch

; same deal, no error code from CPU
isr_nmi:
    cli
    push dword 0
    push dword 2
    jmp exception_dispatch

; common trampoline: stash the CPU state into a registers_t, swap to the
; kernel data segments, call into C, then put everything back and iretd.
exception_dispatch:
    pusha                       ; snapshot edi, esi, ebp, esp, ebx, edx, ecx, eax

    ; remember whatever ds the caller had, so we can restore it later
    xor eax, eax
    mov ax, ds
    push eax

    ; point every data segment at the kernel data selector (GDT entry 2)
    mov ax, 0x10
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax

    push esp                    ; pass &registers_t to the C handler
    call isr_handler
    add esp, 4                  ; pop that pointer back off

    ; put the caller's segments back
    pop eax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax

    popa                        ; restore general regs
    add esp, 8                  ; throw away the vector + fake error code
    sti
    iretd



extern irq_handler

; one stub per IRQ line: push the IRQ number, call into C, send EOI, return
%macro irq_stub 1
irq_stub_%+%1:
    cli
    push dword %1       ; tell irq_handler which IRQ we are
    call irq_handler
    add esp, 4
    mov al, 0x20        ; EOI to the master PIC so it'll send us more IRQs
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

global irq_entry_table
irq_entry_table:
%assign i 0
%rep 16
    dd irq_stub_%+i
%assign i i+1
%endrep