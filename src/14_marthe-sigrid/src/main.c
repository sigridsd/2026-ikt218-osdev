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

SongPlayer* create_song_player() {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    player->play_song = play_song_impl;
    return player;
}

void play_music() {
    Song songs[] = {
        {starwars_theme, sizeof(starwars_theme) / sizeof(Note)},
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);
    SongPlayer* player = create_song_player();

    for(uint32_t i = 0; i < n_songs; i++) {
        printf("Playing Song...\n");
        player->play_song(&songs[i]);
        printf("Finished playing the song.\n");
    }
}

