#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>
#include "Character3.h"

typedef enum {
    ENEMY_NONE,
    ENEMY_DRAGON,
    ENEMY_THIEF
} EnemyType;

typedef enum{
    ROOM_ID_START,
    ROOM_ID_EAST,
    ROOM_ID_SOUTH,
    ROOM_ID_WEST
}ROOM_ID;

typedef struct {
    int16_t x, y; //Current position
    int16_t vx, vy; //Velocity (The direction and speed)
    EnemyType type;
    ROOM_ID currentRoomID;
    uint8_t has_stolen; //Only for the thief
    uint8_t Active; // For the thief when its moving
    int16_t itemX, itemY; // The original coordinates for the stolen item
    uint8_t (*ItemMap)[15]; // The map those coordinates come from
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Counter for animation timing
} Enemy_t;



void Enemy_Init(Enemy_t* enemy, EnemyType type, ROOM_ID room);
void Enemy_Update(Enemy_t* enemy, Character_t* hero);
void Draw_Enemy(Enemy_t* enemy);
#endif