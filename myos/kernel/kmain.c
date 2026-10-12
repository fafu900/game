/* kmain.c — инициализация ядра MyOS */
#include <stdint.h>
#include "vga.h"
#include "idt.h"
#include "drivers.h"
#include "kheap.h"
#include "shell.h"

void serial_init(void) {
    outb(0x3F8 + 1, 0x00);   /* disable interrupts */
    outb(0x3F8 + 3, 0x80);   /* DLAB on */
    outb(0x3F8 + 0, 0x03);   /* 38400 baud (divisor 3) */
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);   /* 8N1, DLAB off */
    outb(0x3F8 + 2, 0xC7);   /* FIFO on, clear, 14-byte trigger */
    outb(0x3F8 + 4, 0x0B);   /* DTR/RTS/OUT2 */
}

static void serial_puts(const char *s) {
    while (*s) {
        while ((inb(0x3F8 + 5) & 0x20) == 0) ;
        outb(0x3F8, *s++);
    }
}

void kmain(void) {
    /* порядок важен: VGA -> serial -> отладочный вывод -> прерывания -> память -> shell */
    vga_clear();
    serial_init();
    serial_puts("MyOS kernel: stage 1 - text mode up\n");

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_puts("MyOS kernel v0.1 booting...\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

    heap_init();
    kmalloc_init();
    serial_puts("MyOS kernel: physical allocator ready\n");

    idt_install();
    serial_puts("MyOS kernel: IDT installed, PIC remapped\n");

    timer_init();
    keyboard_init();
    __asm__ volatile ("sti");
    serial_puts("MyOS kernel: IRQ0(timer)+IRQ1(kbd) enabled\n");

    /* тест таймера: подождать ~300 мс, проверить что тики идут */
    uint32_t t0 = timer_get_ticks();
    sleep_ms(300);
    if (timer_get_ticks() > t0)
        serial_puts("MyOS kernel: PIT tick OK\n");
    else
        serial_puts("MyOS kernel: PIT FAILED\n");

    extern void fs_init(void);
    fs_init();

    shell_run();   /* не возвращается */
}
