#include <gbdk/platform.h>
#include <stdint.h>
#include <stdio.h>

#include <gb/drawing.h>

#include "common.h"
#include "input.h"
#include "util.h"

#include "snes_mouse.h"
#include "snes_gamepad.h"


uint8_t sample_num = 0;

enum {
    POLL_MOUSE,
    POLL_GAMEPAD
};


#define SPRITE_MOUSE_CURSOR 0u

const unsigned char mouse_cursors[] = {
  // Arrow 1
  0xFE, 0xFE, 0xFC, 0x84, 0xF8, 0x98, 0xF8, 0xA8,
  0xFC, 0xB4, 0xCE, 0xCA, 0x87, 0x85, 0x03, 0x03,
  // Arrow 2
  0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xA0, 0xF0, 0x90,
  0xF8, 0x88, 0xF0, 0xB0, 0xD8, 0xD8, 0x08, 0x08,
  // Arrow 3
  0x80, 0x80, 0xC0, 0xC0, 0xE0, 0xA0, 0xF0, 0x90,
  0xF8, 0x88, 0xFC, 0x84, 0xF8, 0x98, 0xE0, 0xE0,
  // Hand
  0x10, 0x10, 0x38, 0x28, 0x38, 0x28, 0x7E, 0x6E,
  0xFE, 0xA2, 0xFE, 0x82, 0x7E, 0x42, 0x3E, 0x3E
};


static void main_init(void) {

    HIDE_BKG;
    HIDE_SPRITES;

    set_sprite_data(0u, 2u, mouse_cursors);
    set_sprite_tile(SPRITE_MOUSE_CURSOR, 1u);
    move_sprite(0, 160u / 2, 144u / 2);

    DISPLAY_ON;
    SPRITES_8x8;

    SHOW_BKG;
    SHOW_SPRITES;

	UPDATE_KEYS();
}


void use_mouse_data(void) {

    uint8_t mouse_buttons = (snes_mouse.buttons & SNES_MOUSE_BUTTON_MASK);

    int8_t mouse_y_move = (snes_mouse.move_y & SNES_MOUSE_Y_MASK);
    if (snes_mouse.move_y & SNES_MOUSE_Y_DIR) mouse_y_move *= -1;

    int8_t mouse_x_move = (snes_mouse.move_x & SNES_MOUSE_X_MASK);
    if (snes_mouse.move_x & SNES_MOUSE_X_DIR) mouse_x_move *= -1;

    scroll_sprite(SPRITE_MOUSE_CURSOR, mouse_x_move, mouse_y_move);

    if (mouse_buttons & SNES_MOUSE_BUTTON_LEFT) {
        OAM_item_t * itm = &shadow_OAM[SPRITE_MOUSE_CURSOR];
        plot_point(itm->x - DEVICE_SPRITE_PX_OFFSET_X + SCX_REG,
                   itm->y - DEVICE_SPRITE_PX_OFFSET_Y + SCY_REG);
    }

    if (mouse_buttons & SNES_MOUSE_BUTTON_RIGHT) {
        scroll_bkg(-mouse_x_move, -mouse_y_move);
    }
}


void use_gamepad_data(void) {

    int8_t gamepad_y_move = 0;
    int8_t gamepad_x_move = 0;

    if      (snes_gamepad.first_byte & SNES_PAD_UP)   gamepad_y_move = -1;
    else if (snes_gamepad.first_byte & SNES_PAD_DOWN) gamepad_y_move =  1;

    if      (snes_gamepad.first_byte & SNES_PAD_LEFT)  gamepad_x_move = -1;
    else if (snes_gamepad.first_byte & SNES_PAD_RIGHT) gamepad_x_move =  1;

    scroll_sprite(SPRITE_MOUSE_CURSOR, gamepad_x_move, gamepad_y_move);

    if (snes_gamepad.second_byte & SNES_PAD_A) {
        OAM_item_t * itm = &shadow_OAM[SPRITE_MOUSE_CURSOR];
        plot_point(itm->x - DEVICE_SPRITE_PX_OFFSET_X + SCX_REG,
                   itm->y - DEVICE_SPRITE_PX_OFFSET_Y + SCY_REG);
    }

    if (snes_gamepad.first_byte & SNES_PAD_B) scroll_bkg(-gamepad_x_move, -gamepad_y_move);

    if      (snes_gamepad.second_byte & SNES_PAD_L) scroll_bkg(-1,0);
    else if (snes_gamepad.second_byte & SNES_PAD_R) scroll_bkg(1,0);

    if      (snes_gamepad.second_byte & SNES_PAD_X) scroll_bkg(0,-1);
    else if (snes_gamepad.first_byte & SNES_PAD_Y)  scroll_bkg(0,1);
}


void poll_loop(uint8_t poll_type) {

    if (poll_type == POLL_MOUSE) {
        // Init and do an first read to get the interrupt cycle running
        snes_mouse_interrupt_init();
        snes_mouse_interrupt_read_start();
    }
    else if (poll_type == POLL_GAMEPAD) {
        // Init and do an first read to get the interrupt cycle running
        snes_gamepad_interrupt_init();
        snes_gamepad_interrupt_read_start();
    }

    UPDATE_KEYS();
    while (1) {
        // Check for exit
        UPDATE_KEYS();
        if (KEY_TICKED(J_ANY)) break;

        if (poll_type == POLL_MOUSE) {
            // Check if data ready for use
            if (snes_mouse_interrupt_data_ready()) {
                use_mouse_data();
                // Queue next read
                snes_mouse_interrupt_read_start();
            }
        }
        else if (poll_type == POLL_GAMEPAD) {
            // Check if data ready for use
            if (snes_gamepad_interrupt_data_ready()) {
                use_gamepad_data();
                // Queue next read
                snes_gamepad_interrupt_read_start();
            }
        }

        vsync();
    }

    if (poll_type == POLL_MOUSE)
        snes_mouse_interrupt_deinstall();
    else if (poll_type == POLL_MOUSE)
        snes_gamepad_interrupt_deinstall();

    apa_exit();
    mode(M_TEXT_OUT);
    SCX_REG = 0u;
    SCY_REG = 0u;
}


void poll_mouse_once_log(void) {

    sample_num++;
    snes_mouse_blocking_wait_poll();

    // cast to uint8_t / unsigned char has the bug
    printf("%hu:%hx %hx %hx %hx",
        (char)sample_num,
        (char)snes_mouse.first_byte,
        (char)snes_mouse.buttons,
        (char)snes_mouse.move_y,
        (char)snes_mouse.move_x);

    printf("\n");
}


void poll_gamepad_once_log(void) {

    sample_num++;
    snes_gamepad_blocking_wait_poll();

    // cast to uint8_t / unsigned char has the bug
    printf("%hu:%hx %hx",
        (char)sample_num,
        (char)snes_gamepad.first_byte,
        (char)snes_gamepad.second_byte);

    printf("\n");
}


void main(void){

    UPDATE_KEYS();


    printf(
      "Mouse \n"
      " ST: Live Draw\n"
      " A:  Poll + Log\n"
      "\n"
      "GamePad \n"
      " SEL: Live Draw\n"
      " B:  Poll + Log\n");
    main_init();

    while (1) {

        UPDATE_KEYS();
        if (KEY_TICKED(J_START)) poll_loop(POLL_MOUSE);
        if (KEY_TICKED(J_A)) poll_mouse_once_log();

        if (KEY_TICKED(J_SELECT)) poll_loop(POLL_GAMEPAD);
        if (KEY_TICKED(J_B)) poll_gamepad_once_log();

        vsync();
    }
}

