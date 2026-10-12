/* timer.c — PIT таймер (IRQ0, 100 Гц) */
#include "drivers.h"
#include "idt.h"
#include "vga.h"

volatile uint32_t ticks = 0;

void timer_init(void) {
    const uint16_t divisor = 11932;           /* 1.1932 МГц / 100 Гц */
    outb(0x43, 0x36);                         /* канал 0, lo/hi, rate generator */
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
    register_irq_handler(0, timer_callback);
}

void timer_callback(registers_t *r) { (void)r; ticks++; }
uint32_t timer_get_ticks(void) { return ticks; }

void sleep_ms(uint32_t ms) {
    uint32_t target = ticks + ms / 10 + 1;
    while (ticks < target) __asm__ volatile ("hlt");
}
