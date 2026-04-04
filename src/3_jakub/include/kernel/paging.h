#ifndef KERNEL_PAGING_H
#define KERNEL_PAGING_H

#include "libc/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_paging(void);
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys);

#ifdef __cplusplus
}
#endif

#endif
