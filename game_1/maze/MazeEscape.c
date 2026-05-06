/**
 * @file maze_escape.c
 * @brief Main game engine implementation for Maze Escape
 * 
 * Handles collision detection and game logic.
 */

#include "MazeEscape.h"
#include "Buzzer.h"
#include "Map.h"
#include "LCD.h"  


// Screen dimensions 
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define BALL_RESET_OFFSET 20
#define BUZZER_WALL_FREQ_HZ 1200
#define BUZZER_PLAYER_FREQ_HZ 800
#define BUZZER_VOLUME 50
#define DETECTED_SCREEN_MS 2800
#define COLOUR_RED_SCREEN 2
#define COLOUR_TEXT 1
#define COLOUR_ALARM 6



extern Buzzer_cfg_t buzzer_cfg;

static const Buzzer_Note_t detected_melody_notes[] = {
    NOTE_A5,
    NOTE_F5,
    NOTE_D5,
    NOTE_E5
};

static const uint16_t detected_melody_durations_ms[] = {
    120,
    120,
    160,
    220
};

static uint32_t melody_next_tick = 0;
static uint8_t melody_note_index = 0;
static uint8_t melody_playing = 0;

/**
 * @brief Start the alarm melody when the player is detected
 *
 */
static void MazeEscape_StartDetectedMelody(void)
{
    melody_note_index = 0;
    melody_playing = 1;
    melody_next_tick = 0;
}



static void MazeEscape_UpdateBuzzer(void)
{
    if (!melody_playing) {
        return;
    }

    uint32_t now = HAL_GetTick();
    uint32_t note_count = sizeof(detected_melody_notes) / sizeof(detected_melody_notes[0]);

    if (melody_next_tick != 0 && (int32_t)(now - melody_next_tick) < 0) {
        return;
    }

    if (melody_note_index >= note_count) {
        buzzer_off(&buzzer_cfg);
        melody_playing = 0;
        melody_next_tick = 0;
        return;
    }

    buzzer_note(&buzzer_cfg, detected_melody_notes[melody_note_index], BUZZER_VOLUME);
    melody_next_tick = now + detected_melody_durations_ms[melody_note_index];
    melody_note_index++;
}

/**
 * @brief Handle ball collision with screen walls
 * 
 * Bounces ball off top, bottom, and right edges of screen.
 * Left edge is handled by goal logic (missed paddle).
 * 
 * @param engine Pointer to game engine
 */
    

/**
 * @brief Handle ball collision with paddle using AABB
 * 
 * Uses AABB (Axis-Aligned Bounding Box) collision detection:
 * - Gets bounding boxes for both ball and paddle
 * - Checks if they overlap using AABB_Collides()
 * - If collision detected, reverses ball X velocity and increments score
 * 
 * **How AABB Collision Works:**
 * Two boxes collide if they overlap in BOTH X and Y axes:
 * - X-axis overlap: box_a.x < box_b.x+box_b.width AND box_a.x+box_a.width > box_b.x
 * - Y-axis overlap: box_a.y < box_b.y+box_b.height AND box_a.y+box_a.height > box_b.y
 * 
 * @param engine Pointer to game engine
 */
/*static void PongEngine_CheckPaddleCollision(PongEngine_t* engine) {
    Ball_t* ball = &engine->ball;
    Paddle_t* paddle = &engine->paddle;
    Vector2D vel = Ball_GetVelocity(ball);
    
    // Get bounding boxes for collision detection
    AABB ball_box = Ball_GetAABB(ball);
    AABB paddle_box = Paddle_GetAABB(paddle);
    
    // Check if ball and paddle AABBs collide
    if (AABB_Collides(&ball_box, &paddle_box)) {
        // Collision detected! Reverse ball X velocity
        Ball_SetVelocity(ball, -vel.x, vel.y);
        
        // Move ball to prevent getting stuck in paddle
        ball->x = paddle_box.x + paddle_box.width;
        
        // Increment paddle score
        Paddle_AddScore(paddle);

        PongEngine_Beep(BUZZER_PADDLE_FREQ_HZ);
    }
}*/

/**
 * @brief Check if ball has left the play area (missed paddle)
 * 
 * If ball goes beyond left edge, decrement lives and reset ball to center.
 * 
 * @param engine Pointer to game engine
 */
/*static void PongEngine_CheckGoal(PongEngine_t* engine) {
    Ball_t* ball = &engine->ball;
    
    // Ball left the left edge (missed by paddle)
    if (ball->x < 0) {
        engine->lives--;

        // Reset ball near center with random offset
        Position2D center_pos;
        center_pos.x = (SCREEN_WIDTH - ball->size) / 2;
        center_pos.y = (SCREEN_HEIGHT - ball->size) / 2;

        // Add a small random offset in range [-BALL_RESET_OFFSET, BALL_RESET_OFFSET] to vary the reset position
        int16_t dx = (int16_t)Random_U16((uint16_t)(2 * BALL_RESET_OFFSET + 1)) - BALL_RESET_OFFSET;
        int16_t dy = (int16_t)Random_U16((uint16_t)(2 * BALL_RESET_OFFSET + 1)) - BALL_RESET_OFFSET;

        center_pos.x += dx;
        center_pos.y += dy;

        Ball_SetPos(ball, center_pos);

        // Reset velocity to initial direction (move right)
        Ball_SetVelocity(ball, 8.0f * 0.707f, 8.0f * 0.707f);
    }
}*/
static void MazeEscape_DrawAlarmSprite(void)
{
    LCD_Draw_Line(120, 55, 95, 95, COLOUR_ALARM);
    LCD_Draw_Line(95, 95, 145, 95, COLOUR_ALARM);
    LCD_Draw_Line(145, 95, 120, 55, COLOUR_ALARM);

    LCD_Draw_Rect(118, 68, 4, 14, COLOUR_ALARM, 1);
    LCD_Draw_Rect(118, 86, 4, 4, COLOUR_ALARM, 1);
}

static void MazeEscape_DrawDetectedScreen(void)
{
    LCD_Fill_Buffer(COLOUR_RED_SCREEN);

    MazeEscape_DrawAlarmSprite();

    LCD_printString("You have been", 36, 120, COLOUR_TEXT, 2);
    LCD_printString("detected!", 66, 145, COLOUR_TEXT, 2);
    LCD_printString("Try again", 72, 175, COLOUR_TEXT, 2);
}

static void MazeEscape_LoadLevel(MazeEscape_t* engine, uint8_t level)

{    
    for (int i = 0; i < GUARD_COUNT; i++) {
    engine->guards[i].x = -100;
    engine->guards[i].y = -100;
}

    engine->level = level;
    engine->alarm_triggered = 0;
    engine->detected_screen_active = 0;
    engine->detected_screen_until = 0;


    Map_SetLevel(level);

    engine->paddle.x = 20;
    engine->paddle.y = 20;

    // Clear all guards by moving them off the screen
   
    if(level==0){
        Guard_Init(&engine->guards[0], 50, 100);
    Guard_Init(&engine->guards[1], 200, 70);
    Guard_Init(&engine->guards[2], 100, 200);
    engine->guards[2].angle = 166.0;
    }
    if (level == 1) {
        Guard_Init(&engine->guards[0], 30, 75);
        Guard_Init(&engine->guards[1], 170, 75);
        Guard_Init(&engine->guards[2], 30, 120);
            Guard_Init(&engine->guards[3], 170, 120);
            Guard_Init(&engine->guards[4], 30, 170);
            Guard_Init(&engine->guards[5], 170, 170);
        
    engine->guards[0].speed = 3.0f;
    engine->guards[1].speed = 3.0f;
    engine->guards[2].speed = 3.0f;
    engine->guards[3].speed = 3.0f;
    engine->guards[4].speed = 3.0f;
    engine->guards[5].speed = 3.0f;
    }
    else if (level == 2) {
    Guard_Init(&engine->guards[0], 204, 24);

    Guard_Init(&engine->guards[1], 12, 84);
    
    Guard_Init(&engine->guards[2], 140, 96);
    Guard_Init(&engine->guards[3], 12, 216);
    engine->guards[1].speed = 5.0f;
    }
}

void MazeEscape_Init(MazeEscape_t* engine,
                     int16_t player_x, int16_t player_y,
                     int16_t player_width, int16_t player_height
                     
                     ) {
    
    Paddle_Init(&engine->paddle, player_x, player_y, 
                player_width, player_height, 6);  



    engine->alarm_triggered = 0;
    engine->detected_screen_active = 0;
    engine->detected_screen_until = 0;
    MazeEscape_LoadLevel(engine, 0);

    // Initialize lives
    /*engine->lives = 4;*/ // Give player 4 lives
}

uint8_t MazeEscape_Update(MazeEscape_t* engine, UserInput input) {
if (engine->game_finished) {
if (input.direction != CENTRE) {
    engine->game_finished = 0;
    MazeEscape_LoadLevel(engine, 0);
    engine->paddle.x = 20;
    engine->paddle.y = 20;
}
        return 0;
    }
    Paddle_Update(&engine->paddle, input);
    if (engine->detected_screen_active) {
        MazeEscape_UpdateBuzzer();
        if ((int32_t)(HAL_GetTick() - engine->detected_screen_until) >= 0) {
            engine->detected_screen_active = 0;
            engine->paddle.x = 20;
            engine->paddle.y = 20;
        }

        return 0;
    }


    // Update all guards
    for (int i = 0; i < GUARD_COUNT; i++) {
        Guard_Update(&engine->guards[i]);
    }

    // Check if any guard can see the player
    engine->alarm_triggered = 0;
    for (int i = 0; i < GUARD_COUNT; i++) {
        if (Guard_CanSeePlayer(&engine->guards[i],
                               engine->paddle.x,
                               engine->paddle.y,
                               engine->paddle.width,
                               engine->paddle.height)) {
            engine->alarm_triggered = 1;
            break;
        }
    }

    // If caught by guard, show detection screen
    if (engine->alarm_triggered) {
        engine->detected_screen_active = 1;
        engine->detected_screen_until = HAL_GetTick() + DETECTED_SCREEN_MS;
        MazeEscape_StartDetectedMelody();
        MazeEscape_UpdateBuzzer();
        return 0;
    }

    // If caught by guard, reset to start
    if (engine->alarm_triggered) {
        engine->paddle.x = 20;
        engine->paddle.y = 20;
    }

    // Check if player reached the exit tile
    if (Map_IsExit(engine->paddle.x, engine->paddle.y)) {
        uint8_t level = Map_GetLevel();

        if (level < 2) {
            // Go to next level
            MazeEscape_LoadLevel(engine, level + 1);
        } else {
            // Finished level 3
            // For now, restart from level 1
            engine->game_finished = 1;
        }
    }

    MazeEscape_UpdateBuzzer();

    return 0;
}

void MazeEscape_Draw(MazeEscape_t* engine) {
    if (engine->game_finished) {
    LCD_Fill_Buffer(3);  // green background
LCD_Draw_Rect(20, 40, 200, 60, 1, 1);   // white box
LCD_printString("WELL DONE", 40, 58, 3, 3);

LCD_printString("You escaped!", 58, 130, 1, 2);
LCD_printString("Move joystick", 52, 180, 1, 2);
LCD_printString("to start again", 46, 205, 1, 2);

    return;
}
        if (engine->detected_screen_active) {
        MazeEscape_DrawDetectedScreen();
        return;
    }
    Map_Draw();
    Paddle_Draw(&engine->paddle);
        for (int i = 0; i < GUARD_COUNT; i++) {
        Guard_Draw(&engine->guards[i]);
    }
}

/*uint8_t MazeEscape_GetLevel(MazeEscape_t* engine) {
    return engine->level;
}*/

/*uint16_t PongEngine_GetScore(PongEngine_t* engine) {
    return Paddle_GetScore(&engine->paddle);
}*/
