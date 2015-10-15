/** @file   flasher.c
    @author Ariel Yap
    @date   14 October 2015
    @brief  Object file which define the pototo
    @note
*/

#include <stdlib.h>
#include "objects.h"
#include "ir_serial.h"
#include "led.h"

#ifndef DISPLAY_INTERLACE
#define DISPLAY_INTERLACE 0
#endif

#if DISPLAY_INTERLACE
/* With an interlaced display we do not h#include "ir_serial.h"ave odd numbered rows and
   columns.  */
enum {BOMB_INC = 2, PLAYER_INC = 2};
#else
enum {BOMB_INC = 1, PLAYER_INC = 1};
#endif

enum {BOMB_SPEED_DEFAULT = 3};
enum {BOMB_NUM_DEFAULT = 3};
enum {CHANCE_SPEED_DEFAULT = 2};

#define min(x, y) ((x) < (y) ? (x) : (y))

static tossing_t data;
static tossing_t * const tossing = &data;


//Function defines the Potato bomb speed
void bomb_speed_set (uint8_t bomb_speed)
{
    tossing->bomb.move_period = tossing->poll_rate / bomb_speed;
}
//Function defines the max speed of the Potato Bomb.
void blow_up_speed_set (uint8_t blow_up_speed)
{
    tossing->blow_up_period = tossing->poll_rate / blow_up_speed;
}

//Function sets the number of Potato Bombs
void bomb_num_set (uint8_t bomb_num)
{
    tossing->bomb.num = min(bomb_num, BOMB_MAX);
}


//Function Defines when the Potato Bomb has been thrown
void tossing_init (uint16_t poll_rate,
             uint8_t x_size, uint8_t y_size,
             void (*display_handler) (void *data, uint8_t row, uint8_t col,
                                      pix_t type),
             void *display_data)
{
    tossing->size.x = x_size;
    tossing->size.y = y_size;
    tossing->display_hook = display_handler;
    tossing->display_data = display_data;
    tossing->active = 0;
    tossing->poll_rate = poll_rate;


    bomb_num_set (BOMB_NUM_DEFAULT);
    bomb_speed_set (BOMB_SPEED_DEFAULT);
    blow_up_speed_set (CHANCE_SPEED_DEFAULT);
}
//Function defines the pixels to turn on when Potato Bomb has been thrown
static void pixel_set (pos_t *pos, pix_t type)
{
    tossing->display_hook (tossing->display_data, pos->x, pos->y, type);
}

/*Function defines the players single direction movement. Movement is
wrapp-around */
static void player_move (int8_t inc)
{
    int8_t player_x;

    pixel_set (&tossing->player, PIX_OFF);

    player_x = tossing->player.x + inc;
    if (player_x >= tossing->size.x)
        player_x = 0;
    else if (player_x < 0)
        player_x = tossing->size.x - 1;
    tossing->player.x = player_x;

   pixel_set (&tossing->player, PIX_PLAYER);
}

/* Return true if move off display.  */
static bool bomb_move_up (obj_t *bomb)
{
    /* Erase the previous position of the shell except if still
       at gun.  */
    if (bomb->pos.x != tossing->player.x
        || bomb->pos.y != tossing->player.y)
        pixel_set (&bomb->pos, PIX_OFF);

    /* Shells only go straight up.  */
    bomb->pos.y -= BOMB_INC;

    if (bomb->pos.y < 0)
        return 1;

    pixel_set (&bomb->pos, PIX_BOMB);
    return 0;
}

/* Return true if reaches player's row.  */
static bool bomb_move_down (obj_t *bomb)
{
    /* Erase the previous position of the shell except if still
       at gun.  */#include "ir_serial.h"
    if (bomb->pos.x != tossing->player.x
        || bomb->pos.y != tossing->player.y)
        pixel_set (&bomb->pos, PIX_OFF);

	if (bomb->pos.y == tossing->player.y && bomb->pos.x == tossing->player.x) {
		tossing->stats.bomb_caught = 1;
		tossing->player.turn = 1;
		led_set (LED1, 1);
	} else if (bomb->pos.y == tossing->player.y && bomb->pos.x != tossing->player.x) {
		tossing->active = 0;
	}
    /* Shells go straight down.  */
    bomb->pos.y += BOMB_INC;

    if (bomb->pos.y > 7)
        return 1;

    pixel_set (&bomb->pos, PIX_BOMB);

    return 0;
}

//Function Initialises the Potato bomb
static void bomb_create (void)
{
    int8_t i;

    for (i = 0; i < tossing->bomb.num; i++)
    {
        obj_t *bomb = &tossing->bomb.array[i];

        if (bomb->active)
            continue;

        bomb->pos = tossing->player;
        bomb->active = 1;
        bomb->direction = 1;
        /* Don't turn shell on initially since it will erase the gun.  */
        return;
    }
    /* If we get to here then we already have too many shells.  */
}

//Function Creates the falling bomb display
static void bomb_create_down (int8_t pos_x)
{
    int8_t i;

    for (i = 0; i < tossing->bomb.num; i++)
    {
        obj_t *bomb = &tossing->bomb.array[i];

        if (bomb->active)
            continue;

        bomb->pos.x = pos_x;
        bomb->pos.y = -1;
        bomb->active = 1;
        bomb->direction = 0;
        /* Don't turn shell on initially since it will erase the gun.  */
        return;
    }
    /* If we get to here then we already have too many shells.  */
}

/* Function removes the Potato Bomb of the Display. Usefully when the Potato
Bomb has either been caught or has left the Display of the thrower. */
static void bomb_kill (obj_t *bomb)
{
    bomb->active = 0;
}


static void
bombs_move (void)
{
    tossing->bomb.move_clock++;
    if (tossing->bomb.move_clock < tossing->bomb.move_period)
        return;
    tossing->bomb.move_clock = 0;


        obj_t *bomb = &tossing->bomb.array[0];

        if (bomb->active) {
		if (bomb->direction == 1) {
			if (bomb_move_up (bomb)) {
				ir_serial_transmit(bomb->pos.x);
				bomb_kill (bomb);
			}
		}
		if (bomb->direction == 0) {
			if (bomb_move_down (bomb)) {
				bomb_kill (bomb);
			}
     }

}
}

/* Update the state of the game.  */
bool tossing_update (int8_t DUMMY)
{
	int8_t pos_x = -1;
    if (DUMMY >= 0 && DUMMY <= 4) {

		pos_x = 4-DUMMY;
		bomb_create_down(pos_x);
	}
	if (DUMMY == 5) {
		tossing->active = 0;
	}

    /* Allow playing with the gun even if game inactive.  */
    bombs_move ();
    if (blow_up_chance() && tossing->player.turn == 1) {
		tossing->active = 0;
	}

    if (!tossing->active)
        return 0;
    return 1;
}

/* Move the gun position to the right wrapping back around on left.  */
void player_move_right (void)
{
    player_move (PLAYER_INC);
}


/* Move the gun position to the left wrapping back around on right.  */
void player_move_left (void)
{
    player_move (-PLAYER_INC);
}


/* Fire the gun.  */
void player_fire (void)
{
	if (tossing->player.turn == 1) {
    bomb_create ();
    if (tossing->stats.bomb_tossed <= 10) {
		tossing->stats.bomb_tossed++;
	}
    bomb_speed_set (BOMB_SPEED_DEFAULT + tossing->stats.bomb_tossed);
	led_set (LED1, 0);
    tossing->player.turn = 0;
}
}

/* Start a new game.  */
void tossing_start (uint8_t ME)
{
    int8_t i;
    int8_t j;

    tossing->bomb.move_clock = 0;

    for (i = 0; i < tossing->bomb.num; i++)
    {
        obj_t *bomb = &tossing->bomb.array[i];

        bomb->active = 0;
    }

    /* Turn all pixels off.  */
    for (i = 0; i < tossing->size.x; i++)
        for (j = 0; j < tossing->size.y; j++)
        {
            pos_t pos = {i, j, 0};

            pixel_set (&pos, PIX_OFF);
        }

    /* Display gun in centre of display.  */
    tossing->player.x = (tossing->size.x / 2) & ~1;
    tossing->player.y = tossing->size.y - 1;
    if (ME == 1) {
		tossing->player.turn = 1;
		led_set (LED1, 1);
	} else if (ME == 2) {
		tossing->player.turn = 0;
	}
    pixel_set (&tossing->player, PIX_PLAYER);

    tossing->stats.bomb_tossed = 0;
    tossing->active = 1;
}


/* Function implements the random timer for how long a player can hold the
Potato Bomb before firing without firing */
bool blow_up_chance(void)
{
	tossing->blow_up_clock++;
    if (tossing->blow_up_clock < tossing->blow_up_period)
        return 0;
    tossing->blow_up_clock = 0;


	uint8_t chance;
	chance = rand () % 200 + 1;
	if (chance == 1) {
		return 1;
	}
	return 0;
}
