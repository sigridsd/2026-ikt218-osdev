#include "terminal.h"
#include "string.h"
#include "shell.h"
#include "memory.h"
#include "song/song.h"
#include "pit.h"

static uint32_t parse_uint(const char* str) {
    uint32_t result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

Song mario = {
    .notes = starwars_theme,
    .length = sizeof(starwars_theme) / sizeof(Note)
};

//possible to add more commands for help here
void command_help(){
    terminal_write("\nCommands avalible:\nclear\nhello\nmemory\ntriangle\nsleep_b <value>\nsleep_i <value>\nhelp\n");
}

void command_triangle() {
    terminal_write("\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEWEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEv iIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAWv   rEAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAA6v      RAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAE0        v1AAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAm           0AAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAIv             iAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAARv               vIAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAWv                 vWAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAA0                     WAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAEr                       1AAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAANm                         iAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAANr                           rIAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
}
void command_hello() {
    terminal_write("\nHello, this is an orange shell terminal made by Herman and Oscar");
}

void command_clear() {
    terminal_initialize();
    terminal_update_cursor();
}
void command_memory(uint32_t mb2_info) {
    print_heap_info();
    print_memory_layout(mb2_info);
}
void command_sleep_busy(const char* args) {
    __asm__("sti");
    uint32_t seconds = parse_uint(args);
    terminal_write("\n");
    if (seconds == 0) {
        terminal_write("\nUsage: sleep_busy <seconds>\n");
        return;
    }

    for (uint32_t i = 1; i <= seconds; i++) {
        terminal_write_dec(i);
        terminal_write("\n");
        sleep_busy(1000);
    }
    terminal_write("Done.\n");
}

void command_sleep_interrupt(const char* args) {
    uint32_t seconds = parse_uint(args);
    terminal_write("\n");
    if (seconds == 0) {
        terminal_write("\nUsage: sleep_interrupt <seconds>\n");
        return;
    }

    for (uint32_t i = 1; i <= seconds; i++) {
        terminal_write_dec(i);
        terminal_write("\n");
        sleep_interrupt(1000);
    }
    terminal_write("Done.\n");
}
void command_song() {
    terminal_write("\nNow playing mario\n");
    play_song(&mario);
}

void shell_execute_command(char* input) {
    if(strlen(input) == 0) {
        return;
    }

    char* cmd = input;
    char* args = "";
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ' ') {
            input[i] = '\0';      // null-terminate the command part
            args = &input[i + 1]; // args starts after the space
            break;
        }
    }

    if(strcmp(input, "clear") == 0) {
        command_clear();
    }
    else if(strcmp(input, "hello") == 0) {
        command_hello();
    }
    else if(strcmp(input, "help") == 0) {
        command_help();
    }
    else if(strcmp(input, "memory") == 0) {
        command_memory(g_mb2_info);
    }
    else if(strcmp(input, "triangle") == 0) {
        command_triangle();
    }
    else if(strcmp(cmd, "sleep_b") == 0) {
        command_sleep_busy(args);
    }
    else if(strcmp(cmd, "sleep_i") == 0) {
        command_sleep_interrupt(args);
    }
    else if(strcmp(input, "song") == 0) {
        command_song();
    }
    else {
        terminal_write("\n");
        terminal_write(input);
        terminal_write(" is not a command, write help for avalible commands\n");
    }
}