#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdint.h>
#include "Utils.h"

typedef struct {// struct declared
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    int16_t speed;
    uint8_t passed;
} Obstacle_t;

//function declared
void Obstacle_Init(Obstacle_t* obs, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed);
void Obstacle_Update(Obstacle_t* obs, int16_t player_x);
void Obstacle_Reset(Obstacle_t* obs, int16_t x, int16_t y);
AABB Obstacle_GetAABB(Obstacle_t* obs);

#endif