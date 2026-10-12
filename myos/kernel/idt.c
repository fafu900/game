/* idt.c — IDT + PIC 8259 + обработчики исключений CPU */
#include "idt.h"
#include "vga.h"

static struct idt_entry idt[256];
static struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install(void) {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base = (uint32_t)&idt;

    /* remap PIC: IRQ0-7 -> int 32..39, IRQ8-15 -> 40..47 */
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFF); outb(0xA1, 0xFF);   /* маскируем всё до регистрации */

    /* таблица адресов 256 stub'ов собрана в isr_stub.asm */
    extern void *isr_addr_table[256];
    for (int i = 0; i < 256; i++)
        idt_set_gate(i, (uint32_t)isr_addr_table[i], 0x08, 0x8E);

    __asm__ volatile ("lidt %0" :: "m"(idtp));
}

/* диспетчер из isr_common (asm): IRQ -> irq_handler, исключения -> fault_handler */
void do_interrupt(registers_t *regs) {
    if (regs->int_no >= 32 && regs->int_no < 48) irq_handler(regs);
    else if (regs->int_no < 32)                  fault_handler(regs);
}

void register_irq_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        handlers[irq] = handler;
        /* открыть линию */
        uint8_t mask = inb(irq < 8 ? 0x21 : 0xA1);
        mask &= ~(1 << (irq % 8));
        outb(irq < 8 ? 0x21 : 0xA1, mask);
    }
}

irq_handler_t handlers[16] = {0};

void irq_handler(registers_t *regs) {
    if (regs->int_no >= 40) outb(0xA0, 0x20);   /* slave EOI */
    outb(0x20, 0x20);                            /* master EOI */
    uint8_t irq = regs->int_no - 32;
    if (irq < 16 && handlers[irq]) handlers[irq](regs);
}

static const char *exc_names[32] = {
    "Division By Zero","Debug","Non Maskable Interrupt","Breakpoint",
    "Into Detected Overflow","Out of Bounds","Invalid Opcode","No Coprocessor",
    "Double Fault","Coprocessor Segment Overrun","Bad TSS","Segment Not Present",
    "Stack Fault","General Protection Fault","Page Fault","Unknown Interrupt",
    "Coprocessor Fault","Alignment Check","Machine Check"
};

void fault_handler(registers_t *r) {
    vga_set_color(VGA_WHITE, VGA_RED);
    vga_puts("\n!!! KERNEL PANIC !!!\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("Exception: ");
    vga_puts(r->int_no < 32 ? exc_names[r->int_no] : "??");
    vga_puts("\nEIP="); vga_print_hex(r->eip);
    vga_puts(" ERR="); vga_print_hex(r->err_code);
    for (;;) __asm__ volatile ("cli; hlt");
}
