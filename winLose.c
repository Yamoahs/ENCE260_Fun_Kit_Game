/** @file   winLose.c
    @author Samuel Yamoah
    @date   13 Oct 2015
    @brief  Module tells Players who won and lost.
    @note   Module works based on state of won boolean.
*/

#include "system.h"
#include "pacer.h"
#include "tinygl.h"
#include "../fonts/font5x7_1.h"
#include "winLose.h"

/* This modules implements the win or lose display on each players board. If the
   player is unable to catch the potato or the potato explodes once the time is
   up, then the game will change into an end state and the misfourtante player
   win state will remain at 0 (False) and the other player will have a win state
   of 1 (True).

*/

#define PACER_RATE 500
#define MESSAGE_RATE 15
#define LOOP_RATE PACER_RATE


void winner_or_loser(uint8_t win)
{
  system_init();
  tinygl_init(LOOP_RATE);

  char* text = '\0';
  tinygl_font_set(&font5x7_1);
  tinygl_text_speed_set (MESSAGE_RATE);
  tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);

  if (win){
    text = "You Win ";
  }
  else {
    text = "You Loose ";
  }

    tinygl_text (text);
    pacer_init (PACER_RATE);
}



int main (void)
{
    uint8_t win = 0;

    winner_or_loser(win);



    while(1)
    {
        pacer_wait();
        tinygl_update();

    }
    return 0;
}
