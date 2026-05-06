/**
 * @file maze_escape.h
 * @brief Main game engine for Maze Escape
 * 
 * Coordinates player updates, handles collisions,
 * and manages game state (level/guards).
 */

#ifndef MAZE_ESCAPE_H
#define MAZE_ESCAPE_H

#include <stdint.h>
#include "Paddle.h"
#include "Utils.h"
#include "Guard.h"
#define GUARD_COUNT 6
typedef struct {
    Player_t player;
    Guard_t guards[GUARD_COUNT];
    uint8_t alarm_triggered;
    uint8_t level;
    uint8_t detected_screen_active;
    uint32_t detected_screen_until;
    uint8_t game_finished;
    uint32_t win_input_unlock_tick;
} MazeEscape_t;

/**
 * @struct MazeEscape_t
 * @brief Main game engine object
 */


/**
 * @brief Initialize the Maze Escape game engine
 * 
 * Sets up the player and initial game state.
 * 
 * @param engine Pointer to game engine
 * @param player_x Initial player X position
 * @param player_y Initial player Y position
 * @param player_width Player width
 * @param player_height Player height
 */
void MazeEscape_Init(MazeEscape_t* engine, 
                     int16_t player_x, int16_t player_y,
                     int16_t player_width, int16_t player_height
                     );

/**
 * @brief Update game state (input + physics + collisions)
 * 
 * This is called every frame:
 * 1. Updates player based on joystick input
 * 2. Updates guard positions
 * 3. Checks collision with guards
 * 4. Checks if player reached exit
 * 
 * @param engine Pointer to game engine
 * @param input Joystick input for this frame
 * @return Game state (0 = continue, other values indicate state)
 */
uint8_t MazeEscape_Update(MazeEscape_t* engine, UserInput input);

/**
 * @brief Draw all game objects
 * 
 * Draws the map, player and guards to the LCD buffer.
 * Note: does NOT call LCD_clear() or LCD_Refresh() - 
 *       those are the caller's responsibility.
 * 
 * @param engine Pointer to game engine
 */
void MazeEscape_Draw(MazeEscape_t* engine);

/**
 * @brief Get current game level
 * 
 * @param engine Pointer to game engine
 * @return Current level number
 */
uint8_t MazeEscape_GetLevel(MazeEscape_t* engine);

/**
 * @brief Get current alarm state
 * 
 * @param engine Pointer to game engine
 * @return Alarm triggered status
 */
/*uint16_t MazeEscape_GetScore(MazeEscape_t* engine);*/

#endif // MAZE_ESCAPE_H
