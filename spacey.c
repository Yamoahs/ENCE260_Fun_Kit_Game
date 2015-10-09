#include <stdlib.h>
#include "spacey.h"


#ifndef DISPLAY_INTERLACE
#define DISPLAY_INTERLACE 0
#endif   

#if DISPLAY_INTERLACE
/* With an interlaced display we do not have odd numbered rows and
   columns.  */
enum {BOMB_INC = 2, PLAYER_INC = 2};
#else
enum {BOMB_INC = 1, PLAYER_INC = 1};
#endif

enum {BOMB_SPEED_DEFAULT = 16};
enum {BOMB_NUM_DEFAULT = 2};

#define min(x, y) ((x) < (y) ? (x) : (y))

static tossing_t data;
static tossing_t * const tossing = &data;



void 
bomb_speed_set (uint8_t bomb_speed)
{
    tossing->bomb.move_period = tossing->poll_rate / bomb_speed;
}

void
bomb_num_set (uint8_t bomb_num)
{
    tossing->bomb.num = min(bomb_num, BOMB_MAX);
}


void
tossing_init (uint16_t poll_rate,
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
}

static void
pixel_set (pos_t *pos, pix_t type)
{
    tossing->display_hook (tossing->display_data, pos->x, pos->y, type);
}

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
static bool bomb_move (obj_t *bomb)
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


static void
bomb_create (void)
{
    int8_t i;

    for (i = 0; i < tossing->bomb.num; i++)
    {
        obj_t *bomb = &tossing->bomb.array[i];

        if (bomb->active)
            continue;

        bomb->pos = tossing->player;
        bomb->active = 1;
        tossing->stats.bomb_tossed++;
        /* Don't turn shell on initially since it will erase the gun.  */
        return;
    }
    /* If we get to here then we already have too many shells.  */
}

static void
bomb_kill (obj_t *bomb)
{
    bomb->active = 0;
}


static void
bombs_move (void)
{
    int8_t i;

    tossing->bomb.move_clock++;
    if (tossing->bomb.move_clock < tossing->bomb.move_period)
        return;
    tossing->bomb.move_clock = 0;

    /* Shells move until they hit an alien or move off the display.  */

    for (i = 0; i < tossing->bomb.num; i++)
    {
        obj_t *bomb = &tossing->bomb.array[i];

        if (!bomb->active)
            continue;

        if (bomb_move (bomb))
            bomb_kill (bomb);
    }   
}

/* Update the state of the game.  */
bool tossing_update (void)
{
    /* Allow playing with the gun even if game inactive.  */
    bombs_move ();
    
    if (!tossing->active)
        return 0;
    return 1;
}

/* Move the gun position to the right wrapping back around on left.  */
void
player_move_right (void)
{
    player_move (PLAYER_INC);
}


/* Move the gun position to the left wrapping back around on right.  */
void
player_move_left (void)
{
    player_move (-PLAYER_INC);
}


/* Fire the gun.  */
void
player_fire (void)
{
    bomb_create ();
}

/* Start a new game.  */
void
tossing_start (void)
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
            pos_t pos = {i, j};

            pixel_set (&pos, PIX_OFF);
        }

    /* Display gun in centre of display.  */
    tossing->player.x = (tossing->size.x / 2) & ~1;
    tossing->player.y = tossing->size.y - 1;
    pixel_set (&tossing->player, PIX_PLAYER);

    tossing->stats.bomb_tossed = 0;
    tossing->active = 1;
}


uint8_t
bomb_tossed_get (void)
{
    return tossing->stats.bomb_tossed;
}




