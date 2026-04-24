# Assignment Five - Music Player

## Overview

This assignment implements a music player that drives the PC Speaker (PCSPK) using PIT channel 2 as a tone generator. Notes are defined as frequency-duration pairs, grouped into songs, and played back through a `SongPlayer` abstraction backed by `malloc` from the heap built in assignment four.

The PIT already runs at 1000 Hz on channel 0 to drive `sleep_interrupt`, which provides millisecond-accurate note timing. Channel 2 is a second, independent PIT counter wired directly to the speaker cone — programming it to a different divisor produces an audible tone at the target frequency.

---

## Files Changed or Created

| File | Role |
|------|------|
| `src/music_player/frequencies.h` | Provided — note frequency macros for octaves 0-9 |
| `src/music_player/song.h` | Provided — `Note`, `Song`, `SongPlayer` structs and song data; fixed broken include |
| `src/music_player/song_player.c` | New — PCSPK hardware functions and song playback logic |
| `src/kernel.c` | Replaced sleep test loop with `play_music()` |
| `CMakeLists.txt` | Added `src/music_player/song_player.c` to the build target |

---

## Hardware Background — PCSPK and PIT Channel 2

### Why PIT channel 2 and not channel 0

Channel 0 is already claimed by the system tick (IRQ0 at 1000 Hz). Reprogramming it would break `sleep_interrupt` and destroy the time base. The PC speaker is hardwired to **channel 2** of the same 8253/8254 chip. Channel 2 is completely independent — it has its own counter, its own gate, and its own output line — so it can be set to any audio frequency without affecting the tick clock.

### Port map

| Port | Name | Purpose |
|------|------|---------|
| `0x43` | PIT command port | Send control word to select channel and mode |
| `0x42` | PIT channel 2 data port | Write the 16-bit frequency divisor (low byte first) |
| `0x61` | PC speaker control | Bit 0: gate (connect PIT ch2 clock); Bit 1: data (connect PIT ch2 output to cone) |

### Frequency calculation

The PIT input clock is fixed at 1 193 180 Hz. To produce a tone at frequency `f`:

```
divisor = 1 193 180 / f
```

The divisor is written to port `0x42` as two bytes, low byte first. The speaker cone vibrates at `f` Hz, which the ear perceives as a musical note.

### Command byte 0xB6

```
0xB6 = 1011 0110
       ^^          channel 2
         ^^        access mode: lobyte then hibyte
           ^^^     mode 3: square wave generator
              ^    binary counting
```

Square wave mode keeps the output toggling at the programmed frequency for as long as the divisor is loaded. No further CPU involvement is needed during playback.

### Speaker control — port 0x61

Bit 1 connects the PIT channel 2 output to the speaker cone. Bit 0 gates the PIT channel 2 clock. Both must be set to produce sound. Clearing bit 1 silences the speaker immediately without reprogramming the PIT, which is used between notes to produce clean gaps.

---

## Data Structures

### `Note`

```c
typedef struct {
    uint32_t frequency;  // Hz; 0 means silence (rest)
    uint32_t duration;   // milliseconds
} Note;
```

A rest is represented by `frequency = 0` (`R` macro in `frequencies.h`). `play_sound` checks for zero and returns without touching the hardware, so the speaker stays silent for the note's duration.

### `Song`

```c
typedef struct {
    Note    *notes;   // pointer to a Note array
    uint32_t length;  // number of notes
} Song;
```

Songs are defined as static arrays in `song.h` and passed by pointer. `length` is computed with `sizeof(array) / sizeof(Note)` at the call site, keeping the struct independent of any fixed size.

### `SongPlayer`

```c
typedef struct {
    void (*play_song)(Song *song);
} SongPlayer;
```

A function-pointer struct allocated on the heap via `create_song_player`. The indirection matches the assignment interface and makes it straightforward to swap playback implementations later.

---

## Implementation — `song_player.c`

### `enable_speaker` / `disable_speaker`

Read port `0x61`, OR or AND the relevant bits, write back. Reading before writing preserves any other bits in the register that the firmware may have set.

```c
static void enable_speaker(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | 0x03);   // set bits 0 and 1
}

static void disable_speaker(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x03);  // clear bits 0 and 1
}
```

### `play_sound`

Guards on zero frequency (rest), computes the divisor, programs channel 2, then enables the speaker. The command byte `0xB6` is sent to `0x43` on every call to ensure the PIT is in the correct mode even if something else touched it.

```c
static void play_sound(uint32_t frequency) {
    if (frequency == 0) return;

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;
    outb(PIT_CMD_PORT, PIT_CH2_CMD);
    outb(PIT_CH2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH2_PORT, (uint8_t)((divisor >> 8) & 0xFF));
    enable_speaker();
}
```

### `stop_sound`

Clears only bit 1, which disconnects the PIT output from the cone. The PIT continues counting, but nothing is driving the speaker. This leaves bit 0 set, so re-enabling for the next note only requires setting bit 1 again via `enable_speaker`.

### `play_song_impl`

Enables the speaker once before the loop, then for each note: plays the sound, waits using `sleep_interrupt`, and stops the sound. `sleep_interrupt` uses the PIT channel 0 tick (IRQ0) to halt the CPU between ticks, so the kernel is not busy-spinning for the duration of every note.

```
enable_speaker()
for each note:
    play_sound(note->frequency)   <- programs PIT ch2; no-op if frequency == 0
    sleep_interrupt(note->duration)
    stop_sound()                  <- clears bit 1, silences cone
disable_speaker()
```

### `create_song_player`

Allocates a `SongPlayer` from the kernel heap and sets the `play_song` function pointer. Uses `malloc` from the bump allocator built in assignment four.

---

## `play_music` in `kernel.c`

Defines the song list as a local array of `Song` structs, creates a player, then loops indefinitely through all songs.

```c
static void play_music(void) {
    Song songs[] = {
        {music_1,                sizeof(music_1)                / sizeof(Note)},
        {starwars_theme,         sizeof(starwars_theme)         / sizeof(Note)},
        {battlefield_1942_theme, sizeof(battlefield_1942_theme) / sizeof(Note)},
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);
    SongPlayer *player = create_song_player();

    while (1) {
        for (uint32_t i = 0; i < n_songs; i++) {
            printf("Playing song %d...\n", i + 1);
            player->play_song(&songs[i]);
            printf("Done.\n");
        }
    }
}
```

---

## Boot Sequence (Final)

```
_start (multiboot2.asm)
  └─ main (kernel.c)
       ├─ gdt_init()
       ├─ terminal_init()
       ├─ idt_init()
       ├─ [ISR tests]
       ├─ init_kernel_memory(&end)
       ├─ init_paging()
       ├─ print_memory_layout()
       ├─ malloc() x 3
       ├─ pic_init()
       ├─ keyboard_init()
       ├─ init_pit()               — channel 0 at 1000 Hz, IRQ0 enabled
       ├─ sti
       └─ play_music()
            └─ play_song_impl()
                 ├─ enable_speaker()
                 ├─ for each note:
                 │    ├─ play_sound()        — programs PIT channel 2
                 │    ├─ sleep_interrupt()   — halts on IRQ0 ticks for timing
                 │    └─ stop_sound()        — silences cone between notes
                 └─ disable_speaker()
```

---

## Key References

- OSDev Wiki - [PC Speaker](https://wiki.osdev.org/PC_Speaker)
- OSDev Wiki - [Programmable Interval Timer](https://wiki.osdev.org/Programmable_Interval_Timer)
- IBM PC Technical Reference - Port 0x61 speaker control register

---

## Update - Playback Control

### What changed

The initial implementation looped indefinitely through all songs. This was replaced with a single play-through followed by an idle-wait loop that lets the user restart playback by pressing SPACE.

### Files changed

| File | Change |
|------|--------|
| `include/keyboard.h` | Added `keyboard_getchar` declaration |
| `src/keyboard.c` | Implemented `keyboard_getchar` |
| `src/kernel.c` | `play_music` plays once and frees the player; `main` enters idle loop after |

### `keyboard_getchar`

The existing `keyboard_poll` drains the ring buffer and prints every character - it has no way to return a specific key to the caller. A new function dequeues one scancode, translates it, and returns the char (or `0` if the buffer is empty):

```c
char keyboard_getchar(void) {
    if (buf_tail == buf_head)
        return 0;
    uint8_t scancode = scancode_buf[buf_tail++];
    return scancode_table[scancode];
}
```

This keeps the ring buffer and translation table entirely inside `keyboard.c` and gives callers a clean interface without exposing internals.

### `play_music` - single play-through

The outer `while(1)` loop was removed. After all songs finish, the player is freed and a prompt is printed:

```c
free(player);
printf("Player stopped. Press SPACE to play again.\n");
```

`free` is a no-op on the current bump allocator but is correct to call so the code is ready if the allocator is ever upgraded.

### Idle-wait loop in `main`

After `play_music` returns, `main` enters a low-CPU idle loop. The CPU halts on each iteration and wakes on any IRQ (keyboard or PIT tick). If the dequeued key is SPACE, `play_music` is called again:

```c
for (;;) {
    asm volatile("sti; hlt");
    if (keyboard_getchar() == ' ')
        play_music();
}
```

`sti` precedes `hlt` for the same reason as `sleep_interrupt` - the interrupt gate clears IF on entry, so IF may be clear when the handler returns. The pair guarantees at least one interrupt is served before the next halt.
