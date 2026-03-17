#pragma once

#include "libc/stdint.h"
#include "isr.h"

/*
 * PS/2 tastatur-driver.
 *
 * Tastaturet sender scancodes via IRQ 1 hver gang en tast trykkes
 * eller slippes. Vi leser scancode fra I/O-port 0x60, oversetter
 * den til ASCII med en lookup-tabell, og skriver tegnet til skjermen.
 *
 * Scancodes over 0x80 betyr at tasten ble sluppet (release),
 * de bryr vi oss ikke om foreløpig.
 */

/* I/O-port for å lese scancode fra tastaturet */
#define KEYBOARD_DATA_PORT 0x60

/* Størrelsen på tastatur-bufferet */
#define KEYBOARD_BUFFER_SIZE 256

/* Initialiserer tastatur-driveren og registrerer IRQ 1 handler */
void keyboard_init(void);

/* IRQ 1 handler - kalles automatisk når en tast trykkes */
void keyboard_handler(struct isr_frame* frame);