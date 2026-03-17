#include "keyboard.h"
#include "irq.h"
#include "terminal.h"

/*
 * Hjelpefunksjon for å lese fra I/O-port.
 * Samme som i irq.c - ideelt sett burde disse ligge i en felles io.h,
 * men for enkelhets skyld dupliserer vi den her.
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/*
 * Scancode til ASCII lookup-tabell (US keyboard layout, set 1).
 *
 * Index = scancode, verdi = ASCII-tegn.
 * 0 betyr at tasten ikke har et printbart tegn (f.eks. Shift, Ctrl).
 *
 * Denne tabellen dekker bare vanlige taster uten Shift.
 * For å støtte store bokstaver og spesialtegn måtte vi
 * holdt styr på om Shift er trykket, men det dropper vi nå.
 */
static const char scancode_to_ascii[128] = {
    0,    27,  '1', '2', '3', '4', '5', '6',   /* 0x00 - 0x07 */
    '7', '8', '9', '0', '-', '=',  '\b', '\t',  /* 0x08 - 0x0F */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',    /* 0x10 - 0x17 */
    'o', 'p', '[', ']', '\n',  0,  'a', 's',    /* 0x18 - 0x1F */
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    /* 0x20 - 0x27 */
    '\'', '`',  0, '\\', 'z', 'x', 'c', 'v',   /* 0x28 - 0x2F */
    'b', 'n', 'm', ',', '.', '/',  0,  '*',     /* 0x30 - 0x37 */
     0,  ' ',  0,   0,   0,   0,   0,   0,      /* 0x38 - 0x3F */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x40 - 0x47 */
     0,   0,  '-',  0,   0,   0,  '+',  0,      /* 0x48 - 0x4F */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x50 - 0x57 */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x58 - 0x5F */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x60 - 0x67 */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x68 - 0x6F */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x70 - 0x77 */
     0,   0,   0,   0,   0,   0,   0,   0,      /* 0x78 - 0x7F */
};

/*
 * Enkel ringbuffer for å lagre tastatur-input.
 * Kan brukes seinere hvis vi vil lese input fra andre deler
 * av kernelen, men akkurat nå printer vi bare direkte.
 */
static char key_buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t buffer_pos = 0;

/*
 * IRQ 1 handler - kjøres hver gang tastaturet sender et interrupt.
 *
 * Leser scancode fra port 0x60, sjekker om det er et "key press"
 * event (ikke release), slår opp ASCII-verdien i tabellen,
 * og printer tegnet til skjermen.
 */
void keyboard_handler(struct isr_frame* frame)
{
    (void)frame;  /* Bruker ikke stackframen for tastatur */

    /* Les scancode fra tastaturet */
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /*
     * Bit 7 satt = tasten ble sluppet (release event).
     * Vi bryr oss bare om key press, så vi ignorerer releases.
     */
    if (scancode & 0x80) {
        return;
    }

    /* Slå opp ASCII-tegnet i lookup-tabellen */
    char c = scancode_to_ascii[scancode];

    /* Hvis det er et printbart tegn, vis det og legg i buffer */
    if (c != 0) {
        terminal_putchar(c);

        /* Lagre i ringbufferen */
        key_buffer[buffer_pos] = c;
        buffer_pos = (buffer_pos + 1) % KEYBOARD_BUFFER_SIZE;
    }
}

/*
 * Starter tastatur-driveren.
 * Registrerer keyboard_handler som handler for IRQ 1 (interrupt 33).
 * Etter dette vil hvert tastetrykk automatisk vises på skjermen.
 */
void keyboard_init(void)
{
    irq_register_handler(1, keyboard_handler);
}