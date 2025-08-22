#include <gbdk/platform.h>
#include <stdint.h>
#include <gbdk/font.h>

#define M_RESET_TO_DEFAULT 0u

void apa_exit(void){

    // Remove the two APA interrupt routines
    // Turn off LCD Interrupt
    // This disables and then re-enables interrupts so it must be outside the critical section.
    set_interrupts(IE_REG & ~LCD_IFLAG);

    // Remove the APA LCD interrupt routine (with interrupts turned off)
    CRITICAL {
        __asm \
            push DE

            LD      DE, #.drawing_lcd
            CALL    .remove_LCD

            pop DE
        __endasm;
    }

    // Reset VRAM to use the 8000 range again
    LCDC_REG &= ~LCDCF_BG8000;

    //sets drawing flag to OFF
    mode(M_RESET_TO_DEFAULT);
}

void return_to_text_mode(void) {

    apa_exit();
    mode(M_TEXT_OUT);

    font_init();
    font_t font;
    font = font_load(font_ibm);
    font_set(font);
}



