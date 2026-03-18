#include "../include/libc/stdbool.h"
#include "../include/libc/stdint.h"
#include "../include/multiboot2.h"
#include "libc/stddef.h"

typedef struct gdt_entry {
  uint16_t limit_low;  // Limit bits 0-15
  uint16_t base_low;   // Base bits 0-15
  uint8_t base_middle; // Base bits 16-23
  uint8_t access;      // Access byte
  uint8_t
      limit_and_flags; // Upper 4 bits: flags, Lower 4 bits: limit bits 16-19
  uint8_t base_high;   // Base bits 24-31
} __attribute__((packed)) gdt_entry_t;

// The GDT pointer that gets passed to lgdt
typedef struct {
  uint16_t limit; // Size of GDT minus 1
  uint32_t base;  // Address of the first gdt_entry_t
} __attribute__((packed)) gdt_ptr_t;

// The actual gdt table
gdt_entry_t gdt[3];
gdt_ptr_t gdt_pointer;

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access,
                   uint8_t flags) {
  gdt[index].limit_low = limit & 0xFFFF;
  gdt[index].base_low = base & 0xFFFF;
  gdt[index].base_middle = (base >> 16) & 0xFF;
  gdt[index].access = access;
  gdt[index].limit_and_flags = (flags << 4) | ((limit >> 16) & 0x0F);
  gdt[index].base_high = (base >> 24) & 0xFF;
}

// Defined in multiboot assembly file
extern void gdt_flush(uint32_t gdt_ptr_address);

void gdt_init(void) {
  // Null descriptor - everything zero
  gdt_set_entry(0, 0, 0, 0, 0);

  // Code segment - base 0, full 4GiB, access 0x9A, flags 0xC
  gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xC);

  // Data segment - base 0, full 4GiB, access 0x92, flags 0xC
  gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);

  // Set up the pointer
  gdt_pointer.limit = sizeof(gdt) - 1;
  gdt_pointer.base = (uint32_t)&gdt;

  // Load it
  gdt_flush((uint32_t)&gdt_pointer);
}

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr) {
  gdt_init();
  printf("hello world");
  // Call cpp kernel_main (defined in kernel.cpp)
  return kernel_main();
}
