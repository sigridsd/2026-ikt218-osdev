// PC-speaker song player.
// Drives the speaker by programming PIT channel 2 to the desired tone
// frequency and then toggling the gate bits in port 0x61.

extern "C" {
    #include "song/song.h"
    #include "kernel/pit.h"
    #include "io.h"
    #include "libc/stdio.h"
}

#define SPEAKER_PORT 0x61    // keyboard controller port B; bits 0/1 control the speaker
#define PIT_CHANNEL2 0x42    // PIT channel 2 data port (wired to the speaker)
#define PIT_COMMAND  0x43    // PIT mode/command register
#define PIT_BASE_FREQ 1193182 // PIT base oscillator frequency in Hz

// Set both gate bits so the PIT square wave drives the speaker
static void enable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | 0x03);
}

// Clear both gate bits to silence the speaker between songs
static void disable_speaker() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x03);
}

// Program PIT channel 2 to emit a square wave at the given frequency
// and connect it to the speaker. A frequency of 0 means "rest" and is
// handled by the caller via sleep_interrupt.
static void play_sound(uint32_t frequency) {
    if (frequency == 0) return;
    uint32_t divisor = PIT_BASE_FREQ / frequency;
    outb(PIT_COMMAND, 0xB6);                          // channel 2, lobyte/hibyte, mode 3 (square wave)
    outb(PIT_CHANNEL2, (uint8_t)(divisor & 0xFF));    // low byte of divisor
    outb(PIT_CHANNEL2, (uint8_t)((divisor >> 8) & 0xFF)); // high byte of divisor
    enable_speaker();
}

// Disconnect the speaker from PIT channel 2 (clears bit 1 of port 0x61)
static void stop_sound() {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x02);
}

// Play a song one note at a time.
// Each note is held for note.duration milliseconds via sleep_interrupt
// (which uses HLT, so the CPU idles between PIT ticks).
extern "C" void play_song_impl(Song *song) {
    for (size_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];
        play_sound(note.frequency);
        sleep_interrupt(note.duration);
        stop_sound();
    }
    disable_speaker();
}
