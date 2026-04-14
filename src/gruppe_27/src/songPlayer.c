#include "song/song.h"
#include "pit.h"
#include "common.h"
#include "libc/stdio.h"
#include "terminal.h"

// PC speaker control port bit masks
#define SPEAKER_GATE_BIT    0x01    // Bit 0: connects PIT channel 2 to speaker
#define SPEAKER_DATA_BIT    0x02    // Bit 1: enables speaker output


void enable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state | SPEAKER_GATE_BIT | SPEAKER_DATA_BIT);
}

void disable_speaker() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~(SPEAKER_GATE_BIT | SPEAKER_DATA_BIT));
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) return; // R = rest, no sound

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    // Configure PIT channel 2: channel 2, lobyte/hibyte, mode 3 (square wave)
    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound() {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~SPEAKER_DATA_BIT);
}

void play_song_impl(Song *song) {
    for (uint32_t i = 0; i < song->length; i++) {
        Note *note = &song->notes[i];

        if (note->frequency == 0) {
            stop_sound();
            sleep_interrupt(note->duration);
        } else {
            play_sound(note->frequency);
            
            // Spill noten i 90% av tiden for å skille dem fra hverandre
            uint32_t play_time = (note->duration * 9) / 10;
            uint32_t pause_time = note->duration - play_time;

            sleep_interrupt(play_time);
            stop_sound(); // En kort pause gjør at man hører hver note tydelig
            if (pause_time > 0) {
                sleep_interrupt(pause_time);
            }
        }
    }
    disable_speaker();
}

void play_song(Song *song) {
    play_song_impl(song);
}