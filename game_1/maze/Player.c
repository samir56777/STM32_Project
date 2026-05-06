/**
 * @file Player.c
 * @brief Player object implementation for Maze Escape game
 */

#include "Paddle.h"
#include "LCD.h"
#include "Map.h"

// Screen dimensions (ST7789V2 display)
#define SCREEN_HEIGHT 240

void Player_Init(Player_t* player, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed) {
    player->x = x;
    player->y = y;
    player->width = width;
    player->height = height;
    player->speed = speed;
    player->score = 0;
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

void Player_Update(Player_t* player, UserInput input)
{
    int newX = player->x;
    int newY = player->y;

    if (input.direction == N)  newY -= player->speed;
    if (input.direction == S)  newY += player->speed;
    if (input.direction == E)  newX += player->speed;
    if (input.direction == W)  newX -= player->speed;

    if (input.direction == NE) {
        newX += player->speed;
        newY -= player->speed;
    }
    if (input.direction == NW) {
        newX -= player->speed;
        newY -= player->speed;
    }
    if (input.direction == SE) {
        newX += player->speed;
        newY += player->speed;
    }
    if (input.direction == SW) {
        newX -= player->speed;
        newY += player->speed;
    }

    if (!IsBlocked(newX, newY, player->width, player->height)) {
        player->x = newX;
        player->y = newY;
    }
}

void Player_Draw(Player_t* player)
{
    LCD_Draw_Rect(player->x, player->y, player->width, player->height, 1, 1);
}
