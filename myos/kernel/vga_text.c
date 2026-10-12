/* vga_text.c — текстовый видеодрайвер (VGA 0xB8000, 80x25) */
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

#include <stddef.h>
static uint16_t *const vga = (uint16_t *)0xB8000;
static size_t cursor_x = 0, cursor_y = 0;
static uint8_t color_attr = VGA_LIGHT_GREY | (VGA_BLACK << 4);

#define COLS 80
#define ROWS 25

static inline void put_char_at(char c, uint8_t attr, size_t x, size_t y) {
    if (x < COLS && y < ROWS)
        vga[y * COLS + x] = (uint16_t)c | ((uint16_t)attr << 8);
}

static void update_hardware_cursor(void) {
    const uint16_t pos = cursor_y * COLS + cursor_x;
    outb(0x3D4, 14); outb(0x3D5, (pos >> 8) & 0xFF);
    outb(0x3D4, 15); outb(0x3D5, pos & 0xFF);
}

void vga_clear(void) {
    for (size_t i = 0; i < COLS * ROWS; i++)
        vga[i] = (' ' | ((uint16_t)color_attr << 8));
    cursor_x = cursor_y = 0;
    update_hardware_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    color_attr = (fg & 0x0F) | (bg << 4);
}

static void scroll_up(void) {
    for (size_t i = 0; i < (ROWS - 1) * COLS; i++)
        vga[i] = vga[i + COLS];
    for (size_t x = 0; x < COLS; x++)
        put_char_at(' ', color_attr, x, ROWS - 1);
    cursor_y = ROWS - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        if (++cursor_y >= ROWS) scroll_up();
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            put_char_at(' ', color_attr, cursor_x, cursor_y);
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7u;
        if (cursor_x >= COLS) { cursor_x = 0; if (++cursor_y >= ROWS) scroll_up(); }
    } else {
        put_char_at(c, color_attr, cursor_x, cursor_y);
        if (++cursor_x >= COLS) {
            cursor_x = 0;
            if (++cursor_y >= ROWS) scroll_up();
        }
    }
    update_hardware_cursor();
}

void vga_puts(const char *s) {
    while (*s) vga_putchar(*s++);
}

void vga_print_hex(uint32_t v) {
    static const char d[] = "0123456789ABCDEF";
    char buf[11] = "0x________\0";
    for (int i = 0; i < 8; i++) buf[2 + i] = d[(v >> (28 - i * 4)) & 0xF];
    vga_puts(buf);
}

void vga_print_int(int32_t v) {
    char buf[12]; int i = 0; bool neg = v < 0;
    uint32_t u = neg ? -v : v;
    do { buf[i++] = '0' + u % 10; u /= 10; } while (u);
    if (neg) buf[i++] = '-';
    while (i--) vga_putchar(buf[i]);
}
