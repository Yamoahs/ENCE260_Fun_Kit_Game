/** @file   potato.c
    @author Samuel Yamoah
    @date   13 Oct 2015
    @brief  Module defines the hot potato firing.
    @note   Module also increaments the speed of the firing.
*/
#include "system.h"
#include "pio.h"
#include "pacer.h"
#include "navswitch.h"
#include "display.h"
#include "potato.h"

/* This Module implements the hot potato being fired and recieved. The potato
   will be fired from the current column the player is in. Once fired, the LED's
   in the corresponding column starting from the bottom (row 7) will flash
   cycling through up to row 1. Once the potato has been fired then it will be
   recieved on the other board in the corresponding inverse row (due to the
   other player's board orientated oppositely). The speed of the potato in both
   the fire and fire recieved are set by ticks where
   ticks = (MAX_TICKS - (*butt_press * FACTOR)).
*/



void fireNav(uint8_t* firing, uint8_t* firing_column, uint8_t* butt_press, uint8_t* current_column) {
  *firing = 1;
  *firing_column = *current_column;
  if (*butt_press <= MAX_BUTTS) {
    (*butt_press)++;
  } else {
    *butt_press = 0;
  }
}

uint8_t fireProj(uint8_t* ticks, uint8_t* butt_press, int8_t* firing_row, uint8_t* firing_column, uint8_t* firing) {
  if (*ticks == (MAX_TICKS - (*butt_press * FACTOR))) {
    *ticks = 0;
    if (*firing_row == (START_ROW - 1)) {
      display_pixel_set (*firing_column, *firing_row, 1);
    }
    else {
      display_pixel_set (*firing_column, *firing_row + 1, 0);
      if (*firing_row >= 0) {
        display_pixel_set (*firing_column, *firing_row, 1);
      }
      else {
        *firing_row = 0;
        *firing = 0;
        *ticks = 0;
        return 1;
      }
    }
    (*firing_row)--;
  }
  (*ticks)++;
  return 0;
}

uint8_t fireReceived(uint8_t* ticks, uint8_t* butt_press, int8_t* firing_row, uint8_t* firing_column, uint8_t* falling)
{
  uint8_t inverted_column = *firing_column;
  inverted_column = ((LEDMAT_COLS_NUM -1) - *firing_column);
  if (*ticks == (MAX_TICKS - (*butt_press * FACTOR))) {
    *ticks = 0;
    if (*firing_row == 0) {
      display_pixel_set (inverted_column, *firing_row, 1);
    }
    else {
      display_pixel_set(inverted_column, *firing_row -1, 0);
      if (*firing_row < LEDMAT_ROWS_NUM - 1) {
        display_pixel_set(inverted_column, *firing_row, 1);
      }
      else {
        *firing_row = START_ROW - 1;
        *falling = 0;
        *ticks = 0;
        return 1;
      }
    }
    (*firing_row)++;
    }
    (*ticks)++;
    return 0;
}
