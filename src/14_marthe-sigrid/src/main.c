#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "libc/stdint.h"
#include "libc/stdio.h"
#include "kernel/memory.h"
#include "kernel/pit.h"
#include "song/song.h"
#include "snake.h"

extern uint32_t end;
void play_song_impl(Song *song);

// Allocate a SongPlayer on the heap and wire it to the PC-speaker backend
SongPlayer* create_song_player() {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    player->play_song = play_song_impl;
    return player;
}

extern int suppress_keyboard_print;

// Menu entry point for the music option.
// Plays the songs back-to-back through the PC speaker.
// Keyboard echo is disabled so typed characters don't garble the screen
// while the songs are playing.
void play_music() {
    suppress_keyboard_print = 1;
    Song songs[] = {
        {music_2, sizeof(music_2) / sizeof(Note)},
        {starwars_theme, sizeof(starwars_theme) / sizeof(Note)},
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);
    SongPlayer* player = create_song_player();

    for(uint32_t i = 0; i < n_songs; i++) {
        printf("Playing Song...\n");
        player->play_song(&songs[i]);
        printf("Finished playing the song.\n");
    }
    suppress_keyboard_print = 0;
}

