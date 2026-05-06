#ifndef GAME_2_H
#define GAME_2_H

#include <stdint.h>
#include <stdbool.h>
#include "Menu.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define SPRITE_SCALE 2

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 46
#define PLAYER_Y 185
#define PLAYER_SPEED 5

#define MAX_BULLETS 6
#define BULLET_WIDTH 4
#define BULLET_HEIGHT 10
#define BULLET_SPEED 8

#define MAX_ENEMIES 6
#define ENEMY_WIDTH 24
#define ENEMY_HEIGHT 28
#define ENEMY_SPEED_MIN 1
#define ENEMY_SPEED_MAX 3

#define STAR_COUNT 20

typedef enum {
    GAME_STATE_START = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER
} GameState;

typedef struct {
    int x;
    int y;
    int lives;
} PlayerState;

typedef struct {
    int x;
    int y;
    bool active;
} Bullet;

typedef struct {
    int x;
    int y;
    int speed;
    uint8_t color;
    bool active;
} Enemy;

typedef struct {
    int x;
    int y;
    int speed;
    uint8_t color;
} Star;

typedef struct {
    GameState state;
} Game_2;

MenuState Game2_Run(void);

void Game_2_Init(void);
void Game_2_Reset(void);
void Game_2_Update(void);
void Game_2_Render(void);

#endif