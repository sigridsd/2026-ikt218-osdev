#include <libc/stdint.h>

void main(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}