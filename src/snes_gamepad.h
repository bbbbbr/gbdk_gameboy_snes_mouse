#ifndef _SNES_GAMEPAD_H
#define _SNES_GAMEPAD_H

#include "common.h"

#define SNES_GAMEPAD_TX_LATCH  0b11111111
#define SNES_GAMEPAD_REPORT_LEN  2


// SNES GAMEPAD BUTTON BITS
//
// First byte                          // Signal order on wire
#define SNES_PAD_B       (1u << 7)     // 1
#define SNES_PAD_Y       (1u << 6)     // 2
#define SNES_PAD_SELECT  (1u << 5)     // 3
#define SNES_PAD_START   (1u << 4)     // 4
#define SNES_PAD_UP      (1u << 3)     // 5
#define SNES_PAD_DOWN    (1u << 2)     // 6
#define SNES_PAD_LEFT    (1u << 1)     // 7
#define SNES_PAD_RIGHT   (1u << 0)     // 8
// Second byte
#define SNES_PAD_A       (1u << 7)     // 9
#define SNES_PAD_X       (1u << 6)     // 10
#define SNES_PAD_L       (1u << 5)     // 11
#define SNES_PAD_R       (1u << 4)     // 12

// State indicates the meaning of the data in SB Reg
// when the SIO interrupt fires for a completed serial transfer
enum {
    SNES_GAMEPAD_STATE_LATCH,       // Ignore serial data during this state
    SNES_GAMEPAD_STATE_FIRST_BYTE,
    SNES_GAMEPAD_STATE_SECOND_BYTE, // Last transfer received, do not trigger another serial transfer
    SNES_GAMEPAD_STATE_DONE,
};


typedef struct snes_gamepad_t {
    uint8_t first_byte;
    uint8_t second_byte;
} snes_gamepad_t;

extern snes_gamepad_t snes_gamepad;

void snes_gamepad_blocking_wait_poll(void);

bool snes_gamepad_interrupt_data_ready(void);
void snes_gamepad_interrupt_read_start(void);
void snes_gamepad_interrupt_init(void);
void snes_gamepad_interrupt_deinstall(void);


#endif // _SNES_GAMEPAD_H
