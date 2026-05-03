#ifndef SNAKE_H
#define SNAKE_H

// Board dimensions (from github.com/serene-dev/snake-c)
#define COLS 74
#define ROWS 20

// Board offset on VGA screen
#define BOARD_OFF_X 1
#define BOARD_OFF_Y 2

// VGA colors
#define COLOR_BLACK      0x00
#define COLOR_GREEN      0x02
#define COLOR_RED        0x04
#define COLOR_WHITE      0x0F
#define COLOR_YELLOW     0x0E
#define COLOR_CYAN       0x0B
#define COLOR_DARK_GRAY  0x08

void snake_game(void);

#endif