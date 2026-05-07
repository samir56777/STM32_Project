#include "Game_1.h"
#include "LCD.h"
#include "Joystick.h"
#include "InputHandler.h"
#include "Menu.h"
#include "stm32l4xx_hal.h"
#include "Character.h"
#include <stdint.h>
#include <Tileset.h>
#include "Enemy.h"



// These connect to the ones defined in your main.c
extern ST7789V2_cfg_t cfg0;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
uint8_t (*current_map)[15];
ROOM_ID activeRoomID;
Enemy_t thief; // had to make it global
Enemy_t dragon1;
Enemy_t dragon2;
int DragonsDefeated;



int IsItem(int16_t x, int16_t y){
    int col = x/16;
    int row = y/16;
    if (current_map[row][col] >= 4){
        return current_map[row][col]; //Returns the items value so the item can be determined
    }
return 0;
}

void ItemHeld(Character_t* hero){  //Effectively the inventory slot
    if (hero->currentItem == ITEM_KEY){
        LCD_Draw_Sprite(0,0,16,16,DOOR_KEY); // Draws the item held in the top left corner
    }else if(hero->currentItem == ITEM_SWORD){
        LCD_Draw_Sprite(0,0,16,16,SWORD);
    }else if(hero->currentItem == ITEM_CHALICE){
        LCD_Draw_Sprite(0,0,16,16,CHALICE);
    } 
}


int IsWall(int16_t x, int16_t y, Character_t* hero){
    int col = x/16;
    int row = y/16;

    if (col <0 || col >=15 || row <0 || row>=15){
        return 1;
    }

    if (current_map[row][col] == 1 || current_map[row][col] == 2){ // Any wall block
        return 1;
    }
    if (current_map[row][col] == 3){
        if (hero->currentItem == ITEM_KEY){
            current_map[row][col] = 0;
            hero->currentItem = 0;
            thief.Active = 0;
            return 0;
        }else{
            return 1;
        }
    }

    return 0; // If its floor then the player can move
}    

void draw_tile(int16_t x, int16_t y, const uint8_t *tile_data) {
    //LCD_Draw_Sprite(x, y, rows, cols, data_pointer)
    LCD_Draw_Sprite(x, y, 16, 16, tile_data); //Draws the entire square rather than 256 individual rectangles
}


void Game1_DrawMap() {
    for (int r = 0; r < 15; r++) {
        for (int c = 0; c < 15; c++) { // The grid is 15x15 when using 16x16 squares
            int16_t x_pos = c * 16;
            int16_t y_pos = r * 16;

            uint8_t tile = current_map[r][c];

            if (tile == 0) { // Checks to see what tile needs to be drawn
                draw_tile(x_pos, y_pos, tile_floorland);
            } else if (tile == 1) {
                draw_tile(x_pos, y_pos, tile_wall_1);
            } else if (tile == 2) {
                draw_tile(x_pos, y_pos, tile_wall_2);
            } else if (tile == 3){
                draw_tile(x_pos,y_pos,tile_door);
            } else if (tile == 4){
                draw_tile(x_pos, y_pos, DOOR_KEY);
            }else if (tile == 5){
                draw_tile(x_pos, y_pos, SWORD);
            }else if (tile == 6){
                draw_tile(x_pos, y_pos, CHALICE);
            }
        }
    }
}


MenuState Game1_Run(void) {
    // Initialising the character
    Character_t hero;
    Character_Init(&hero);

    Enemy_Init(&dragon1, ENEMY_DRAGON,ROOM_ID_EAST);
    Enemy_Init(&dragon2, ENEMY_DRAGON,ROOM_ID_EAST);
    Enemy_Init(&thief, ENEMY_THIEF,ROOM_ID_START);


    current_map = (uint8_t (*)[15])ROOM_START;
    activeRoomID = ROOM_ID_START;
    GameState_t currentState = STATE_PLAYING; // This will change if the character wins or dies

    while (1) {
        uint32_t frame_start = HAL_GetTick();

        // --- INPUT ---
        Joystick_Read(&joystick_cfg, &joystick_data);
        Input_Read(); // Reads BT2/BT3

        // --- LOGIC ---
        // Exit back to menu if BT3 is pressed
        if (current_input.btn3_pressed) {
            return MENU_STATE_HOME;
        }

        // --- RENDER ---
        LCD_Fill_Buffer(0); // Clear screen
        LCD_Set_Palette(PALETTE_VINTAGE);

        if (currentState == STATE_PLAYING){
        
            // Draw the player
            Character_Update(&hero, &joystick_data);
            Game1_DrawMap();
            ItemHeld(&hero);
            Character_Draw(&hero);

            if(thief.Active == 1){
                Enemy_Update(&thief, &hero);
                Draw_Enemy(&thief);
            }
            if(dragon1.Active == 1 && dragon1.currentRoomID == activeRoomID){
                Enemy_Update(&dragon1, &hero);
                Draw_Enemy(&dragon1);
            }
            if(dragon2.Active == 1 && dragon2.currentRoomID == activeRoomID){
                Enemy_Update(&dragon2, &hero);
                Draw_Enemy(&dragon2);
            }

            if (hero.currentItem == ITEM_CHALICE){
                currentState = STATE_WIN; // I just made it win if the character has the chalice as i ran out of time
            }else if(hero.dead == 1){
                currentState = STATE_GAMEOVER;
            }
        }else if(currentState == STATE_GAMEOVER){
            LCD_Fill_Buffer(0);
            LCD_printString("GAME OVER!", 30, 75, 1, 3); // Prints a message
        }else if (currentState == STATE_WIN){
            LCD_Fill_Buffer(0);
            LCD_printString("YOU WIN!", 30, 75, 1, 3);

        }

        LCD_Refresh(&cfg0);

        // --- TIMING ---
        // Maintain roughly 30 FPS
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < 33) {
            HAL_Delay(33 - frame_time);
        }
    }
}