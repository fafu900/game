; entry.asm — точка входа ядра MyOS (вызывается загрузчиком по 0x8:0x10000)
; Поднимаем свой стек и прыгаем в kmain (C). GDT/IDT настраивает само ядро.

[bits 32]

global kernel_entry
extern kmain

section .bss
align 16
kernel_stack resb 16384          ; 16 КБ стек ядра

section .text
kernel_entry:
    cli
    mov esp, kernel_stack + 16384
    call kmain

.hang:
    hlt
    jmp .hang
