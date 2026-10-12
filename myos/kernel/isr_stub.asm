; isr_stub.asm — 256 stub'ов прерываний + адресная таблица для IDT.
; Формат стека при входе в C-обработчик = struct registers (см. idt.h):
;   [ds edi esi ebp esp ebx edx ecx eax int_no err_code eip cs eflags]

[bits 32]
extern do_interrupt

%macro ISR_NOERR 1
isr_stub_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push dword %1
    jmp isr_common
%endmacro

%assign i 0
%rep 256
  %if (i==8) || (i>=10 && i<=14) || (i==17)
    ISR_ERR i
  %else
    ISR_NOERR i
  %endif
  %assign i i+1
%endrep

isr_common:
    pusha                       ; eax..edi (8 регистров)
    push ds                     ; ds — первое поле registers_t
    mov ax, 0x10                ; data selector
    mov ds, ax
    push esp                    ; аргумент: registers_t*
    call do_interrupt
    add esp, 4
    pop ds
    popa
    add esp, 8                  ; int_no + error_code
    iretd

section .data
global isr_addr_table
isr_addr_table:
%assign i 0
%rep 256
    dd isr_stub_%+i
  %assign i i+1
%endrep
