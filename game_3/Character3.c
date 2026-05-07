#include "Character3.h"
#include "Enemy.h"
#include "LCD.h"
#include "Tileset.h"
#include "InputHandler.h"


extern int IsWall(int16_t x, int16_t y, Character_t* hero);
extern uint8_t (*current_map)[15];
extern int IsItem(int16_t x, int16_t y);
extern ROOM_ID activeRoomID;
extern Enemy_t thief;

// ===== ANIMATION SPRITES =====

const uint8_t CharacterIDLE[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {255, 255, 0, 255, 255, 0, 255, 255}
};

/**
 * @brief WALKING animation frame 1
 * 8x8 pixel sprite
 */
const uint8_t CharacterWALK1[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 255, 255, 255, 0, 0, 255},
    {255, 0, 255, 255, 0, 255, 255, 255},
    {255, 255, 0, 0, 255, 0, 255, 255},
    {255, 255, 0, 255, 255, 255, 0, 255}
};

/**
 * @brief WALKING animation frame 2
 * 8x8 pixel sprite
 */
const uint8_t CharacterWALK2[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 0, 255, 255, 255, 255, 255},
    {255, 0, 255, 255, 0, 255, 255, 255},
    {255, 255, 0, 255, 255, 0, 0, 255},
    {255, 255, 255, 0, 255, 255, 0, 255}
};



// ===== IMPLEMENTATION =====

/**
 * Initialize character at screen center
 */
void Character_Init(Character_t* character) {
    character->x = 120;
    character->y = 120;
    character->currentItem = 0; // The character does not hold the item to begin with
    character->state = CHAR_IDLE;
    character->animation_frame = 0;
    character->frame_counter = 0;
    character->dead = 0;
}

/**
 * Update character position and state
 * 
 * Movement: Use joystick->direction for 8-way movement
 * State: IDLE when stopped, WALKING when moving
 */
void Character_Update(Character_t* character, Joystick_t* joy) {
    
    // ===== STEP 1: Calculate movement based on joystick direction =====
    int16_t move_x = 0;
    int16_t move_y = 0;
    
    switch (joy->direction) {
        case N:  move_y = -1; break;
        case NE: move_x = 1; move_y = -1; break;
        case E:  move_x = 1; break;
        case SE: move_x = 1; move_y = 1; break;
        case S:  move_y = 1; break;
        case SW: move_x = -1; move_y = 1; break;
        case W:  move_x = -1; break;
        case NW: move_x = -1; move_y = -1; break;
        default: break;  // CENTRE - no movement
    }
    
    
    // ===== STEP 2: Apply movement with speed (normal) =====
    uint8_t current_speed = CHAR_SPEED;
    
    int16_t new_x = character->x + (move_x * current_speed);
    int16_t new_y = character->y + (move_y * current_speed);
    
    // ===== STEP 3: Check For Collisions
    // Check X movement independently
    if (!IsWall(new_x - 7, character->y - 7, character) && !IsWall(new_x + 7, character->y - 7, character) && // Using -7 as the sprite has radius of 8 from centre
        !IsWall(new_x - 7, character->y + 7, character) && !IsWall(new_x + 7, character->y + 7, character)) { // 7 Allows there to be space to actually move into
        
        // Screen Boundary Check
        if (new_x >= 2 && new_x <= 238) {
            character->x = new_x;
        }
    }

    // Check Y movement independently
    if (!IsWall(character->x - 7, new_y - 7, character) && !IsWall(character->x + 7, new_y - 7, character) &&
        !IsWall(character->x - 7, new_y + 7, character) && !IsWall(character->x + 7, new_y + 7, character)) { // Check seperately so the character cannot get frozen (It makes it smoother)
        
        // Screen Boundary Check
        if (new_y >= 2 && new_y <= 238) {
            character->y = new_y;
        }
    }


    // ===== STEP 4: Update state (IDLE, WALKING, DASHING) =====
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    
    if (is_moving) {
        character->state = CHAR_WALKING;
    } else {
        character->state = CHAR_IDLE;
    }
    
    // ===== STEP 5: Update animation frame for walk cycle =====
    if (character->state == CHAR_WALKING) {
        character->frame_counter++;
        if (character->frame_counter >= 10) {
            character->frame_counter = 0;
            character->animation_frame = (character->animation_frame + 1) % 2;
        }
    } else {
        character->animation_frame = 0;
        character->frame_counter = 0;
    }

    if (character -> x >228){
        if(current_map == (uint8_t (*)[15])ROOM_START){ //Going from first room to the east room
            current_map = (uint8_t (*)[15])ROOM_EAST;
            activeRoomID = ROOM_ID_EAST;
            character -> x = 8;
            thief.x -= 240;
        }
    }

    if (character -> x < 5){
        if(current_map == (uint8_t (*)[15])ROOM_EAST){ //Going from east room to first room
            current_map = (uint8_t (*)[15])ROOM_START;
            activeRoomID = ROOM_ID_START;
            character -> x = 228;
            thief.x += 240;
        }
    }

    if (character -> x >228){ //Need to be careful here to stop a loop and also because of how speed works
        if(current_map == (uint8_t (*)[15])ROOM_WEST){ //Going from west room to the first room
            current_map = (uint8_t (*)[15])ROOM_START;
            activeRoomID = ROOM_ID_START;
            character -> x = 8; // Need to be multiples of 4
            thief.x -= 240;
        }
    }

    if (character -> x < 5){
        if(current_map == (uint8_t (*)[15])ROOM_START){ //Going from first room to west room
            current_map = (uint8_t (*)[15])ROOM_WEST;
            activeRoomID = ROOM_ID_WEST;
            character -> x = 228;
            thief.x += 240;
        }
    }

    if (character -> y >228){
        if(current_map == (uint8_t (*)[15])ROOM_START){ //Going from start room to the south room
            current_map = (uint8_t (*)[15])ROOM_SOUTH;
            activeRoomID = ROOM_ID_SOUTH;
            character -> y = 8;
            thief.y += 240;
        }
    }

    if (character -> y < 5){
        if(current_map == (uint8_t (*)[15])ROOM_SOUTH){ //Going from south room to start room
            current_map = (uint8_t (*)[15])ROOM_START;
            activeRoomID = ROOM_ID_START;
            character -> y = 228;
            thief.y -= 240;
        }
    }


    uint8_t foundItem = IsItem(character->x + 7, character->y + 7);
    if (foundItem > 0 ){

        if(character->currentItem == 0){

            if(foundItem == 4){
                character->currentItem = ITEM_KEY;
            } else if (foundItem == 5) {
                character->currentItem = ITEM_SWORD;
            } else if (foundItem == 6) {
                character->currentItem = ITEM_CHALICE;
            }
            int col = (character->x + 7) / 16;
            int row = (character->y + 7) / 16;
            current_map[row][col] = 0;
        
            if (activeRoomID == ROOM_ID_START) {
                thief.x = 120;
            } 
            else if (activeRoomID == ROOM_ID_EAST) {
                thief.x = -120; 
            }
            else if (activeRoomID == ROOM_ID_WEST) {
                thief.x = 360; 
            }

            thief.itemX = row;
            thief.itemY = col;
            thief.ItemMap = current_map;

            thief.y = 120;
            thief.Active = 1;
        }else{ // To handle the swap logic
            if (current_input.btn2_pressed) { // I was having all sorts of problems so used button 9
            
                int col = (character->x + 7) / 16;
                int row = (character->y + 7) / 16;
        
                uint8_t tileToDrop = 0; // This next part ensures that the correct item gets dropped back down and not a wall by accident
                if (character->currentItem == ITEM_KEY){
                    tileToDrop = 4;
                }else if (character->currentItem == ITEM_SWORD){
                    tileToDrop = 5;
                }else if (character->currentItem == ITEM_CHALICE){
                    tileToDrop = 6;
                }

                uint8_t itemToPickUp = 0;
                if (foundItem == 4) {
                    itemToPickUp = ITEM_KEY;
                }else if (foundItem == 5){
                    itemToPickUp = ITEM_SWORD;
                }else if (foundItem == 6) {
                    itemToPickUp = ITEM_CHALICE; 
                }

                if (itemToPickUp > 0) {
                    character->currentItem = itemToPickUp;
                    current_map[row][col] = tileToDrop; // The correct tile should have been placed down
                }

            }

        }

    }
}



/**
 * Draw character sprite based on current state
 */
void Character3_Draw(Character_t* character) {
    
    int16_t x_pos = character->x - 8;  // 8x8 sprite * 4x scale = 16 offset
    int16_t y_pos = character->y - 8; // Changed to 8 from 16 as the scale ofthe character has gone down by facor of 2
    
    switch (character->state) {
        case CHAR_IDLE:
            LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8, (uint8_t*)CharacterIDLE, 5, 2); //Changed from 4 to 2
            break;
        
        case CHAR_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8, (uint8_t*)CharacterWALK1, 5, 2);
            } else {
                LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8, (uint8_t*)CharacterWALK2, 5, 2);
            }
            break;
    }
}
