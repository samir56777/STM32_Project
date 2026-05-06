/**
 * @file Player.h
 * @brief Player object for Maze Escape game
 * 
 * Controls the player position, movement, and rendering.
 * The player is controlled by joystick input.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include "Utils.h"
#include "Joystick.h"

/**
 * @struct Player_t
 * @brief Player object containing position, size, and score
 */
typedef struct {
    int16_t x;        // Player X position (left edge)
    int16_t y;        // Player Y position (top edge)
    int16_t width;    // Player width
    int16_t height;   // Player height
    int16_t speed;    // Movement speed (pixels per frame)
    uint16_t score;   // Game score (incremented on successful hit)
} Player_t;

/**
 * @brief Initialize player at left side of screen
 * 
 * @param player Pointer to player object
 * @param x Starting X position
 * @param y Starting Y position
 * @param width Player width in pixels
 * @param height Player height in pixels
 * @param speed Movement speed in pixels/frame
 */
void Player_Init(Player_t* player, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed);

/**
 * @brief Update player position based on joystick input
 * 
 * Moves player up/down based on joystick Y direction.
 * Constrains player to stay within screen bounds.
 * 
 * @param player Pointer to player object
 * @param input Joystick input
 */
void Player_Update(Player_t* player, UserInput input);

/**
 * @brief Draw player on LCD
 * 
 * Draws the player as a filled rectangle at its current position.
 * 
 * @param player Pointer to player object
 */
void Player_Draw(Player_t* player);

/**
 * @brief Get player bounding box for collision detection
 * 
 * @param player Pointer to player object
 * @return AABB structure representing the player's collision box
 */
/*AABB Player_GetAABB(Player_t* player);*/     /*FUNCTIONNNNNNNN */ 

/**
 * @brief Increment player score
 * 
 * Called when player successfully hits the ball.
 * 
 * @param player Pointer to player object
 */
/*void Player_AddScore(Player_t* player);*/   /*FUNCTIONNNNNNNN */ 

/**
 * @brief Get current score
 * 
 * @param player Pointer to player object
 * @return Current score
 */
/*uint16_t Player_GetScore(Player_t* player);*/  /*FUNCTIONNNNNNNN */

/**
 * @brief Get player position
 * 
 * @param player Pointer to player object
 * @return Current position (top-left corner)
 */
/*Position2D Player_GetPos(Player_t* player);*/     /*FUNCTIONNNNNNNN */

/**
 * @brief Get player height
 * 
 * @param player Pointer to player object
 * @return Height in pixels
 */
/*int16_t Player_GetHeight(Player_t* player);*/   /*FUNCTIONNNNNNNN */

/**
 * @brief Get player width
 * 
 * @param player Pointer to player object
 * @return Width in pixels
 */
/*int16_t Player_GetWidth(Player_t* player); */    /*FUNCTIONNNNNNNN */

#endif // PLAYER_H
