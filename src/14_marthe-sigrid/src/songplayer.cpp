extern "C" {
    #include "song/song.h"
    #include "kernel/pit.h"
    #include "io.h"
    #include "libc/stdio.h"
}

extern uint32_t end;

#define SPEAKER_PORT 0x61
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND  0x43
#define PIT_BASE_FREQ 1193182

void enable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | 0x03); // Set bit 0 and bit 1
}

void disable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x03); // Clear bit 0 and bit 1
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) return;

    uint32_t divisor = PIT_BASE_FREQ / frequency;

    outb(PIT_COMMAND, 0xB6);
    outb(PIT_CHANNEL2, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker(); 
}

void stop_sound() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x02); // Clear bit 1 (speaker data)
}

extern "C" void play_song_impl(Song *song) {


    for (size_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];
        play_sound(note.frequency);
        sleep_interrupt(note.duration);
        stop_sound();
    }

    disable_speaker();
}

void play_song(Song *song) {
    play_song_impl(song);
}