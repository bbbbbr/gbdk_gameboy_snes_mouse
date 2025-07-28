#include <gbdk/platform.h>
#include <stdint.h>
#include <gb/isr.h>

#include <stdio.h>
#include <gbdk/console.h>

#include "common.h"

#include "snes_mouse.h"


/*
  SNES CONTROLLER                             GAME BOY
  *PORT*                                     LINK *PORT*
   _
  / \
 | 7 | GND         -   GB_Link.GND   [6]
 | 6 |
 | 5 |
 |---|                                           _______
 | 4 | Data  (out)  -> GB_Link.S-IN      [3]    /       \
 | 3 | Latch (in)  <-  GB_Link.S-OUT(*2) [2]   | 5  3  1 |
 | 2 | Clock (in)  <-  GB_Link.S-CLK(*1) [5]   | 6  4  2 |
 | 1 | 5v    (in)  <-  GB_Link.5v        [1]   |_________|
 |___|

*1: GB_Link.S-CLK should probably have an inverter on it,
    works on DMG (only) without one despite that.

*2: The problem with using GB_Link.S-OUT to trigger the controller
    protocol latch is that the clock runs during that triggering
    the sensitivity adjustment each time the mouse is polled.
    GB_Link.4(S-Data) might work as an alternative, controlled
    via bit 4 (d-pad select) of the joypad register.
*/



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


// == Link port ==
//
// SCLK
// - Normally HI
// - Data updated on: HI -> LO clock transition (falling edge)
// - Data sampled on: LO -> HI clock transition (rising edge)


// Issues: need to invert clock
// - Could get signal inverter
//
// - Could try using SD/P14 as clock and SCLK as latch strobe?
//
// - Could do detection of Mouse vs not mouse based on the last
//   bit clocked of the second byte read being low (i.e. bit 16 == 0)


// On the DMG each subsequent poll of the mouse triggers a change in
// the sensitivity setting, likely due to the clock (S-CLK) continuing
// to run during the simulated latch behavior on S-OUT. Ref:
//
// Switching sensitivity mode: First, a normal 12us latch pulse, 
// next the first 16 bits are read using normal button timings.
// Shortly after (about 1ms), 31 short latch pulses (3.4uS) are sent, with the
// clock going low for 700ns during each latch pulse.
// For selecting a specific sensitivity, simply execute the 
// special sequence until bits 11 and 12 are as desired.



// Poll the SNES mouse with 1 byte worth of latch and 4 bytes of data
//
// This is a simplistic implementation with blocking waits for serial transfers
// to finish. In actual use it would be better to make it interrupt driven upon
// completion of each serial transfer.
//
snes_mouse_t snes_mouse;

void snes_mouse_poll(void) {

    // Fake a overly long LATCH signal on S-OUT with a transfer of all bits = 1
    SB_REG = SNES_CONTROLLER_TX_LATCH;
    SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);

    uint8_t * p_snes_mouse = (uint8_t *) &snes_mouse;
    for (uint8_t c = 0u; c < SNES_MOUSE_REPORT_LEN; c++) {
        // Start another transfer
        // No bits set in Serial Out byte to avoid disturbing the LATCH line
        SB_REG = 0u;
        SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;

        // Wait for transfer to complete
        while (SC_REG & SIOF_XFER_START);
        // Save incoming data (which is active low so invert bits)
        *p_snes_mouse++ = ~SB_REG;
    }
}


// Poll the SNES gamepad with 1 byte worth of latch and 2 bytes of data
//
snes_gamepad_t snes_gamepad;

void snes_gamepad_poll(void) {

    // Fake a overly long LATCH signal on S-OUT with a transfer of all bits = 1
    SB_REG = SNES_CONTROLLER_TX_LATCH;
    SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);

    uint8_t * p_snes_gamepad = (uint8_t *) &snes_gamepad;
    for (uint8_t c = 0u; c < SNES_GAMEPAD_REPORT_LEN; c++) {
        // Start another transfer
        // No bits set in Serial Out byte to avoid disturbing the LATCH line
        SB_REG = 0u;
        SC_REG = SIOF_XFER_START | SIOF_SPEED_32X | SIOF_CLOCK_INT;

        // Wait for transfer to complete
        while (SC_REG & SIOF_XFER_START);
        // Save incoming data (which is active low so invert bits)
        *p_snes_gamepad++ = ~SB_REG;
    }
}


