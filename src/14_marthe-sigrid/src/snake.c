/*
 * Snake!
 * The game logic (ring buffer for the body, apple placement, collision
 * checks) is borrowed from https://github.com/serene-dev/snake-c.
 * We just rip out all the libc I/O and talk to the VGA buffer, PC
 * speaker, and keyboard IRQ directly since we're running on bare metal.
 */

#include "../include/snake.h"
#include "../include/libc/stdint.h"
#include "../include/kernel/pit.h"
#include "../include/io.h"

// VGA text mode lives at 0xB8000, 80x25 grid of (char, attr) pairs
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
        // pull digits out from the back, then flip the buffer
        while (val > 0) { buf[i++] = '0' + (val % 10); val /= 10; }
        buf[i] = '\0';
        for (int a = 0, b = i - 1; a < b; a++, b--) {
            char t = buf[a]; buf[a] = buf[b]; buf[b] = t;
        }
    }
    vga_print(x, y, buf, color);
}

// PC speaker stuff for apple-eat blip and game-over jingle
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

// keyboard scancodes for the keys we care about (PS/2 set 1)
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

// dead simple LCG. seeded with the PIT tick so it's a bit different each run
extern uint32_t tick;
static uint32_t rng_state = 12345;

static void rng_seed(void) { rng_state = tick ^ 0xDEADBEEF; }
static uint32_t rng_next(void) {
    rng_state = rng_state * 1664525 + 1013904223;
    return rng_state;
}

// Snake body lives in a ring buffer (head grows, tail shrinks each step).
// Same structure as the original snake-c, just renamed to fit our coords.
#define SNAKE_BUF 1000
static int sx[SNAKE_BUF], sy[SNAKE_BUF];
static int head, tail;

// where the apple is right now (-1 in applex means none placed yet)
static int applex, appley;

// movement direction in cells per step
static int xdir, ydir;

static int score;
static int gameover;
static int speed_ms;
static int ate_apple;

// draw the wall of '#' around the play field
static void draw_border(void) {
    uint8_t col = COLOR_CYAN;
    // top + bottom rows
    for (int x = 0; x <= COLS + 1; x++) {
        vga_put(BOARD_OFF_X + x, BOARD_OFF_Y,          '#', col);
        vga_put(BOARD_OFF_X + x, BOARD_OFF_Y + ROWS + 1, '#', col);
    }
    // left + right sides
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

// translates from board coords (0..COLS-1, 0..ROWS-1) to actual VGA cells
static void draw_cell(int x, int y, char c, uint8_t color) {
    vga_put(BOARD_OFF_X + 1 + x, BOARD_OFF_Y + 1 + y, c, color);
}

static void draw_apple(void) {
    if (applex >= 0)
        draw_cell(applex, appley, '*', COLOR_RED);
}

static void draw_snake(void) {
    // head is white '@', the rest of the body is green 'o'
    draw_cell(sx[head], sy[head], '@', COLOR_WHITE);
    for (int i = tail; i != head; i = (i + 1) % SNAKE_BUF)
        draw_cell(sx[i], sy[i], 'o', COLOR_GREEN);
}

static void erase_cell(int x, int y) {
    draw_cell(x, y, ' ', COLOR_BLACK);
}

// keep rolling random spots until we land on one that isn't on the snake.
// the attempt cap is just a safety net so we never spin forever.
static void place_apple(void) {
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

// reset everything for a fresh round: snake in the middle, moving right
static void snake_init(void) {
    head = 0;
    tail = 0;
    sx[head] = COLS / 2;
    sy[head] = ROWS / 2;

    xdir = 1; ydir = 0;
    applex = -1;
    score = 0;
    gameover = 0;
    speed_ms = 150;

    rng_seed();
    place_apple();
}

// one game tick: handle apple, move the head, check for crashes
static void snake_update(void) {
    int old_tail_x = sx[tail];
    int old_tail_y = sy[tail];

    if (sx[head] == applex && sy[head] == appley) {
        // got the apple, snake grows so we skip moving the tail
        applex = -1;
        score += 10;
        if (speed_ms > 80) speed_ms -= 10;  // speed up a little each apple
        ate_apple = 1;
        place_apple();
    } else {
        // normal move: wipe old tail and shift the tail pointer
        erase_cell(old_tail_x, old_tail_y);
        tail = (tail + 1) % SNAKE_BUF;
    }

    // push the head one cell forward
    int newhead = (head + 1) % SNAKE_BUF;
    sx[newhead] = sx[head] + xdir;
    sy[newhead] = sy[head] + ydir;
    head = newhead;

    // ran into a wall? (the original snake-c wrapped, we don't)
    if (sx[head] < 0 || sx[head] >= COLS || sy[head] < 0 || sy[head] >= ROWS) {
        gameover = 1;
        return;
    }

    // ran into ourselves?
    for (int i = tail; i != head; i = (i + 1) % SNAKE_BUF) {
        if (sx[i] == sx[head] && sy[i] == sy[head]) {
            gameover = 1;
            return;
        }
    }
}

// game-over splash + a sad descending jingle
static void show_game_over(void) {
    int cx = BOARD_OFF_X + COLS / 2 - 5;
    int cy = BOARD_OFF_Y + ROWS / 2;
    vga_print(cx, cy,     "GAME OVER!", COLOR_RED);
    vga_print(cx, cy + 1, "Score: ",    COLOR_WHITE);
    vga_print_int(cx + 7, cy + 1, score, COLOR_YELLOW);
    vga_print(cx - 5, cy + 2, "ENTER=Restart  ESC=Quit", COLOR_WHITE);

    beep(300, 100); beep(200, 100); beep(100, 200);
}

// entry point from kernel.c. runs until the player ESC's out.
void snake_game(void) {
      suppress_keyboard_print = 1;  // don't let the IRQ echo keys to the screen mid-game
    int running = 1;

    while (running) {
        vga_clear();
        snake_init();
        draw_border();
        draw_hud();
        draw_apple();
        draw_snake();

        while (!gameover) {
            // input: WASD turns, ESC quits. extra check to stop instant 180s.
            uint8_t sc = read_scancode();
            if (sc == KEY_W    && ydir != 1)  { xdir = 0;  ydir = -1; }
            if (sc == KEY_S  && ydir != -1) { xdir = 0;  ydir = 1;  }
            if (sc == KEY_A  && xdir != 1)  { xdir = -1; ydir = 0;  }
            if (sc == KEY_D && xdir != -1) { xdir = 1;  ydir = 0;  }
            if (sc == KEY_ESC) { gameover = 1; running = 0; }

            ate_apple = 0;
            snake_update();

            if (!gameover) {
                draw_border();
                draw_hud();
                draw_apple();
                draw_snake();
            }

            if (ate_apple) {
                // little blip when we eat an apple. don't double up on the
                // wait, so subtract the blip time from the normal step delay.
                sound_play(800);
                sleep_busy(50);
                sound_stop();
                sleep_busy(speed_ms > 50 ? speed_ms - 50 : 0);
            } else {
                sleep_busy(speed_ms);
            }
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