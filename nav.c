/** @file   nav.c
    @author Samuel Yamoah
    @date   12 Oct 2015
    @brief  Module allows the player move left and right.
    @note   Wrap around movement enabled.
*/

#include "system.h"
#include "pio.h"
#include "navswitch.h"
#include "display.h"
#include "nav.h"


/* This module implements a simple left/right joystick. The player will appear
   along the bottom of the display and will move either left or right with a
   push of NAVSWITCH_EAST or NAVSWITCH_WEST respectively. The player can only
   move across the bottom and when it gets to the side it wraps around to the
   other side.
*/

void nav_move (uint8_t* current_column)
{
  navswitch_update ();

  if (navswitch_push_event_p (NAVSWITCH_EAST)) {
    display_pixel_set (*current_column, START_ROW, 0);
    if (*current_column == (LEDMAT_COLS_NUM-1)) {
      *current_column = -1;
    }
    *current_column += 1;
  }

  if (navswitch_push_event_p (NAVSWITCH_WEST)) {
    display_pixel_set (*current_column, START_ROW, 0);
    if (*current_column == 0) {
      *current_column = LEDMAT_COLS_NUM;
    }

    *current_column -= 1;
  }
}
