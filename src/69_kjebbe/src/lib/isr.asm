extern isr_handler  ; the C function we call from the common stub

global isr0
global isr1
global isr2

; ─── ISR stubs ────────────────────────────────────────────────────────────────
;
; For interrupts where the CPU does NOT push an error code we push a dummy 0
; so the stack layout is always the same when we reach isr_common_stub.
;
; Stack on entry to isr_common_stub (top = low address):
;   [int_no]  ← we pushed
;   [err_code]← we pushed (dummy 0, or real code for some exceptions)
;   [eip]     ← CPU pushed
;   [cs]      ← CPU pushed
;   [eflags]  ← CPU pushed

; ISR 0 — Division By Zero (no CPU error code)
isr0:
    cli
    push dword 0   ; dummy error code
    push dword 0   ; interrupt number
    jmp isr_common_stub

; ISR 1 — Debug (no CPU error code)
isr1:
    cli
    push dword 0   ; dummy error code
    push dword 1   ; interrupt number
    jmp isr_common_stub

; ISR 2 — Non-Maskable Interrupt (no CPU error code)
isr2:
    cli
    push dword 0   ; dummy error code
    push dword 2   ; interrupt number
    jmp isr_common_stub

; ─── Common stub ──────────────────────────────────────────────────────────────
;
; All stubs jump here. We save the remaining registers, switch to the kernel
; data segment, call the C handler, then restore everything and return.

isr_common_stub:
    pusha               ; push eax, ecx, edx, ebx, esp, ebp, esi, edi

    mov ax, ds          ; save the current data segment
    push eax

    mov ax, 0x10        ; load kernel data segment (entry 2 in our GDT)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; push pointer to the registers_t struct on the stack
    call isr_handler    ; call the C handler
    add esp, 4          ; remove the pointer we pushed

    pop eax             ; restore original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; restore general purpose registers
    add esp, 8          ; remove int_no and err_code we pushed in the stub
    sti
    iret                ; return from interrupt (pops eip, cs, eflags automatically)
