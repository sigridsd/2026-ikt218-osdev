/*
 * Snake game for UiAOS
 * Game logic (ring-buffer, apple placement, collision) adapted from:
 *   https://github.com/serene-dev/snake-c/blob/main/main.c
 * All I/O replaced with bare-metal VGA, PCSPK, and keyboard IRQ.
 */

#include "/include/snake.h"
#include "/include/libc/stdint.h"
#include "/include/kernel/pit.h"
#include "/include/io.h"

// VGA
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)0xB8000;

static void vga_put(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    vga[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = ((uint16_t)COLOR_BLACK << 8) | ' ';
}

static void vga_print(int x, int y, const char *str, uint8_t color) {
    for (int i = 0; str[i]; i++)
        vga_put(x + i, y, str[i], color);
}

static void vga_print_int(int x, int y, int val, uint8_t color) {
    char buf[12];
    int i = 0;
    if (val == 0) { buf[i++] = '0'; buf[i] = '\0'; }
    else {
        while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
        buf[i] = '\0';
        // reverse
        for (int a = 0, b = i - 1; a < b; a++, b--) {
            char t = buf[a]; buf[a] = buf[b]; buf[b] = t;
        }
    }
    vga_print(x, y, buf, color);
}

//PCSPK 
static void sound_play(uint32_t freq) {
    uint32_t div = 1193180 / freq;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    outb(0x61, inb(0x61) | 0x03);
}

static void sound_stop(void) {
    outb(0x61, inb(0x61) & ~0x03);
}

static void beep(uint32_t freq, uint32_t ms) {
    sound_play(freq);
    sleep_busy(ms);
    sound_stop();
}

//Keyboard
#define KEY_W  0x11
#define KEY_S  0x1F
#define KEY_A  0x1E
#define KEY_D  0x20
#define KEY_ESC   0x01
#define KEY_ENTER 0x1C

extern uint32_t tick;
extern uint8_t get_last_scancode(void);
extern int suppress_keyboard_print;

static uint8_t read_scancode(void) {
    return get_last_scancode();
}
//RNG (seeded by PIT tick)
extern uint32_t tick;
static uint32_t rng_state = 12345;

static void rng_seed(void) { rng_state = tick ^ 0xDEADBEEF; }
static uint32_t rng_next(void) {
    rng_state = rng_state * 1664525 + 1013904223;
    return rng_state;
}

// Ring-buffer (adapted from serene-dev/snake-c)
// Original used: int x[1000], y[1000]; int head=0, tail=0;
// We keep the same structure, mapped to board coordinates.
#define SNAKE_BUF 1000
static int sx[SNAKE_BUF], sy[SNAKE_BUF];  // ring-buffer for snake body
static int head, tail;

// Apple (original called it applex/appley)
static int applex, appley;

// Direction (original used xdir/ydir)
static int xdir, ydir;

static int score;
static int gameover;
static int speed_ms;

//Drawing
static void draw_border(void) {
    uint8_t col = COLOR_CYAN;
    // top/bottom
    for (int x = 0; x <= COLS + 1; x++) {
        vga_put(BOARD_OFF_X + x, BOARD_OFF_Y,          '#', col);
        vga_put(BOARD_OFF_X + x, BOARD_OFF_Y + ROWS + 1, '#', col);
    }
    // sides
    for (int y = 0; y <= ROWS + 1; y++) {
        vga_put(BOARD_OFF_X,          BOARD_OFF_Y + y, '#', col);
        vga_put(BOARD_OFF_X + COLS + 1, BOARD_OFF_Y + y, '#', col);
    }
}

static void draw_hud(void) {
    vga_print(0, 0, "SNAKE  Score:", COLOR_WHITE);
    vga_print_int(14, 0, score, COLOR_YELLOW);
    vga_print(25, 0, "ESC=Quit  WASD=Move", COLOR_WHITE);
}

static void draw_cell(int x, int y, char c, uint8_t color) {
    // x,y are board coords (0..COLS-1, 0..ROWS-1)
    vga_put(BOARD_OFF_X + 1 + x, BOARD_OFF_Y + 1 + y, c, color);
}

static void draw_apple(void) {
    if (applex >= 0)
        draw_cell(applex, appley, '*', COLOR_RED);
}

static void draw_snake(void) {
    // Draw head
    draw_cell(sx[head], sy[head], '@', COLOR_WHITE);
    // Draw body
    for (int i = tail; i != head; i = (i + 1) % SNAKE_BUF)
        draw_cell(sx[i], sy[i], 'o', COLOR_GREEN);
}

static void erase_cell(int x, int y) {
    draw_cell(x, y, ' ', COLOR_BLACK);
}

//Game logic (adapted from serene-dev/snake-c)
static void place_apple(void) {
    // Original: applex = rand() % COLS; + collision check loop
    applex = -1;
    int attempts = 0;
    while (applex < 0 && attempts++ < 2000) {
        int ax = rng_next() % COLS;
        int ay = rng_next() % ROWS;
        int on_snake = 0;
        for (int i = tail; i != head; i = (i + 1) % SNAKE_BUF)
            if (sx[i] == ax && sy[i] == ay) { on_snake = 1; break; }
        if (sx[head] == ax && sy[head] == ay) on_snake = 1;
        if (!on_snake) { applex = ax; appley = ay; }
    }
}

static void snake_init(void) {
    // Original: head=0; tail=0; x[head]=COLS/2; y[head]=ROWS/2;
    head = 0;
    tail = 0;
    sx[head] = COLS / 2;
    sy[head] = ROWS / 2;

    xdir = 1; ydir = 0;   // original default: move right
    applex = -1;
    score = 0;
    gameover = 0;
    speed_ms = 150;

    rng_seed();
    place_apple();
}

static void snake_update(void) {
    // Original: clear tail cell, check apple, advance head
    int old_tail_x = sx[tail];
    int old_tail_y = sy[tail];

    if (sx[head] == applex && sy[head] == appley) {
        // Ate apple – don't advance tail (snake grows)
        applex = -1;
        score += 10;
        if (speed_ms > 80) speed_ms -= 10;
        beep(800, 50);
        place_apple();
    } else {
        // Erase tail, advance tail pointer
        erase_cell(old_tail_x, old_tail_y);
        tail = (tail + 1) % SNAKE_BUF;
    }

    // Advance head (original: x[newhead] = (x[head] + xdir + COLS) % COLS)
    int newhead = (head + 1) % SNAKE_BUF;
    sx[newhead] = sx[head] + xdir;
    sy[newhead] = sy[head] + ydir;
    head = newhead;

    // Wall collision (original wrapped around; we end game instead)
    if (sx[head] < 0 || sx[head] >= COLS || sy[head] < 0 || sy[head] >= ROWS) {
        gameover = 1;
        return;
    }

    // Self collision (original loop)
    for (int i = tail; i != head; i = (i + 1) % SNAKE_BUF) {
        if (sx[i] == sx[head] && sy[i] == sy[head]) {
            gameover = 1;
            return;
        }
    }
}

// Game over 
static void show_game_over(void) {
    beep(300, 100); beep(200, 100); beep(100, 200);

    int cx = BOARD_OFF_X + COLS / 2 - 5;
    int cy = BOARD_OFF_Y + ROWS / 2;
    vga_print(cx, cy,     "GAME OVER!", COLOR_RED);
    vga_print(cx, cy + 1, "Score: ",    COLOR_WHITE);
    vga_print_int(cx + 7, cy + 1, score, COLOR_YELLOW);
    vga_print(cx - 5, cy + 2, "ENTER=Restart  ESC=Quit", COLOR_WHITE);
}

//Main game loop (called from main.c)
void snake_game(void) {
      suppress_keyboard_print = 1;  // skru av printing
    int running = 1;

    while (running) {
        vga_clear();
        snake_init();
        draw_border();
        draw_hud();
        draw_apple();
        draw_snake();

        while (!gameover) {
            // Input
            uint8_t sc = read_scancode();
            if (sc == KEY_W    && ydir != 1)  { xdir = 0;  ydir = -1; }
            if (sc == KEY_S  && ydir != -1) { xdir = 0;  ydir = 1;  }
            if (sc == KEY_A  && xdir != 1)  { xdir = -1; ydir = 0;  }
            if (sc == KEY_D && xdir != -1) { xdir = 1;  ydir = 0;  }
            if (sc == KEY_ESC) { gameover = 1; running = 0; }

            snake_update();

            if (!gameover) {
                draw_border();
                draw_hud();
                draw_apple();
                draw_snake();
            }

            sleep_busy(speed_ms);
        }

        if (running) {
            show_game_over();
            while (1) {
                uint8_t sc = read_scancode();
                if (sc == KEY_ENTER) break;
                if (sc == KEY_ESC)  { running = 0; break; }
                sleep_busy(10);
            }
        }
    }
    vga_clear();
    suppress_keyboard_print = 0;
}