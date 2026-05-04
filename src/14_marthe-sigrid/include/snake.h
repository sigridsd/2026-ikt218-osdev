#ifndef SNAKE_H
#define SNAKE_H

// play field size (numbers borrowed from serene-dev/snake-c)
#define COLS 74
#define ROWS 20

// where on the VGA screen the field starts (top-left corner)
#define BOARD_OFF_X 1
#define BOARD_OFF_Y 2

// VGA color attributes (low nibble = foreground, high nibble = background)
#define COLOR_BLACK      0x00
#define COLOR_GREEN      0x02
#define COLOR_RED        0x04
#define COLOR_WHITE      0x0F
#define COLOR_YELLOW     0x0E
#define COLOR_CYAN       0x0B
#define COLOR_DARK_GRAY  0x08

void snake_game(void);

#endif