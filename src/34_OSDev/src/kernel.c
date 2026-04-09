#include "terminal.h"
#include "gdt.h"

void main(uint32_t mb_magic, void* mb_info) {
    (void)mb_magic; //stop warnings about unused variables
    (void)mb_info;

    terminal_initialize();
    init_gdt();

    printf("Hello World\n");

    while (1) {} //loop forever (kernel should never return so CPU doesnt execute random memory)
}