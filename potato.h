/** @file   potato.h
    @author Samuel Yamoah
    @date   13 Oct 2015
    @brief  Module creates, shoots and recieves the potato.
*/

#ifndef POTATO_H
#define POTATO_H

#include "system.h"

#define START_ROW 6
#define START_COL 2
#define STARTING_SPEED 100
#define MAX_BUTTS 8
#define MAX_TICKS 150
#define FACTOR 10

/** Moves the player left or right with wrap around enambled */
void fireNav(uint8_t* firing, uint8_t* firing_column, uint8_t* butt_press, uint8_t* current_column);


uint8_t fireProj(uint8_t* ticks, uint8_t* butt_press, int8_t* firing_row, uint8_t* firing_column, uint8_t* firing);


uint8_t fireReceived(uint8_t* ticks, uint8_t* butt_press, int8_t* firing_row, uint8_t* firing_column, uint8_t* falling);


#endif
