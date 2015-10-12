#include "system.h"
#include "pio.h"
#include "pacer.h"
#include "navswitch.h"
#include "display.h"

#define START_ROW 6
#define START_COL 2
#define STARTING_SPEED 100
#define MAX_BUTTS 10
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

void shoot (uint8_t* current_column) {
  //shooter code
  if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
    uint8_t i;
    for (i = 0; i < LEDMAT_ROWS_NUM; i++) {
      display_pixel_set (*current_column, i, 1);
    }
  }
}

void ledmat (void)
{
  pio_config_set(LEDMAT_COL1_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_COL2_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_COL3_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_COL4_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_COL5_PIO, PIO_OUTPUT_HIGH);

  pio_config_set(LEDMAT_ROW1_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW2_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW3_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW4_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW5_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW6_PIO, PIO_OUTPUT_HIGH);
  pio_config_set(LEDMAT_ROW7_PIO, PIO_OUTPUT_HIGH);
}


int main (void)

{
    system_init ();
    display_init ();
    navswitch_init ();
    ledmat();
    pacer_init (600);
    uint8_t current_column = START_COL;
    int8_t firing_row = START_ROW - 1;
    uint8_t firing_column = START_COL;
    uint8_t ticks = 0;
    uint8_t firing = 0;
    uint8_t butt_press = 0;


    while (1)
    {
      pacer_wait();
      nav_move(&current_column);

      if (navswitch_push_event_p (NAVSWITCH_PUSH) || navswitch_push_event_p (NAVSWITCH_NORTH)) {
        if (!firing) {
          firing = 1;
          firing_column = current_column;
          if (butt_press <= MAX_BUTTS) {
            butt_press++;
          } else {
            butt_press = 0;

          }
        }
      }

      if (firing) {
        if (ticks == (MAX_TICKS - (butt_press * FACTOR))) {
          ticks = 0;
          if (firing_row == (START_ROW - 1)) {
            display_pixel_set (firing_column, firing_row, 1);
          }
          else {
            display_pixel_set (firing_column, firing_row + 1, 0);
            if (firing_row >= 0) {
              display_pixel_set (firing_column, firing_row, 1);
            }
            else {
              firing_row = START_ROW;
              firing = 0;
              ticks = 255;
            }
          }
          firing_row--;
        }
        ticks++;
      }

      display_pixel_set (current_column, START_ROW, 1);
      display_update();



      //counter += 1
    }

}
