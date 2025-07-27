#ifndef _SNES_MOUSE_H
#define _SNES_MOUSE_H

#include "common.h"

#define SNES_MOUSE_TX_LATCH  0b11111111
#define SNES_MOUSE_REPORT_LEN  4

// In Byte 2
#define SNES_MOUSE_BUTTON_LEFT  0b01000000u
#define SNES_MOUSE_BUTTON_RIGHT 0b10000000u
#define SNES_MOUSE_BUTTON_MASK  0b11000000u

// In Byte 3
#define SNES_MOUSE_Y_DIR   0b10000000u   // .7: 1 = Down, 0 = Up, 6..0: Movement
#define SNES_MOUSE_Y_MASK  0b01111111u

// In Byte 4
#define SNES_MOUSE_X_DIR   0b10000000u   // .7: 1 = Right, 0 = Left, 6..0: Movement
#define SNES_MOUSE_X_MASK  0b01111111u


typedef struct snes_mouse_t {
    uint8_t first_byte;  // Unused by mouse
    uint8_t buttons;
    uint8_t move_y;
    uint8_t move_x;
} snes_mouse_t;

extern snes_mouse_t snes_mouse;

// extern bool    sio_rx_ready;
// extern uint8_t sio_rx_buf[SNES_MOUSE_REPORT_LEN];


void snes_mouse_init(void);
void snes_mouse_start(void);
bool snes_mouse_new_data(void);

// void snes_mouse_poll(void) __naked;
void snes_mouse_poll(void);

#endif // _SNES_MOUSE_H
