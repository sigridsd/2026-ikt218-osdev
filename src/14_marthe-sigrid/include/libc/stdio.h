#pragma once
#include "stdbool.h"
#include "stddef.h"

int putchar(int ic);
bool print(const char* data, size_t length);
int printf(const char* __restrict__ format, ...);
void printf_color(const char* str, uint8_t color);