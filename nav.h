/** @file   nav.h
    @author Samuel Yamoah
    @date   12 Oct 2015
    @brief  Module allows the player move left and right.
*/

#ifndef NAV_H
#define NAV_H

#include "system.h"
#define START_ROW 6
#define START_COL 2

/** Moves the player left or right with wrap around enambled */
void nav_move (uint8_t* current_column);


#endif
