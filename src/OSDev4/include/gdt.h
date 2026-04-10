#pragma once
#include <stdint.h>

/*
 * GDT Entry (segment descriptor) - 8 bytes
 *
 * Layout per Intel manual:
 *   [15:0]  limit_low   - Limit bits 0-15
 *   [31:16] base_low    - Base bits 0-15
 *   [39:32] base_mid    - Base bits 16-23
 *   [47:40] access      - Access byte (present, DPL, type flags)
 *   [51:48] limit_flags - Limit bits 16-19 in low nibble, flags in high nibble
 *   [63:56] base_high   - Base bits 24-31
 */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit_flags;  /* limit[16:19] | (granularity:size:L:AVL) */
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

/*
 * GDT Pointer - loaded with the lgdt instruction
 *   limit: size of GDT in bytes minus 1
 *   base:  linear address of the GDT
 */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* Initializes the GDT and reloads all segment registers */
void gdt_init(void);
