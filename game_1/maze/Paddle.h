/**
 * @file Paddle.h
 * @brief Paddle object for Maze Escape game
 * 
 * Controls the paddle position, movement, and rendering.
 * The paddle is controlled by joystick input.
 */

#ifndef PADDLE_H
#define PADDLE_H

#include <stdint.h>
#include "Utils.h"
#include "Joystick.h"

/**
 * @struct Paddle_t
 * @brief Paddle object containing position, size, and score
 */
typedef struct {
    int16_t x;        // Paddle X position (left edge)
    int16_t y;        // Paddle Y position (top edge)
    int16_t width;    // Paddle width
    int16_t height;   // Paddle height
    int16_t speed;    // Movement speed (pixels per frame)
    uint16_t score;   // Game score (incremented on successful hit)
} Paddle_t;

/**
 * @brief Initialize paddle at left side of screen
 * 
 * @param paddle Pointer to paddle object
 * @param x Starting X position
 * @param y Starting Y position
 * @param width Paddle width in pixels
 * @param height Paddle height in pixels
 * @param speed Movement speed in pixels/frame
 */
void Paddle_Init(Paddle_t* paddle, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed);

/**
 * @brief Update paddle position based on joystick input
 * 
 * Moves paddle up/down based on joystick Y direction.
 * Constrains paddle to stay within screen bounds.
 * 
 * @param paddle Pointer to paddle object
 * @param input Joystick input
 */
void Paddle_Update(Paddle_t* paddle, UserInput input);

/**
 * @brief Get the AABB (Axis-Aligned Bounding Box) for the paddle
 * 
 * @param paddle Pointer to paddle object
 * @return AABB structure representing the paddle's bounding box
 */
AABB Paddle_GetAABB(Paddle_t* paddle);

/**
 * @brief Increment the paddle's score
 * 
 * @param paddle Pointer to paddle object
 */
void Paddle_AddScore(Paddle_t* paddle);

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
