extern main
global _start
global idt_flush

section .multiboot_header
header_start:
    dd 0xe85250d6
    dd 0
    dd header_end - header_start
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
align 8
    dw 0
    dw 0
    dw 8
header_end:

section .text
bits 32
_start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call main
.hang:
    hlt
    jmp .hang

idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

    global gdt_flush
gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

section .bss
stack_bottom:
    resb 4096 * 16
stack_top:
