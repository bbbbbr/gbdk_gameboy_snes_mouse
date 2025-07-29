#include <gbdk/platform.h>
#include <stdint.h>
#include <gb/isr.h>

#include <stdio.h>
#include <gbdk/console.h>

#include "common.h"

#include "snes_gamepad.h"



snes_gamepad_t snes_gamepad;


uint8_t snes_gamepad_state;
bool    snes_gamepad_data_ready;
uint8_t *p_snes_gamepad_data;

// ========== GamePad Read Blocking Wait Poll Version ==========

// Poll the SNES gamepad with 1 byte worth of latch and 2 bytes of data
//
//
// This is a simplistic implementation with blocking waits for serial transfers
// to finish. In actual use it's almost always better to use the interrupt version
// which has much lower cpu use.

void snes_gamepad_blocking_wait_poll(void) {

    // Fake a overly long LATCH signal on S-OUT with a transfer of all bits = 1
    SB_REG = SNES_GAMEPAD_TX_LATCH;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);

    uint8_t * p_snes_gamepad = (uint8_t *) &snes_gamepad;
    for (uint8_t c = 0u; c < SNES_GAMEPAD_REPORT_LEN; c++) {
        // Start another transfer
        // No bits set in Serial Out byte to avoid disturbing the LATCH line
        SB_REG = 0u;
        SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;

        // Wait for transfer to complete
        while (SC_REG & SIOF_XFER_START);
        // Save incoming data (which is active low so invert bits)
        *p_snes_gamepad++ = ~SB_REG;
    }
}


// ========== Gamepad Read Interrupt Version ==========


uint8_t snes_gamepad_state;
bool    snes_gamepad_data_ready;
uint8_t *p_snes_gamepad_data;


// Returns whether polling the gamepad via serial interrupts is
// complete and data is ready for use in the "snes_gamepad" var
bool snes_gamepad_interrupt_data_ready(void) {
    return snes_gamepad_data_ready;
}


// Call this to start the process of reading the gamepad via serial
// interrupt transfers
void snes_gamepad_interrupt_read_start(void) {
    CRITICAL {
        p_snes_gamepad_data = (uint8_t *) &snes_gamepad;
        snes_gamepad_state = SNES_GAMEPAD_STATE_LATCH;
        snes_gamepad_data_ready = false;
    }

    // Fake a overly long LATCH signal on S-OUT with a transfer of all bits = 1
    SB_REG = SNES_GAMEPAD_TX_LATCH;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
}


// Called whenever a transfer is complete
static void snes_gamepad_SIO(void) {

    if (snes_gamepad_state != SNES_GAMEPAD_STATE_DONE) {
        // Save incoming data (which is active low so invert bits)
        // except from initial LATCH transfer
        if (snes_gamepad_state != SNES_GAMEPAD_STATE_LATCH) {
            *p_snes_gamepad_data++ = ~SB_REG;
        }

        snes_gamepad_state++;
        if (snes_gamepad_state == SNES_GAMEPAD_STATE_DONE) {
            // All transfers complete, signal data is ready
            snes_gamepad_data_ready = true;
        } else {
            // Start another transfer if needed
            // No bits set in Serial Out byte to avoid disturbing the LATCH line
            SB_REG = 0u;
            SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
        }
    }
}


// Called in install the gamepad SIO interrupt and init state vars
void snes_gamepad_interrupt_init(void) {
    snes_gamepad_state = SNES_GAMEPAD_STATE_DONE;
    snes_gamepad_data_ready = false;

    CRITICAL {
        // Remove first to avoid accidentally double-adding it
        remove_SIO(snes_gamepad_SIO);
        add_SIO(snes_gamepad_SIO);
    }

    set_interrupts(VBL_IFLAG | SIO_IFLAG);    
}


// Called in de-install the gamepad SIO interrupt
void snes_gamepad_interrupt_deinstall(void) {
    CRITICAL {
        // Remove first to avoid accidentally double-adding it
        remove_SIO(snes_gamepad_SIO);
    }
    set_interrupts(VBL_IFLAG & ~SIO_IFLAG);
}
