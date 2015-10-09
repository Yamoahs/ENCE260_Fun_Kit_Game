
#ifndef BULLET_H
#define BULLET_H

#include "system.h"

enum {BOMB_MAX = 1};

typedef enum {PIX_OFF, PIX_PLAYER, PIX_BOMB, PIX_TYPE_NUM} pix_t;

typedef struct
{   int8_t x;
    int8_t y;
} pos_t;

typedef struct
{  pos_t pos;
   bool active;
} obj_t;

typedef struct
{
	obj_t array[BOMB_MAX];
    uint8_t num;
    uint16_t move_period;
    uint16_t move_clock;
} objs_t;

typedef struct
{
    uint8_t bomb_tossed;
    
} stats_t;

typedef struct
{
    /* If we have lots of objects then a linked list would be more
       efficient for the aliens and shells.  */
    objs_t bomb;
    pos_t player;
    pos_t size;
    stats_t stats;
    uint16_t poll_rate;		/* Hz  */
    void (*display_hook) (void *data, uint8_t row, uint8_t col, pix_t type);
    void *display_data;
    bool active;
} tossing_t;

void 
bomb_speed_set (uint8_t bomb_speed);

void
bomb_num_set (uint8_t bomb_num);


void
tossing_init (uint16_t poll_rate,
             uint8_t x_size, uint8_t y_size,
             void (*display_handler) (void *data, uint8_t row, uint8_t col,
                                      pix_t type),
             void *display_data);


/* Update the state of the game.  */
bool tossing_update (void);

/* Move the gun position to the right wrapping back around on left.  */
void
player_move_right (void);

/* Move the gun position to the left wrapping back around on right.  */
void
player_move_left (void);


/* Fire the gun.  */
void
player_fire (void);

/* Start a new game.  */
void
tossing_start (void);

uint8_t
bomb_tossed_get (void);

#endif

