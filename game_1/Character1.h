#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdint.h>
#include "Joystick.h"
#include "LCD.h"

typedef enum {
    CHAR_PLAYER = 0,
    CHAR_ENEMY
} CharacterType_t;

typedef enum {
    CHAR_IDLE = 0,
    CHAR_WALKING
} CharacterState_t;

typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    CharacterType_t type;           // Player or guard sprite set
    CharacterState_t state;         // Current state
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Counter for animation timing
} Character_t;

// ===== CONSTANTS =====

#define CHAR_SPEED 2                // Pixels per frame (normal)

// ===== FUNCTIONS =====

/**
 * @brief Draw character sprite on LCD
 */
void Character_Draw(const Character_t* character);

void Update_Animation(Character_t* character);

#endif