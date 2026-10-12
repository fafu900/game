/* kheap.h */
#ifndef KHEAP_H
#define KHEAP_H
#include <stddef.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define HEAP_INIT_PAGES 256      /* 1 МБ куча ядра v0.1 */

void   heap_init(void);
void  *palloc(size_t n);
void   pfree(void *addr, size_t n);
size_t mem_free_pages(void);

void   kmalloc_init(void);
void  *kmalloc(size_t sz);
void  *kcalloc(size_t n, size_t sz);
void   kfree(void *p);

#endif
