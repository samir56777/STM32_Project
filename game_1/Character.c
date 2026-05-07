#include "Character.h"

#define CHARACTER_SPRITE_W 8
#define CHARACTER_SPRITE_H 8
#define CHARACTER_SCALE    2

// ===== ANIMATION SPRITES =====

static const uint8_t PlayerIDLE[8][8] = {
    {255, 255, 4,   4,   4,   4,   255, 255},
    {255, 1,   2,   2,   2,   2,   1,   255},
    {255, 1,   2,   1,   1,   2,   1,   255},
    {255, 1,   1,   5,   5,   1,   1,   255},
    {255, 255, 1,   5,   5,   1,   255, 255},
    {255, 255, 1,   5,   5,   1,   255, 255},
    {255, 255, 1,   8,   8,   1,   255, 255},
    {255, 255, 8,   255, 255, 8,   255, 255}
};

static const uint8_t EnemyIDLE[8][8] = {
    {255, 255, 8,   8,   8,   8,   255, 255},
    {255, 1,   3,   3,   3,   3,   1,   255},
    {255, 1,   3,   1,   1,   3,   1,   255},
    {255, 1,   1,   6,   6,   1,   1,   255},
    {255, 255, 1,   6,   6,   1,   255, 255},
    {255, 255, 1,   7,   7,   1,   255, 255},
    {255, 255, 1,   8,   8,   1,   255, 255},
    {255, 255, 8,   255, 255, 8,   255, 255}
};

/**
 * @brief WALKING animation frame 1
 * 8x8 pixel sprite
 */
static const uint8_t PlayerWALK1[8][8] = {
    {255, 255, 4,   4,   4,   4,   255, 255},
    {255, 1,   2,   2,   2,   2,   1,   255},
    {255, 1,   2,   1,   1,   2,   1,   255},
    {255, 1,   1,   5,   5,   1,   1,   255},
    {255, 255, 1,   5,   5,   1,   255, 255},
    {255, 1,   5,   5,   5,   5,   1,   255},
    {255, 1,   8,   1,   1,   8,   1,   255},
    {255, 8,   8,   255, 255, 8,   8,   255}
};

/**
 * @brief WALKING animation frame 2
 * 8x8 pixel sprite
 */
static const uint8_t PlayerWALK2[8][8] = {
    {255, 255, 4,   4,   4,   4,   255, 255},
    {255, 1,   2,   2,   2,   2,   1,   255},
    {255, 1,   2,   1,   1,   2,   1,   255},
    {255, 1,   1,   5,   5,   1,   1,   255},
    {255, 255, 1,   5,   5,   1,   255, 255},
    {255, 1,   5,   5,   5,   5,   1,   255},
    {255, 1,   1,   8,   8,   1,   1,   255},
    {255, 8,   255, 255, 255, 255, 8,   255}
};

static const uint8_t EnemyWALK1[8][8] = {
    {255, 255, 8,   8,   8,   8,   255, 255},
    {255, 1,   3,   3,   3,   3,   1,   255},
    {255, 1,   3,   1,   1,   3,   1,   255},
    {255, 1,   1,   6,   6,   1,   1,   255},
    {255, 255, 1,   6,   6,   1,   255, 255},
    {255, 1,   7,   7,   7,   7,   1,   255},
    {255, 1,   8,   1,   1,   8,   1,   255},
    {255, 8,   8,   255, 255, 8,   8,   255}
};

static const uint8_t EnemyWALK2[8][8] = {
    {255, 255, 8,   8,   8,   8,   255, 255},
    {255, 1,   3,   3,   3,   3,   1,   255},
    {255, 1,   3,   1,   1,   3,   1,   255},
    {255, 1,   1,   6,   6,   1,   1,   255},
    {255, 255, 1,   6,   6,   1,   255, 255},
    {255, 1,   7,   7,   7,   7,   1,   255},
    {255, 1,   1,   8,   8,   1,   1,   255},
    {255, 8,   255, 255, 255, 255, 8,   255}
};


// Draw character sprite
void Character_Draw(const Character_t* character)
{
    const uint8_t (*sprite)[8] = PlayerIDLE;
    int16_t x_pos = character->x;
    int16_t y_pos = character->y;

    switch (character->type)
    {
        case CHAR_ENEMY:
            if (character->state == CHAR_WALKING)
            {
                sprite = (character->animation_frame == 0) ? EnemyWALK1 : EnemyWALK2;
            }
            else
            {
                sprite = EnemyIDLE;
            }
            break;

        case CHAR_PLAYER:
        default:
            if (character->state == CHAR_WALKING)
            {
                sprite = (character->animation_frame == 0) ? PlayerWALK1 : PlayerWALK2;
            }
            else
            {
                sprite = PlayerIDLE;
            }
            break;
    }

    LCD_Draw_Sprite_Scaled(x_pos, y_pos, CHARACTER_SPRITE_W, CHARACTER_SPRITE_H, (uint8_t*)sprite, CHARACTER_SCALE);
}

// Walking animation check and update
void Update_Animation(Character_t *character)
{
    character->frame_counter++;

    if (character->frame_counter >= 5)
    {
        character->frame_counter = 0;
        character->animation_frame = 1 - character->animation_frame;
    }
}