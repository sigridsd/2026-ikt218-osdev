#include <libc/stdint.h>
#include <kernel/terminal.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/interrupt.h>
#include <kernel/keyboard.h>
#include <kernel/memory.h>
#include <kernel/pit.h>
#include <songApp/song.h>
#include <songApp/frequencies.h>
#include <snakeApp/snake.h>

extern uint32_t end;

void PlayMusic(void) {
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)},
        {music_2, sizeof(music_2) / sizeof(Note)},
        {music_3, sizeof(music_3) / sizeof(Note)},
        {music_4, sizeof(music_4) / sizeof(Note)},
        {music_5, sizeof(music_5) / sizeof(Note)},
        {music_6, sizeof(music_6) / sizeof(Note)}
    };

    uint32_t songCount = sizeof(songs) / sizeof(Song);

    SongPlayer* player = CreateSongPlayer();

    if (!player) {
        TerminalWriteString("Failed to create SongPlayer.\n");
        return;
    }

    for(uint32_t i = 0; i < songCount; i++) {
        player->play_song(&songs[i]);
        SleepInterrupt(1000);
    }
}

void main(void) {
    TerminalInitialize();
    GdtInitialize();
    IdtInitialize();
    PitInitialize();

    RegisterInterruptHandler(IRQ1, KeyboardHandler);

    InitKernelMemory(&end);
    InitPaging();
    PrintMemoryLayout();

    while (1) {
        TerminalClear();
        TerminalWriteString("Enter application number (0 for music, 1 for snake): ");
        char input = TerminalGetChar();

        switch (input) {
            case '0':
                PlayMusic();
                break;
            case '1':
                PlayGame();
                break;
            default:
                TerminalWriteString("Invalid application number.\n");
                break;
        }
    }

    for (;;) {
        __asm__ volatile("hlt");
    }
}