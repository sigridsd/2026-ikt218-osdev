; gdt_flush.asm
;
; Loads the GDT using the lgdt instruction and reloads all segment registers
; so they point to the new descriptors.
;
; C prototype:  void gdt_flush(gdt_ptr_t* ptr);
;
; The argument (pointer to the gdt_ptr_t struct) is on the stack at [esp+4]
; after the call instruction has pushed the return address.

global gdt_flush

section .text
bits 32

gdt_flush:
    mov eax, [esp+4]    ; Load the address of the gdt_ptr_t struct
    lgdt [eax]          ; Load the new GDT

    ; Reload all data segment registers with the data selector.
    ; Selector 0x10 = GDT index 2 (data segment), RPL 0.
    ;   index 2 * 8 bytes per entry = 16 = 0x10
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; CS cannot be changed with a normal mov.  A far jump flushes the
    ; instruction pipeline and reloads CS with the code selector.
    ; Selector 0x08 = GDT index 1 (code segment), RPL 0.
    ;   index 1 * 8 bytes per entry = 8 = 0x08
    jmp 0x08:.reload_cs
.reload_cs:
    ret
