#ifndef GAME_3_H
#define GAME_3_H

#include "Menu.h"

/**
 * @brief Game 1 - Student can implement their own game here
 * 
 * Placeholder for Student 1's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 * 
 * The menu system calls this function when Game 1 is selected.
 * The function runs its own loop and returns when the game exits.
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */

MenuState Game3_Run(void);

typedef enum {
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN
} GameState_t;

#endif // GAME_1_H
