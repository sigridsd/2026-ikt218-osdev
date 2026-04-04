#pragma once

#include "libc/stddef.h"
#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t num);
void *memset(void *ptr, int value, size_t num);
void *memset16(void *ptr, uint16_t value, size_t num);

#ifdef __cplusplus
}
#endif
