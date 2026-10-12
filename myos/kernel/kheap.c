/* kheap.c — физический аллокатор ядра (bitmap по 4КБ-страницам) + kmalloc через sbrk */
#include <stdint.h>
#include <stddef.h>
#include "kheap.h"
#include "vga.h"

#define TOTAL_PAGES 8192            /* 32 МБ RAM достаточно для v0.1 */
static uint8_t page_bitmap[TOTAL_PAGES / 8];
static size_t free_pages = TOTAL_PAGES;

void heap_init(void) {
    /* помечаем занятые первые страницы: low memory, VGA, ядро до ~1 МБ
       и область ядра на 0x10000 (64 КБ). Страницы 0..255 (1 МБ) считаем занятыми. */
    for (size_t i = 0; i < 256; i++) {
        page_bitmap[i / 8] |= 1 << (i % 8);
        free_pages--;
    }
}

void *palloc(size_t n) {
    size_t need = (n + PAGE_SIZE - 1) / PAGE_SIZE;
    if (need == 0) need = 1;
    size_t start = 0; bool found = false;
    for (size_t p = 256; p <= TOTAL_PAGES - need && !found; p++) {
        bool ok = true;
        for (size_t k = 0; k < need; k++)
            if (page_bitmap[(p + k) / 8] & (1 << ((p + k) % 8))) { ok = false; break; }
        if (ok) { start = p; found = true; }
    }
    if (!found) return 0;
    for (size_t k = 0; k < need; k++) {
        page_bitmap[(start + k) / 8] |= 1 << ((start + k) % 8);
        free_pages--;
    }
    return (void *)(start * PAGE_SIZE);
}

void pfree(void *addr, size_t n) {
    size_t start = (size_t)addr / PAGE_SIZE;
    size_t cnt = (n + PAGE_SIZE - 1) / PAGE_SIZE;
    if (cnt == 0) cnt = 1;
    for (size_t k = 0; k < cnt; k++) {
        page_bitmap[(start + k) / 8] &= ~(1 << ((start + k) % 8));
        free_pages++;
    }
}

size_t mem_free_pages(void) { return free_pages; }

/* ---------- kmalloc: линейная куча поверх palloc ---------- */
static char *kheap_ptr = 0;
static char *kheap_end = 0;

void kmalloc_init(void) {
    kheap_ptr = (char *)palloc(HEAP_INIT_PAGES * PAGE_SIZE);
    kheap_end = kheap_ptr + HEAP_INIT_PAGES * PAGE_SIZE;
}

void *kmalloc(size_t sz) {
    sz = (sz + 7) & ~7u;
    if (kheap_ptr + sz > kheap_end) return 0;   /* OOM: рост кучи пока не делаем */
    void *r = kheap_ptr;
    kheap_ptr += sz;
    return r;
}

void kfree(void *p) { (void)p; /* bump-аллокатор: освобождение no-op в v0.1 */ }

void *kcalloc(size_t n, size_t sz) {
    void *p = kmalloc(n * sz);
    if (p) { uint8_t *b = p; for (size_t i = 0; i < n * sz; i++) b[i] = 0; }
    return p;
}

/* newlib-совместимый sbrk, если когда-нибудь подключим libc */
void *_sbrk(ptrdiff_t incr) {
    char *prev = kheap_ptr;
    if (prev + incr > kheap_end) return (void *)-1;
    kheap_ptr += incr;
    return prev;
}
