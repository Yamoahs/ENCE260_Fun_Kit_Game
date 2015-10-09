#include <string.h>
#include "system.h"
#include "tinygl.h"
#include "pacer.h"
#include "ir_serial.h"
#include "flasher.h"
#include "spacey.h"
#include "eeprom.h"
#include "uint8toa.h"
#include "font3x5_1.h"
#include "navswitch.h"
#include "led.h"
#include "ir_uart.h"
#include <stdio.h>
#include <stdlib.h>


enum {LOOP_RATE = 500};
enum {FLASHER_UPDATE_RATE = LOOP_RATE};
enum {BUTTON_POLL_RATE = 100};
enum {GAME_UPDATE_RATE = 100};
enum {GAME_OVER_PERIOD = 2};
enum {BUTTON_HOLD_PERIOD = 1};

#define MESSAGE_RATE 10

/** Define flasher modes.  */
static flasher_pattern_t flasher_patterns[] =
{
    /** POLL_RATE, MOD_FREQ, MOD_DUTY, FLASHER_PERIOD,
       FLASHER_DUTY, FLASHES, PERIOD.  */
    {FLASHER_PATTERN (FLASHER_UPDATE_RATE, 100, 100, 0.4, 100, 1, 0.4)},
    {FLASHER_PATTERN (FLASHER_UPDATE_RATE, 100, 100, 0.4, 100, 1, 0.4)},
    {FLASHER_PATTERN (FLASHER_UPDATE_RATE, 200, 100, 0.1, 50, 1, 0.1)},
};

typedef enum {FLASH_MODE_PLAYER, FLASH_MODE_BOMB,
			  FLASH_MODE_NUM} flash_mode_t;


typedef enum {STATE_DUMMY, STATE_INIT, STATE_WAIT, STATE_START,
              STATE_PLAYING, STATE_OVER, 
              STATE_READY, STATE_MENU_LEVEL} state_t;
              

              
typedef struct
{
    uint8_t level;
    uint8_t games;
} game_data_t;

              
/** Draw pixel on display.  */
static void
display_handler (void *data, uint8_t col, uint8_t row, pix_t type)
{
    uint8_t *display = data;
    uint8_t pixel;

    pixel = row * TINYGL_WIDTH + col;
    display[pixel] = type;
}

static void
game_start (game_data_t *data)
{
    tinygl_clear ();
    data->games++;
    tossing_start ();
    eeprom_write (0, data, sizeof (*data));
}




int main (void)
{
	uint8_t navswitch_ticks;
    uint8_t game_ticks;
    uint8_t game_over_ticks;
    uint8_t navswitch_down_count;
    state_t state = STATE_INIT;
    flasher_t flashers[PIX_TYPE_NUM];
    uint8_t flasher_state[PIX_TYPE_NUM];
    flasher_obj_t flashers_info[PIX_TYPE_NUM];
    uint8_t display[TINYGL_WIDTH * TINYGL_HEIGHT];
    uint8_t i;
    uint8_t j;
    game_data_t data;
    uint8_t NUM_ONE;
    uint8_t NUM_TWO;
    
	
    system_init ();
    
    
    eeprom_read (0, &data, sizeof (data));
    if (data.level == 0xff)
    {
        data.level = 0;
        data.games = 0;
    }

    for (i = 0; i < ARRAY_SIZE (flashers); i++)
    {
        flashers[i] = flasher_init (&flashers_info[i]);
        flasher_state[i] = 0;
    }

    for (i = 0; i < ARRAY_SIZE (display); i++)
        display[i] = 0;

    /* Set up flash patterns for different pixel types.  */
    flasher_pattern_set (flashers[PIX_PLAYER],
                         &flasher_patterns[FLASH_MODE_PLAYER]);
    flasher_pattern_set (flashers[PIX_BOMB],
                         &flasher_patterns[FLASH_MODE_BOMB]);
                         
    tinygl_init (LOOP_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_speed_set (MESSAGE_RATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
	
    
    navswitch_init();
    ir_uart_init ();

    tossing_init (GAME_UPDATE_RATE, TINYGL_WIDTH, TINYGL_HEIGHT, 
                 display_handler, display);

    navswitch_ticks = 0;
    game_ticks = 0;
    game_over_ticks = 0;
    navswitch_down_count = 0;

    pacer_init (LOOP_RATE);
    

    while (1)
    {
		pacer_wait ();	
					
		if (state == STATE_PLAYING)
        {
            uint8_t *src;

            /* Update flasher states.  NB, the first flasher is always off.  */
            for (i = 1; i < ARRAY_SIZE (flashers); i++)
                flasher_state[i] = flasher_update (flashers[i]);
            
            /* Update display.  */
            src = display;
            for (j = 0; j < TINYGL_HEIGHT; j++)
                for (i = 0; i < TINYGL_WIDTH; i++)
                {
                    tinygl_point_t point = {i, j};

                    tinygl_draw_point (point, flasher_state[*src++]);
                }
        }
        
        /* Advance messages and refresh display.  */
        tinygl_update ();
        
        game_ticks++;
        if (game_ticks >= LOOP_RATE / GAME_UPDATE_RATE)
        {
            game_ticks = 0;
                
            switch (state)
            {
            case STATE_PLAYING:
                if (! tossing_update ())
                {
                    game_over_ticks = 0;
                    led_set (LED1, 1);
                    state = STATE_OVER;
                }
                break;
                
            case STATE_INIT:
                tinygl_text ("HOT POTATO");	
                break;
                
/*
            case STATE_WAIT:
				NUM_ONE = rand() % 100 + 1;
				if (ir_uart_read_ready_p ()) {	
					NUM_TWO = ir_uart_getc ();
				}
				if (ir_uart_write_ready_p ()) {
					ir_uart_putc (NUM_ONE);
				}
				
				if (NUM_ONE!= 0 && NUM_TWO != 0 && NUM_ONE != NUM_TWO) {
					if (NUM_ONE > NUM_TWO) {
						tinygl_text ("YOU ARE P1");
					}
					else 
						tinygl_text ("YOU ARE P2");
					}
				break;				
*/
						
			                
            case STATE_OVER:
                game_over_ticks ++;
                if (game_over_ticks >= GAME_UPDATE_RATE * GAME_OVER_PERIOD)
                    state = STATE_READY;
                /* Fall through.  */
                
            case STATE_READY:
            case STATE_MENU_LEVEL:
            default:
                break;
                
            case STATE_START:
                /* Turn that bloody blimey space invader off...  */
                game_start (&data);
                led_set (LED1, 0);
                state = STATE_PLAYING;
                break;
            }
        }
        
        
        /* Poll navswitch.  */
        navswitch_ticks++;
        if (navswitch_ticks >= LOOP_RATE / BUTTON_POLL_RATE)
        {
            navswitch_ticks = 0;

            navswitch_update ();

            if (navswitch_down_p (NAVSWITCH_EAST))
                navswitch_down_count++;
            else
                navswitch_down_count = 0;

            if (navswitch_down_count >= BUTTON_POLL_RATE * BUTTON_HOLD_PERIOD)
                state = STATE_INIT;

            if (navswitch_push_event_p (NAVSWITCH_EAST))
            {
                switch (state)
                {
                case STATE_READY:
                    break;

                case STATE_MENU_LEVEL:
                    break;

                case STATE_PLAYING:
                    player_move_right ();
                    break;

                default:
                    break;
                }
            }

            if (navswitch_push_event_p (NAVSWITCH_WEST))
            {
                switch (state)
                {
                case STATE_READY:
                    break;

                

                case STATE_PLAYING:
                    player_move_left ();
                    break;

                default:
                    break;
                }
            }

            if (navswitch_push_event_p (NAVSWITCH_WEST))
            {
                switch (state)
                {
                case STATE_MENU_LEVEL:
                    break;

                case STATE_PLAYING:
                    break;

                default:
                    break;
                }
            }

            if (navswitch_push_event_p (NAVSWITCH_PUSH))
            {
                switch (state)
                {
				case STATE_INIT:
					state = STATE_WAIT;
					break;
                case STATE_READY:
                    state = STATE_START;
                    break;

                case STATE_PLAYING:
                    player_fire ();
                    break;

                case STATE_MENU_LEVEL:
                    state = STATE_READY;
                    break;

                default:
                    break;
                }
            }
        }
        

    }
}
