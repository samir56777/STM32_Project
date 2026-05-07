#include "Obstacle.h"

void Obstacle_Init(Obstacle_t* obs, int16_t x, int16_t y, int16_t width, int16_t height, int16_t speed) {
    obs->x = x;
    obs->y = y;
    obs->width = width;
    obs->height = height;
    obs->speed = speed;
}

void Obstacle_Update(Obstacle_t* obs, int16_t player_x) {
    // Always move left (chasing direction)
    obs->x -= obs->speed;

    // Adjust toward player horizontally 
    if (obs->x > player_x) { // if obs if in front of player move left
        obs->x -= 1; // move left
    } else if (obs->x < player_x) { //if obstacle is behind player move right
        obs->x += 1; // move right
    }
}
//obstacle goes to original position
void Obstacle_Reset(Obstacle_t* obs, int16_t x, int16_t y) {
    obs->x = x;
    obs->y = y;
}

// creates a collision box for the obstacle
AABB Obstacle_GetAABB(Obstacle_t* obs) {
    AABB box;   // creates a box variable
    box.x = obs->x; // sets box x position to obstacle x position
    box.y = obs->y;    // sets box y position to obstacle y position
    box.width = obs->width;  // sets box width to obstacle width
    box.height = obs->height;   // sets box height to obstacle height
    return box;   // returns the obstacle collision box
}