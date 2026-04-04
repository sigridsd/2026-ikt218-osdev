#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include "libc/stddef.h"
#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

void init_kernel_memory(uint32_t *kernel_end);

char *pmalloc(size_t size);
void pfree(void *mem);
void *malloc(size_t size);
void free(void *mem);

void print_memory_layout(void);

#ifdef __cplusplus
}
#endif

#endif
