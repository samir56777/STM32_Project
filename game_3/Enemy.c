#include "Enemy.h"
#include "Character3.h"
#include "LCD.h"

extern int IsWall(int16_t x, int16_t y, Character_t* hero);
extern int DragonsDefeated;
extern uint8_t (*current_map)[15];
int StolenItem;

// Bat Wings up

const uint8_t BatFrame1[8][8] = {
    {255, 0, 255, 255, 255, 255, 0, 255},
    {0, 0, 0, 255, 255, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 255, 0, 0, 255, 255, 255},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255}
};

// Bat Wings down
const uint8_t BatFrame2[8][8] = {
    {255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 0, 0, 255, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 255, 255, 0, 0, 0},
    {0, 255, 255, 255, 255, 255, 255, 0},
    {255, 255, 255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255, 255, 255}
};

const uint8_t DragonFrame1[8][8] = { // This was honestly the best I can do
    {255, 255, 255, 255, 0, 0, 255, 255},  // It looks awful though
    {255, 255, 0, 0, 0, 0, 0, 255},       
    {255, 0, 0, 0, 0, 255, 255, 255},    
    {0, 0, 0, 0, 0, 0, 255, 255},         
    {0, 255, 0, 255, 0, 0, 0, 255},       
    {0, 255, 0, 255, 255, 0, 0, 0},       
    {255, 255, 255, 255, 255, 255, 0, 0}, 
    {255, 255, 255, 255, 255, 255, 255, 255}
};

const uint8_t DragonFrame2[8][8] = {
    {255, 255, 255, 255, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 0, 255},
    {0, 0, 0, 0, 0, 255, 255, 255},      
    {0, 0, 0, 0, 0, 0, 255, 255},
    {255, 0, 255, 0, 0, 0, 0, 255},       
    {255, 0, 255, 255, 0, 0, 0, 255},
    {255, 255, 255, 255, 255, 0, 0, 0},   
    {255, 255, 255, 255, 255, 255, 255, 255}
};


void Enemy_Update(Enemy_t* enemy, Character_t* hero) { 
    
    if(enemy->type == ENEMY_DRAGON){
        int16_t current_vx = enemy->vx;
        int16_t current_vy = enemy->vy;

        if ((enemy->x % 16 == 8 && enemy->y % 16 == 8) || (current_vx == 0 && current_vy == 0)) { // Starting point is 120 so 120%16 = 8
        
            int valid_directions[4]; // It can go 4 directions however it needs to be more likely to move at a junction
            int count = 0;

            // The next part checks to see how many valid directions there are, as this will make it more likely for a turn
            // Checks north
            if (current_vy != 2 && !IsWall(enemy->x, enemy->y - 16, NULL)) {
                valid_directions[count++] = 0;
            }
            // Checks east
            if (current_vx != -2 && !IsWall(enemy->x + 16, enemy->y, NULL)) {
                valid_directions[count++] = 1;
            }
            // Checks south
            if (current_vy != -2 && !IsWall(enemy->x, enemy->y + 16, NULL)) {
                valid_directions[count++] = 2;
            }
            // Checks west
            if (current_vx != 2 && !IsWall(enemy->x - 16, enemy->y, NULL)) {
                valid_directions[count++] = 3;
            }

            if (count > 0) {
                int choice = valid_directions[rand() % count]; // Based off of the valid directions rather than a constant 25% chance of moving

                switch(choice) {
                    case 0: 
                        enemy->vx = 0; // North
                        enemy->vy = -2; 
                        break; 
                    case 1: 
                        enemy->vx = 2;  // East
                        enemy->vy = 0;  
                        break; 
                    case 2: 
                        enemy->vx = 0;  // South
                        enemy->vy = 2;  
                        break; 
                    case 3: 
                        enemy->vx = -2; // West
                        enemy->vy = 0;  
                        break; 
                }
            }
        }

    
        int16_t next_x = enemy->x + enemy->vx;
        int16_t next_y = enemy->y + enemy->vy;

        // Checks collisions with the wall (This part is for movement when theres no junction)
        if (IsWall(next_x - 7, next_y - 7, NULL) || IsWall(next_x + 7, next_y - 7, NULL) || 
            IsWall(next_x - 7, next_y + 7, NULL) || IsWall(next_x + 7, next_y + 7, NULL)) {
        
            enemy->vx = 0;
            enemy->vy = 0;
        } else {
            enemy->x = next_x;
            enemy->y = next_y;
        }

        if (enemy->Active == 1){
            int16_t BetweenDistanceX = enemy->x - hero->x;
            int16_t BetweenDistanceY = enemy->y - hero->y; 
            if((BetweenDistanceX*BetweenDistanceX) + (BetweenDistanceY*BetweenDistanceY) < 36){
                if (hero->currentItem == ITEM_SWORD) {
                    enemy->Active = 0;
                    DragonsDefeated ++ ;
                    if (DragonsDefeated == 2){
                        int row = (enemy->x + 7) / 16;
                        int col = (enemy->y + 7) / 16;
                        current_map[col][row] = 6; // Needs to be column then row as checks the other way around 
                    }
                }else{
                    hero->dead = 1; // Game over condition
                }
            }

        }

    }else if(enemy->type == ENEMY_THIEF){
        // As the thief can fly as its based off the bat, i dont need to worry about wall collisions
        // Check x and y separately
        if(enemy->has_stolen == 0){
            if(hero->x > enemy->x){
                enemy->vx = 1; // The player is to the right
            }else if(hero->x<enemy->x) {
                enemy->vx = -1; // The player is to the left
            }else{
                enemy->vx = 0;
            }

            if(hero->y > enemy->y){
                enemy->vy = 1; // The player is below
            }else if(hero->y< enemy->y) {
                enemy->vy = -1; // The player is above
            }else{
                enemy->vy = 0;
            }
            enemy->x += enemy->vx;
            enemy->y += enemy->vy;

            int16_t BetweenDistanceX = enemy->x - hero->x;
            int16_t BetweenDistanceY = enemy->y - hero->y; 

            // Need to use pythagoras to find the actual distance
            if((BetweenDistanceX*BetweenDistanceX) + (BetweenDistanceY*BetweenDistanceY) < 36){ // 6 Pixels
                StolenItem = hero->currentItem;
                hero->currentItem = 0; // Item is stolen 
                enemy->has_stolen = 1;    // Thief stops chasing and the if statement changes 
            }
        }else if(enemy->has_stolen==1){
            enemy->vx = -4;
            enemy->vy =0;
            enemy->x += enemy->vx;
            enemy->y += enemy->vy;

            if(enemy->x < -50){
                uint8_t tileToDrop = 0;
                if (StolenItem == ITEM_KEY){ // the wrong tile was getting placed down due to the ENUM so had to add this to make
                    tileToDrop = 4; // sure that the correct tile gets placed down after it is stolen
                }else if (StolenItem == ITEM_SWORD){ // Swaped items that are stolen get sent to where the original item was so 
                    tileToDrop = 5; // that the item that was dropped does not get overwritten
                }else if (StolenItem == ITEM_CHALICE){
                    tileToDrop = 6;
                }

                enemy->ItemMap[enemy->itemX][enemy->itemY] = tileToDrop;
                enemy->Active = 0;
                enemy->has_stolen = 0;
            }
        }

    }
    enemy->frame_counter++;
    if (enemy->frame_counter >= 8) { 
    enemy->frame_counter = 0; // Frame counter for the enemies
    enemy->animation_frame = (enemy->animation_frame + 1) % 2; // They both use this which probably isnt great but oh well
    }

}

void Draw_Enemy(Enemy_t *enemy){
    if(enemy->type == ENEMY_DRAGON){
        switch (enemy->animation_frame) {
        case 0:
            LCD_Draw_Sprite_Colour_Scaled(enemy->x - 8, enemy->y - 8, 8, 8, (uint8_t*)DragonFrame1, 3, 2);
            break;
        case 1:
            LCD_Draw_Sprite_Colour_Scaled(enemy->x - 8, enemy->y - 8, 8, 8, (uint8_t*)DragonFrame2, 3, 2);
            break;
        }
    }else if(enemy->type == ENEMY_THIEF){
        switch (enemy->animation_frame) {
        case 0:   
            LCD_Draw_Sprite_Colour_Scaled(enemy->x - 8, enemy-> y - 8, 8, 8, (uint8_t*)BatFrame1, 13, 2);
            break;
        case 1:
            LCD_Draw_Sprite_Colour_Scaled(enemy-> x - 8, enemy-> y - 8, 8, 8, (uint8_t*)BatFrame2, 13, 2); 
            break;
        }
    }
}

void Enemy_Init(Enemy_t* enemy, EnemyType type, ROOM_ID room){
    enemy->type = type;
    enemy->currentRoomID = room;
    enemy->has_stolen = 0;
    enemy->frame_counter = 0;
    enemy->animation_frame = 0;

    if (enemy->currentRoomID == ROOM_ID_EAST){ // Can use this to change where an enemy spawns based on the room
        enemy->x = 120;
        enemy->y = 120;
    }else if (enemy->currentRoomID == ROOM_ID_START) {
        enemy->x = 120;
        enemy->y = 120;
    }else if (enemy->currentRoomID == ROOM_ID_WEST) {
        enemy->x = 120;
        enemy->y = 120;
    }else if (enemy->currentRoomID == ROOM_ID_SOUTH) {
        enemy->x = 120;
        enemy->y = 120;
    }
    
    switch(type) {
        case ENEMY_DRAGON:
            enemy->vx = 2; 
            enemy->vy = 0;
            enemy->Active = 1; // Need to set the dragon as active to start
            break;
        case ENEMY_THIEF:
            enemy->vx = 2; 
            enemy->vy = 0;
            break;
        default:
            enemy->vx = 0;
            enemy->vy = 0;
            break;
    }
}