// little PC speaker song player. drives PIT channel 2 to make tones

extern "C" {
    #include "song/song.h"
    #include "kernel/pit.h"
    #include "io.h"
    #include "libc/stdio.h"
}

#define SPEAKER_PORT 0x61
#define PIT_CHANNEL2 0x42
#define PIT_COMMAND  0x43
#define PIT_BASE_FREQ 1193182

static void enable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | 0x03);
}

static void disable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x03);
}

// kicks off a tone at `frequency` Hz. PIT divides its base clock to get there.
static void play_sound(uint32_t frequency) {
    if (frequency == 0) return; // 0 = a rest, no sound
    uint32_t divisor = PIT_BASE_FREQ / frequency;
    outb(PIT_COMMAND, 0xB6);  // channel 2, lo/hi byte, mode 3 (square wave)
    outb(PIT_CHANNEL2, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2, (uint8_t)((divisor >> 8) & 0xFF));
    enable_speaker();
}

static void stop_sound() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x02);
}

// walk the notes one by one: tone on, sleep for the duration, tone off
extern "C" void play_song_impl(Song *song) {
    for (size_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];
        play_sound(note.frequency);
        sleep_interrupt(note.duration);
        stop_sound();
    }
    disable_speaker();
}
