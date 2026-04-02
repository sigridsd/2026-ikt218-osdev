#pragma once

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#define VGA_RED 0x44
#define VGA_WHITE 0x77
#define VGA_BLUE 0x11
#define VGA_GREEN_DARK 0x20

int printf(const char *__restrict__ format, ...);
int printf_color(const char *__restrict__ format, uint8_t terminal_color, ...);
void clearTerminal();
