/* ramfs.c — файловая система в RAM с корзиной (для shell) */
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

#define MAX_FILES 32
#define MAX_NAME 24
#define MAX_DATA 512

typedef struct {
    char name[MAX_NAME];
    char data[MAX_DATA];
    uint32_t size;
    bool used, in_trash;
} file_t;

static file_t files[MAX_FILES];

static int str_len(const char *s){int n=0;while(s[n])n++;return n;}
static bool str_eq(const char*a,const char*b){while(*a&&*b){if(*a!=*b)return false;a++;b++;}return *a==*b;}
static void str_cpy(char*d,const char*s){while((*d++=*s++));}

void fs_init(void) {
    /* пара стартовых файлов + README про корзину */
    fs_touch("readme.txt");
    int i = 0; while (files[i].used && !str_eq(files[i].name,"readme.txt")) i++;
    const char *msg = "MyOS RAM FS v0.1. rm -> trash, restore/emptytrash.\n";
    for (uint32_t k = 0; msg[k] && k < MAX_DATA-1; k++) files[i].data[k]=msg[k];
    files[i].size = str_len(msg);
    fs_touch("autoexec.bat");
    i = 0; while (files[i].used && !str_eq(files[i].name,"autoexec.bat")) i++;
    const char *bat = "echo boot ok\n";
    for (uint32_t k = 0; bat[k] && k < MAX_DATA-1; k++) files[i].data[k]=bat[k];
    files[i].size = str_len(bat);
}

int fs_touch(const char *name) {
    if (!*name || str_len(name) >= MAX_NAME) return 0;
    for (int i = 0; i < MAX_FILES; i++)
        if (files[i].used && !files[i].in_trash && str_eq(files[i].name, name)) return 0; /* уже есть */
    for (int i = 0; i < MAX_FILES; i++)
        if (!files[i].used) {
            str_cpy(files[i].name, name);
            files[i].size = 0; files[i].used = true; files[i].in_trash = false;
            return 1;
        }
    return 0;
}

static file_t *find(const char *name, bool trash) {
    for (int i = 0; i < MAX_FILES; i++)
        if (files[i].used && files[i].in_trash == trash && str_eq(files[i].name, name))
            return &files[i];
    return 0;
}

int fs_cat(const char *name) {
    file_t *f = find(name, false);
    if (!f) return -1;
    for (uint32_t i = 0; i < f->size; i++) vga_putchar(f->data[i]);
    vga_putchar('\n');
    return 0;
}

/* запись текста в файл (для echo > не делаем; touch+cat достаточно для v0.1) */

void fs_ls(void) {
    bool any = false;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && !files[i].in_trash) {
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print_int((int32_t)files[i].size);
            vga_puts("  ");
            vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
            vga_puts(files[i].name);
            vga_putchar('\n');
            any = true;
        }
    }
    if (!any) vga_puts("(пусто)\n");
}

int fs_rm(const char *name) {
    file_t *f = find(name, false);
    if (!f) return 0;
    f->in_trash = true;   /* как в Windows: не удаляем сразу, а в корзину */
    return 1;
}

void fs_trash_list(void) {
    bool any = false;
    for (int i = 0; i < MAX_FILES; i++)
        if (files[i].used && files[i].in_trash) {
            vga_puts("[корзина] "); vga_puts(files[i].name); vga_putchar('\n');
            any = true;
        }
    if (!any) vga_puts("корзина пуста\n");
}

int fs_restore(const char *name) {
    file_t *f = find(name, true);
    if (!f) return 0;
    f->in_trash = false;
    return 1;
}

int fs_empty_trash(void) {
    for (int i = 0; i < MAX_FILES; i++)
        if (files[i].used && files[i].in_trash) files[i].used = false;
    return 1;
}
