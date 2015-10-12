#include "system.h"
#include "pio.h"
#include "pacer.h"
#include "navswitch.h"
#include "display.h"

#define START_ROW 6
#define START_COL 2
#define STARTING_SPEED 100
#define MAX_BUTTS 8
#define MAX_TICKS 150
#define FACTOR 10


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


int main (void)

{
    system_init ();
    display_init ();
    navswitch_init ();
    pacer_init (600);
    uint8_t current_column = START_COL;
    int8_t firing_row = START_ROW - 1;
    uint8_t firing_column = START_COL;
    uint8_t ticks = 0;
    uint8_t firing = 0;
    uint8_t falling = 0;
    uint8_t butt_press = 0;


    while (1)
    {
      pacer_wait();
      nav_move(&current_column);

      if (!firing && !falling && (navswitch_push_event_p (NAVSWITCH_PUSH) || navswitch_push_event_p (NAVSWITCH_NORTH))) {
        fireNav(&firing, &firing_column, &butt_press, &current_column);
      }

      if (firing) {
        if (fireProj(&ticks, &butt_press, &firing_row, &firing_column, &firing)) {
          falling = 1;
        }
      }

      if (falling) {
        if (fireReceived(&ticks, &butt_press, &firing_row, &firing_column, &falling)) {
          firing = 1;
        }
      }




      display_pixel_set (current_column, START_ROW, 1);
      display_update();



      //counter += 1
    }

}
