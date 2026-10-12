/* shell.c — текстовая командная оболочка MyOS (аналог cmd.exe) */
#include <stdint.h>
#include <stdbool.h>
#include "shell.h"
#include "vga.h"
#include "drivers.h"
#include "kheap.h"

#define LINE_MAX 128
static char line[LINE_MAX];
static size_t line_len = 0;

/* ---------- мини-fs: RAM-диск с корзиной ---------- */
void fs_init(void);
void fs_ls(void);
int  fs_touch(const char *name);
int  fs_cat(const char *name);
int  fs_rm(const char *name);          /* -> в корзину */
int  fs_restore(const char *name);     /* из корзины */
int  fs_empty_trash(void);
void fs_trash_list(void);

static void prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("MyOS:\\> ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

static bool starts(const char *cmd, const char *name) {
    while (*name) if (*cmd++ != *name++) return false;
    return *cmd == 0 || *cmd == ' ';
}
static const char *arg(const char *cmd) {
    while (*cmd && *cmd != ' ') cmd++;
    while (*cmd == ' ') cmd++;
    return *cmd ? cmd : 0;
}

static void execute(void) {
    const char *a;
    if (line_len == 0) return;

    if (starts(line, "help")) {
        vga_puts("Команды: help ver mem ls touch cat rm trash restore "
                 "emptytrash clear time echo <текст>\n");
    } else if (starts(line, "ver")) {
        vga_puts("MyOS Kernel v0.1 (i686, protected mode)\n");
    } else if (starts(line, "mem")) {
        vga_puts("Свободно страниц: ");
        vga_print_int((int32_t)mem_free_pages());
        vga_puts(" (~");
        vga_print_int((int32_t)(mem_free_pages() * 4));
        vga_puts(" КБ)\n");
    } else if (starts(line, "ls") || starts(line, "dir")) {
        fs_ls();
    } else if (starts(line, "touch") && (a = arg(line))) {
        if (fs_touch(a)) vga_puts("OK\n"); else vga_puts("ошибка\n");
    } else if (starts(line, "cat") && (a = arg(line))) {
        if (fs_cat(a) != 0) vga_puts("нет такого файла\n");
    } else if (starts(line, "rm") && (a = arg(line))) {
        if (fs_rm(a)) vga_puts("перемещено в корзину\n"); else vga_puts("нет такого файла\n");
    } else if (starts(line, "restore") && (a = arg(line))) {
        if (fs_restore(a)) vga_puts("восстановлено\n"); else vga_puts("в корзине нет\n");
    } else if (starts(line, "trash") || starts(line, "lstrash")) {
        fs_trash_list();
    } else if (starts(line, "emptytrash")) {
        fs_empty_trash();
        vga_puts("корзина очищена\n");
    } else if (starts(line, "clear")) {
        vga_clear();
    } else if (starts(line, "time")) {
        vga_print_int((int32_t)(timer_get_ticks() / 100));
        vga_puts(" сек с момента загрузки\n");
    } else if (starts(line, "echo")) {
        a = arg(line);
        if (a) { vga_puts(a); vga_putchar('\n'); } else vga_putchar('\n');
    } else if (starts(line, "shutdown")) {
        vga_puts("Остановка... (no real ACPI in v0.1)\n");
        for (;;) __asm__ volatile ("hlt");
    } else {
        vga_puts("неизвестная команда: '");
        vga_puts(line);
        vga_puts("' (help)\n");
    }
}

void shell_run(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("\n===============================================\n");
    vga_puts("   MyOS v0.1 — text-mode kernel + shell\n");
    vga_puts("===============================================\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    prompt();

    for (;;) {
        char c = kb_get_last();
        if (!c) continue;
        if (c == '\n') {
            vga_putchar('\n');
            line[line_len] = 0;
            execute();
            line_len = 0;
            prompt();
        } else if (c == '\b') {
            if (line_len > 0) { line_len--; vga_putchar('\b'); }
        } else if (line_len < LINE_MAX - 1) {
            line[line_len++] = c;
            vga_putchar(c);
        }
    }
}
