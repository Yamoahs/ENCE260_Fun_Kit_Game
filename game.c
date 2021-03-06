/** @file   game.c
    @author Ariel Yap & Samuel Yamoah. (Program structure based on Space 9
    written by M. P. Hayes, UCECE)
    @date   5 Oct 2015
    @brief  Main Game file (Hot Potato)
    @note
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#include "tinygl.h"
#include "pacer.h"
#include "ir_serial.h"
#include "ir_uart.h"
#include "flasher.h"
#include "objects.h"
#include "eeprom.h"
#include "uint8toa.h"
#include "font3x5_1.h"
#include "navswitch.h"
#include "led.h"
#include "timer.h"
#include "score_display.h"


/* Define enum constants */
enum { LOOP_RATE = 500 };
enum { FLASHER_UPDATE_RATE = LOOP_RATE };
enum { BUTTON_POLL_RATE = 100 };
enum { GAME_UPDATE_RATE = 500 };
enum { IR_UPDATE_RATE = 1000 };
enum { GAME_OVER_PERIOD = 2 };
enum { BUTTON_HOLD_PERIOD = 1 };

#define MESSAGE_RATE 30

/** Define flasher modes.  */
static flasher_pattern_t flasher_patterns[] = {
  /** POLL_RATE, MOD_FREQ, MOD_DUTY, FLASHER_PERIOD,
     FLASHER_DUTY, FLASHES, PERIOD.  */
    {FLASHER_PATTERN(FLASHER_UPDATE_RATE, 100, 100, 0.4, 100, 1, 0.4)},
    {FLASHER_PATTERN(FLASHER_UPDATE_RATE, 100, 100, 0.4, 100, 1, 0.4)},
    {FLASHER_PATTERN(FLASHER_UPDATE_RATE, 200, 100, 0.1, 50, 1, 0.1)},
};

/* Defining the typedef with a alias flash_mode_t */
typedef enum { FLASH_MODE_PLAYER, FLASH_MODE_BOMB,
    FLASH_MODE_NUM
} flash_mode_t;


/* Defining the typedef with a alias state_t */
typedef enum { STATE_DECIDE, STATE_WAIT, STATE_INIT, STATE_INVITE,
        STATE_START,
    STATE_PLAYING, STATE_OVER,
    STATE_READY
} state_t;


/* initialising  game_data_t struct*/
typedef struct {
    uint8_t games;
} game_data_t;


/* Draw pixel on display.  */
static void display_handler(void *data, uint8_t col, uint8_t row,
                            pix_t type)
{
    uint8_t *display = data;
    uint8_t pixel;

    pixel = row * TINYGL_WIDTH + col;
    display[pixel] = type;
}

/* Initialisng Game Start */
static void game_start(game_data_t * data, uint8_t ME)
{
    tinygl_clear();
    data->games++;
    tossing_start(ME);
    eeprom_write(0, data, sizeof(*data));
}


int main(void)
{
    state_t state = STATE_INIT;
    flasher_t flashers[PIX_TYPE_NUM];
    uint8_t flasher_state[PIX_TYPE_NUM];
    flasher_obj_t flashers_info[PIX_TYPE_NUM];
    uint8_t display[TINYGL_WIDTH * TINYGL_HEIGHT];

    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t ME = 0;
    uint8_t OPPONENT = 0;
    uint8_t RECEIVED = 0;
    uint8_t ir_ticks = 0;
    uint8_t game_ticks = 0;
    uint8_t navswitch_ticks = 0;
    uint8_t navswitch_down_count = 0;


    //Game States
    uint8_t GAME_INVITE = 0;
    uint8_t GAME_ACCEPT = 0;
    uint8_t DUMMY = -2;

    char SCORE = '0';
    game_data_t data;

    /* Initialising Built-in Functions */
    system_init();
    navswitch_init();
    ir_serial_init();

    //Set up the text display using tinygl
    tinygl_init(LOOP_RATE);
    tinygl_font_set(&font3x5_1);
    tinygl_text_speed_set(MESSAGE_RATE);
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set(TINYGL_TEXT_DIR_ROTATE);


    eeprom_read(0, &data, sizeof(data));

    for (i = 0; i < ARRAY_SIZE(flashers); i++) {
        flashers[i] = flasher_init(&flashers_info[i]);
        flasher_state[i] = 0;
    }

    for (i = 0; i < ARRAY_SIZE(display); i++)
        display[i] = 0;

    /* Set up flash patterns for different pixel types.  */
    flasher_pattern_set(flashers[PIX_PLAYER],
                        &flasher_patterns[FLASH_MODE_PLAYER]);
    flasher_pattern_set(flashers[PIX_BOMB],
                        &flasher_patterns[FLASH_MODE_BOMB]);


    tossing_init(GAME_UPDATE_RATE, TINYGL_WIDTH, TINYGL_HEIGHT,
                 display_handler, display);

    navswitch_down_count = 0;
    navswitch_ticks = 0;
    game_ticks = 0;
    ir_ticks = 0;


    pacer_init(LOOP_RATE);


    while (1) {
        pacer_wait();

        if (state == STATE_PLAYING) {
            uint8_t *src;

            /* Update flasher states.  NB, the first flasher is always off.  */
            for (i = 1; i < ARRAY_SIZE(flashers); i++)
                flasher_state[i] = flasher_update(flashers[i]);

            /* Update display.  */
            src = display;
            for (j = 0; j < TINYGL_HEIGHT; j++)
                for (i = 0; i < TINYGL_WIDTH; i++) {
                    tinygl_point_t point = { i, j };

                    tinygl_draw_point(point, flasher_state[*src++]);
                }
        }

        /* Advance messages and refresh display.  */
        tinygl_update();

        game_ticks++;
        if (game_ticks >= LOOP_RATE / GAME_UPDATE_RATE) {
            game_ticks = 0;

            switch (state) {
            case STATE_PLAYING:
                if (!tossing_update(DUMMY)) {
                    tinygl_clear();
                    if (DUMMY == 5) {
                        OPPONENT = 0;
                    } else {
                        ME = 0;
                    }

                    led_set(LED1, 0);
                    state = STATE_OVER;
                }
                DUMMY = -2;
                break;


            //Initial State once game compiled
            case STATE_INIT:
                tinygl_text("HOT POTATO   ");
                state = STATE_DECIDE;
                break;

            //State after a player loses
            case STATE_OVER:
                tinygl_clear();
                if (ME == 0) {
                    tinygl_text(":(  ");
                } else if (OPPONENT == 0) {
                    tinygl_text("WIN   ");
                    SCORE++;
                }
                state = STATE_DECIDE;
                /* Fall through.  */

            default:
                break;

            case STATE_START:
                /* State changes to Start...  */
                game_start(&data, ME);
                state = STATE_PLAYING;
                break;
            }
        }


        /* Poll navswitch.  */
        navswitch_ticks++;
        if (navswitch_ticks >= LOOP_RATE / BUTTON_POLL_RATE) {
            navswitch_ticks = 0;

            navswitch_update();

            if (navswitch_down_p(NAVSWITCH_EAST))
                navswitch_down_count++;
            else
                navswitch_down_count = 0;

            if (navswitch_down_count >=
                BUTTON_POLL_RATE * BUTTON_HOLD_PERIOD)
                state = STATE_INIT;

            /*Initialises the player as player 1 and the player 2 when in the
            start state */
            if (navswitch_push_event_p(NAVSWITCH_NORTH)) {
                switch (state) {
                case STATE_DECIDE:
                    ME = 1;
                    OPPONENT = 2;
                    tinygl_clear();

                    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                    tinygl_text("P1   ");
                    break;
                default:
                    break;
                }
            }

            /*Initialises the player as player 2 and the player 1 when in the
            start state */
            if (navswitch_push_event_p(NAVSWITCH_SOUTH)) {
                switch (state) {
                case STATE_DECIDE:
                    ME = 2;
                    OPPONENT = 1;
                    tinygl_clear();

                    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                    tinygl_text("P2   ");
                    break;
                default:
                    break;
                }
            }

            //Nav switch EAST event
            if (navswitch_push_event_p(NAVSWITCH_EAST)) {
                switch (state) {
                case STATE_WAIT:
                    break;

                case STATE_PLAYING:
                    player_move_right();
                    break;

                default:
                    break;
                }
            }

            //Nav switch WEST event
            if (navswitch_push_event_p(NAVSWITCH_WEST)) {
                switch (state) {
                case STATE_PLAYING:
                    player_move_left();
                    break;
                case STATE_DECIDE:
                    tinygl_clear();
                    score_display(SCORE);
                    break;
                default:
                    break;
                }
            }


            //Nav switch PUSH event
            if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
                switch (state) {
                case STATE_READY:
                    //Resets any active pixels to start the game
                    tinygl_clear();
                    GAME_INVITE = 7;
                    state = STATE_INVITE;
                    break;
                case STATE_DECIDE:
                    if (ME == 0 || OPPONENT == 0) {
                        break;
                    } else {
                        tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                        if (ME == 1) {
                            tinygl_clear();
                            tinygl_text("YOU = P1   ");
                        } else if (ME == 2) {
                            tinygl_clear();
                            tinygl_text("YOU = P2   ");
                        }
                        state = STATE_WAIT;
                    }
                    break;

                case STATE_PLAYING:
                    player_fire();
                    break;
                default:
                    break;
                }
            }
        }

        //IR Transiving code
        ir_ticks++;
        if (ir_ticks) {
            ir_ticks = 0;

            switch (state) {
            case STATE_PLAYING:
                if (ir_serial_receive(&DUMMY) == IR_SERIAL_OK) {
                }
                break;

            //If IR recieved is code 3, change state to ready state
            case STATE_WAIT:
                if (ir_serial_receive(&RECEIVED) == IR_SERIAL_OK) {
                    if (RECEIVED == 3) {
                        state = STATE_READY;
                    }
                }
                DUMMY = 6;
                ir_serial_transmit(OPPONENT);
                break;

            //State Tells each palyer which player they are
            case STATE_DECIDE:
                if (ir_serial_receive(&ME) == IR_SERIAL_OK) {
                    if (ME == 1) {
                        OPPONENT = 2;
                        tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                        ir_serial_transmit(3);
                        tinygl_clear();
                        tinygl_text("YOU = P1   ");
                        state = STATE_READY;
                    }
                    if (ME == 2) {
                        OPPONENT = 1;
                        tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
                        ir_serial_transmit(3);
                        tinygl_clear();
                        tinygl_text("YOU = P2   ");
                        state = STATE_READY;

                    }
                }
                break;

            /*If IR recieved is code 5, change state to Start state else keep
            transmitting Game Invite State*/
            case STATE_INVITE:
                if (ir_serial_receive(&GAME_ACCEPT) == IR_SERIAL_OK) {
                    if (GAME_ACCEPT == 5) {
                        state = STATE_START;
                    }
                }
                ir_serial_transmit(GAME_INVITE);
                break;


            /*If IR recieved is code 7, clear the display then change to and
            transmit Game Accept*/
            case STATE_READY:
                if (ir_serial_receive(&GAME_INVITE) == IR_SERIAL_OK) {
                    if (GAME_INVITE == 7) {
                        tinygl_clear();
                        GAME_ACCEPT = 5;
                        ir_serial_transmit(GAME_ACCEPT);
                        state = STATE_START;
                    }
                }
                break;
            default:
                break;

            case STATE_OVER:
                ir_serial_transmit(5);
            }

        }



    }
}
