/* keyboard.c — драйвер клавиатуры PS/2 (порт 0x60, IRQ1) */
#include "drivers.h"
#include "idt.h"
#include "vga.h"

static const char keymap[128] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/',
    0,   '*', 0, ' ', 0
};
static const char keymap_shift[128] = {
    0,   0,  '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,   '|','Z','X','C','V','B','N','M','<','>','?',
    0,   '*', 0, ' ', 0
};

static bool shift = false, ctrl = false;
char last_key = 0;
void keyboard_handler(registers_t *r);

void keyboard_init(void) {
    /* очищаем буфер контроллера */
    while (inb(0x64) & 1) inb(0x60);
    register_irq_handler(1, keyboard_handler);
}

void keyboard_handler(registers_t *r) {
    (void)r;
    uint8_t sc = inb(0x60);
    if (sc == 0x2A || sc == 0x36) { shift = true; return; }
    if (sc == 0xAA || sc == 0xB6) { shift = false; return; }
    if (sc == 0x1D) { ctrl = true; return; }
    if (sc == 0x9D) { ctrl = false; return; }
    if (sc & 0x80) return;       /* release-коды игнорируем */

    char c = shift ? keymap_shift[sc] : keymap[sc];
    if (c) {
        last_key = c;
        vga_putchar(c);          /* эхо ввода */
    }
}

bool kb_ctrl_pressed(void) { return ctrl; }
char kb_get_last(void) { char k = last_key; last_key = 0; return k; }
