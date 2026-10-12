; boot.asm — MBR-загрузчик MyOS (NASM)
; 1. BIOS грузит нас на 0x7C00, печатаем сообщение в текстовый режим
; 2. Включаем A20, грузим GDT, переходим в protected mode
; 3. Читаем ядро (LBA 1..N) через INT 0x13/0x42 (packet НА СТЕКЕ) в сегмент 0x1000
;    (физический адрес 0x10000: int 0x13 работает только ниже 1 МБ)
; 4. Дальний прыжок 0x8:0x10000 -> kernel_entry (ядро само поднимет Paging/IDT/TSS)

BITS 16
ORG 0x7C00

jmp short start
nop

; ---------- BPB FAT12 (совместимость) ----------
bpbOEM            db "MyOS    "
bpbBytesPerSec    dw 512
bpbSectorsPerClust db 1
bpbResSectors     dw 1
bpbFATs           db 2
bpbRootDirEnts    dw 224
bpbSectors        dw 2880
bpbMedia          db 0xF8
bpbFATSectors     dw 9
bpbSectorsPerTrack dw 18
bpbHeads          dw 2
bpbHiddenSectors  dd 0
bpbDriveNumber    db 0x80

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, msg_loading
    call print16

    ; --- A20 fast gate ---
    in al, 0x92
    or al, 2
    out 0x92, al

    ; --- GDT + protected mode ---
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword 0x8:flush32

BITS 32
flush32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7C00

    ; --- Disk Address Packet на стеке (int 0x13 читает по DS:SI) ---
    sub esp, 16
    mov esi, esp
    mov word [esi+0], 0x0010    ; размер пакета
    mov word [esi+2], 128       ; sector count
    mov word [esi+4], 0x0000    ; target offset
    mov word [esi+6], 0x1000    ; target segment (физ = 0x1000*16 = 0x10000)
    mov dword [esi+8], 1        ; starting LBA = 1 (сразу после MBR)
    mov dword [esi+12], 0

    mov ah, 0x42
    mov dl, 0x80
    int 0x13
    jc read_fail

    ; --- прыжок в начало ядра: CS=0x8, EIP=0x10000 ---
    jmp dword 0x8:0x10000

read_fail:
.hang:
    cli
    hlt
    jmp .hang

BITS 16
print16:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    xor bh, bh
    int 0x10
    jmp print16
.done:
    ret

msg_loading db 13,10,"MyOS bootloader v0.1 - loading kernel...",13,10,0

; ---------- GDT ----------
align 8
gdt_start:
    dq 0                       ; 0x00 null
    dq 0x00CF9A000000FFFF      ; 0x08 код
    dq 0x00CF92000000FFFF      ; 0x10 данные
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start + 0x7C00

times 510-($-$$) db 0
dw 0xAA55
