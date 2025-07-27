#include <gbdk/platform.h>
#include <stdint.h>
#include <gb/isr.h>

#include <stdio.h>
#include <gbdk/console.h>

#include "common.h"

#include "snes_mouse.h"

// From:
// https://www.repairfaq.org/REPAIR/F_SNES.html
//
//
// I could not find a Nintendo numbering scheme, so I made one up.
// The view is looking back "into" the connector on the CABLE.
//
//  ----------------------------- ---------------------
// |                             |                      \
// | (1)     (2)     (3)     (4) |   (5)     (6)     (7) |
// |                             |                      /
//  ----------------------------- ---------------------
//
//
//   Pin     Description             Color of wire in cable    Game Boy Link Port
//   ===     ===========             ======================    =================
//   1       +5v                     White                     +5V
//   2       Data clock              Yellow                    SCLK
//   3       Data latch              Orange                    SOUT
//   4       Serial data             Red                       SIN
//   5       ?                       no wire
//   6       ?                       no wire
//   7       Ground                  Brown                     GND
//



// From:
// https://www.repairfaq.org/REPAIR/F_SNES.html
//
//
//                            |<------------16.67ms------------>|
//
//                            12us
//                         -->|   |<--
//
//                             ---                               ---
//                            |   |                             |   |
//  S-OUT-> Data Latch      ---     -----------------/ /----------    --------...
//
//
//  S-CLK-> Data clock      ----------   -   -   -  -/ /----------------   -  ...
//                                   | | | | | | | |                   | | | |
//                                    -   -   -   -                     -   -
//                                    1   2   3   4                     1   2
//
//  S-IN-> Serial Data         ----     ---     ----/ /           ---
//                            |    |   |   |   |                 |
//         (Buttons B      ---      ---     ---        ----------
//         & Select        norm      B      SEL           norm
//         pressed).       low                            low
//                                 12us
//                              -->|   |<--
//
//  The controllers serially shift the latched button states out pin 4
//  on every rising edge of the clock
//
//  And the CPU samples the data on every falling edge.


// GB 4194304 / 4 = 1,048,576 cycles per sec ... 1 cycle = 0.954us
// 1 second = 1,000,000us


// == Link port ==
//
// SCLK
// - Normally HI
// - Data updated on: HI -> LO clock transition (falling edge)
// - Data sampled on: LO -> HI clock transition (rising edge)


// Issues: need to invert clock
// - Could get signal inverter
//   - https://www.adafruit.com/product/3877
//
// - Could try using SD/P14 as clock and SCLK as latch strobe?




// Data is active low, so invert bits after reading controller data

void snes_mouse_init(void) {

}


void snes_mouse_start(void) {

}


bool snes_mouse_new_data(void) {

    return true;
}


// bool    sio_rx_ready = false;
// uint8_t sio_rx_buf[SNES_MOUSE_REPORT_LEN];
snes_mouse_t snes_mouse;

void snes_mouse_poll(void) {

    // Fake a overly long LATCH signal on S-OUT with a transfer of all bits = 1
    SB_REG = SNES_MOUSE_TX_LATCH;
    SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);

    uint8_t * p_snes_mouse = (uint8_t *) &snes_mouse;
    for (uint8_t c = 0u; c < SNES_MOUSE_REPORT_LEN; c++) {
        // Start a new transfer
        // No bits set in Serial Out byte to avoid disturbing the LATCH line
        SB_REG = 0u;
        SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;

        // Wait for transfer to complete
        while (SC_REG & SIOF_XFER_START);
        *p_snes_mouse++ = SB_REG;
    }
}


// Alternate approach, hook up normally unused SD pin
// (connected to P14)
// set or clear 4 bit on FF00 (d-pad matrix)
//
// P1_REG = 0x10u// Select d-pad


/*
void snes_mouse_poll(void) __naked {
    __asm \
        push AF
        push BC
        push HL

        // Here we're going to hope that SNES Controller DATA CLOCK
        ld   a, #SNES_MOUSE_TX_LATCH
        ldh  (_SB_REG), a

        ld   a, #(SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT)
        ldh  (_SC_REG), a

        // Wait at least 12 microseconds, meaning about ~ 13 M-Cycles
        // and then abort the transfer

        // This is too short and seems to result in unreliable Latch strobes
        // .rept (13 - 2 - 2)  // - 4 for the 4 M-Cycles below to stop SIO
        // .rept (128)  // 183 usec
        .rept (60)  // 183 usec
            nop
        .endm
        // Early abort the transfer
        //
        // However: The HI state on S-OUT doesn't go low until the next transfer
        //          below starts
        ld   a, #SIOF_CLOCK_INT
        ldh  (_SC_REG), a

        .rept (100)
            nop
        .endm

        // Read 4 bytes from serial
        ld  c,  #SNES_MOUSE_REPORT_LEN
        ld  hl, #_sio_rx_buf
        .readloop:
            // Make sure SB_REG is all zeros since it's wired up to the SNES pad latch line
            xor  a
            ldh  (_SB_REG), a

            ld   a, #(SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT)
            ldh  (_SC_REG), a

            // Wait for serial transfer to complete
            .tx_loop:
                ldh  a, (_SC_REG)
                bit  #SIOF_B_XFER_START, a
                jr   nz, .tx_loop

            // Save received serial byte
            ldh  a, (_SB_REG)
            ld  (hl+), a

            .rept (100)
                nop
            .endm


            dec c
            jr  nz, .readloop

        pop HL
        pop BC
        pop AF

        ret
    __endasm;
}
*/
