#include "system.h"
#include "pio.h"
#include "pacer.h"
#include "navswitch.h"
#include "display.h"
#include "nav.h"
#include "potato.h"


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
