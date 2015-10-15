/** @file   score_display.c
    @author Ariel Yap & Samuel Yamoah
    @date   14 Oct 2015
    @brief  Module Displays the score once player loses.
    @note   Score is per player and can have a maximum of 9 wins
*/

#include "system.h"
#include "tinygl.h"
#include "score_display.h"

/*
  This Module take the win count (Score) which is passed as a parameter into the
   function score_display and Displays it using Tinygl.
*/

/* Function takes in the score from the win and Displays it */
void score_display (char character)
{
    char buffer[2];
    buffer[0] = character;
    buffer[1] = '\0';
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (buffer);
}
