#include "../../include/kernel/pit.h"
#include "../../include/libc/stdio.h"
char *width_string = "0000000000000000000000000000000000000000000000000000000"
                     "0000000000000000000000000";

char *width1_string = "00000000000000000000";

char *width2_string = "00000";

char *width3_string = "0000000000";

char *width4_string = "00000";

char *width5_string = "0000000000000000000000000000000000000000";

void print_strip() {
  printf_color(width1_string, VGA_RED);
  printf_color(width2_string, VGA_WHITE);
  printf_color(width3_string, VGA_BLUE);
  printf_color(width4_string, VGA_WHITE);
  printf_color(width5_string, VGA_RED);
}
void print_top() { // 9 lines
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
}
void print_bottom() { // 7
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
  print_strip();
}
void print_middle_strip() {
  printf_color(width1_string, VGA_WHITE);
  printf_color(width2_string, VGA_WHITE);
  printf_color(width3_string, VGA_BLUE);
  printf_color(width4_string, VGA_WHITE);
  printf_color(width5_string, VGA_WHITE);
}
void print_middle() { // 9
  print_middle_strip();
  print_middle_strip();
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  printf_color(width_string, VGA_BLUE);
  print_middle_strip();
  print_middle_strip();
}

void ja_vi_elsker() {
  clearTerminal();
  print_top();
  print_middle();
  print_bottom();
}

void colorshow() {
  while (true) {
    clearTerminal();
    ja_vi_elsker();
    sleep_interrupt(10000);
  }
}
