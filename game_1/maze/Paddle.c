/**
 * @file Paddle.c
 * @brief Paddle object implementation for Maze Escape game
 */

#include "Paddle.h"
#include "LCD.h"
#include "Map.h"

// Screen dimensions (ST7789V2 display)
#define SCREEN_HEIGHT 240

void Paddle_Init(Paddle_t* paddle, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed) {
    paddle->x = x;
    paddle->y = y;
    paddle->width = width;
    paddle->height = height;
    paddle->speed = speed;
    paddle->score = 0;
}

static int IsBlocked(int newX, int newY, int width, int height)
{
    int leftCol   = newX / TILE_SIZE;
    int rightCol  = (newX + width - 1) / TILE_SIZE;
    int topRow    = newY / TILE_SIZE;
    int bottomRow = (newY + height - 1) / TILE_SIZE;

    if (Map_IsWall(topRow, leftCol)) return 1;
    if (Map_IsWall(topRow, rightCol)) return 1;
    if (Map_IsWall(bottomRow, leftCol)) return 1;
    if (Map_IsWall(bottomRow, rightCol)) return 1;

    return 0;
}

void Paddle_Update(Paddle_t* paddle, UserInput input)
{
    int newX = paddle->x;
    int newY = paddle->y;

    if (input.direction == N)  newY -= paddle->speed;
    if (input.direction == S)  newY += paddle->speed;
    if (input.direction == E)  newX += paddle->speed;
    if (input.direction == W)  newX -= paddle->speed;

    if (input.direction == NE) {
        newX += paddle->speed;
        newY -= paddle->speed;
    }
    if (input.direction == NW) {
        newX -= paddle->speed;
        newY -= paddle->speed;
    }
    if (input.direction == SE) {
        newX += paddle->speed;
        newY += paddle->speed;
    }
    if (input.direction == SW) {
        newX -= paddle->speed;
        newY += paddle->speed;
    }

    if (!IsBlocked(newX, newY, paddle->width, paddle->height)) {
        paddle->x = newX;
        paddle->y = newY;
    }
}

AABB Paddle_GetAABB(Paddle_t* paddle) {
    AABB box = {paddle->x, paddle->y, paddle->width, paddle->height};
    return box;
}

void Paddle_AddScore(Paddle_t* paddle) {
    paddle->score++;
}

void Paddle_Draw(Paddle_t* paddle)
{
    LCD_Draw_Rect(paddle->x, paddle->y, paddle->width, paddle->height, 1, 1);
}