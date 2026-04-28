#include "gdt.h"
#include "terminal.h"

void kernel_main(void)
{
    gdt_init();

    terminal_write("Hello World");

    for (;;) {
    }
}
