#include "libc/libs.h"
#include <multiboot2.h>
#include "gdt.h"



struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};

int kernel_main();


int compute(int a, int b) {
    return a + b;
}

int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    int result = compute(2, 3);
    //printf("Result of compute(2, 3) is: %d\n", result);

    init_gdt();

    printf("Kernel initialized successfully!\n");

    // Call cpp kernel_main (defined in kernel.cpp)
    return kernel_main();
}