#ifndef RUNNER_ENGINE_H
#define RUNNER_ENGINE_H
#define MAX_OBSTACLES 3


#include <stdint.h>
#include "Obstacle.h"
#include "Joystick.h"

typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} Player_t;

typedef struct {
    Player_t player;
    Obstacle_t obstacles[MAX_OBSTACLES];
    uint8_t obstacleCount;

    uint8_t isJumping;
    uint32_t jumpStartTick;
    uint8_t jumpMoveWorld;

    uint8_t isRunning;
    uint8_t runFrame;
    uint32_t lastRunAnimTick;
    uint16_t score;
    

    Obstacle_t bull;
    uint8_t bullActive;
    uint8_t gameOver;
} RunnerEngine_t;

void RunnerEngine_Init(RunnerEngine_t* engine);
uint8_t RunnerEngine_Update(RunnerEngine_t* engine, UserInput input, uint8_t jumpPressed);
void RunnerEngine_Draw(RunnerEngine_t* engine);
uint16_t RunnerEngine_GetScore(RunnerEngine_t* engine);
static void RunnerEngine_SpawnObstacles(RunnerEngine_t* engine);

#endif