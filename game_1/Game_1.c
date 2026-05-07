#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "Character.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;               // LED PWM control
extern Buzzer_cfg_t buzzer_cfg;         // Buzzer control
extern Joystick_cfg_t joystick_cfg;     // Joystick control
extern Joystick_t joystick_data;

// Screen dimensions
#define SCREEN_W              240
#define SCREEN_H              240

// Control frame rate 33fps
#define FRAME_TICKS            30

// Player tuning
#define PLAYER_W               16
#define PLAYER_H               16
#define PLAYER_MOVE_STEP        6     // Horizontal movement speed
#define PLAYER_JUMP_VY         -9     // Initial jump velocity
#define PLAYER_GRAVITY          1
#define PLAYER_MAX_FALL_VY      8     // Terminal fall velocity

// Level tuning
#define START_LIVES              3
#define REQUIRED_COINS           10
#define TOTAL_LEVELS             2

// Respawn time
#define ENEMY_RESPAWN_TIME      8000

// Combat tuning
#define PLAYER_MAX_HP         100
#define ENEMY_MAX_HP          40
#define ENEMY_ATTACK_RANGE    20
#define ENEMY_ATTACK_DAMAGE   10
#define ENEMY_ATTACK_COOLDOWN 1000
#define MAX_ENEMIES           10

// Game object colours
#define COLOUR_TEXT             1
#define COLOUR_PLAYER           15
#define COLOUR_ENEMY            8
#define COLOUR_COIN             11
#define COLOUR_PLATFORM         0

// Coordinates for drawing, physics, enemy spawn and platforms
typedef struct
{
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} Coordinate_t;

// Internal FSM for the game state itself.
typedef enum
{
    STATE_INTRO = 0,
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE
} FSM_State_t;

// Enemy state used for multiple enemies.
typedef struct
{
    Character_t enemy;
    uint8_t enemy_alive;
    int16_t enemy_hp;
    uint32_t enemy_death_time;
    int8_t enemy_dir;
    uint32_t enemy_last_attack_time;
} Enemy_t;

// Game state used in the main loop and rendering.
typedef struct
{
    FSM_State_t mode;

    // Player state
    Character_t player;
    int16_t player_vy;
    uint8_t grounded;
    int16_t player_hp;

    // Level progression
    uint8_t lives;
    uint8_t score;

    // Enemy
    Enemy_t enemies[MAX_ENEMIES];
    uint8_t enemy_count;
    uint32_t next_enemy_spawn_time;

    // Collectible coin dropped by enemies
    uint8_t coin_active;
    int16_t coin_x;
    int16_t coin_y;

} State_t;

// static world objects
static const Coordinate_t g_level_platforms[] =
{
    // Ground
    {   0, 190, 240, 60 },

    // Platforms
    {   0, 160, 240, 8 },
    {   0, 130, 240, 8 },
    {   0,  90, 240, 8 },
    {   0,  50, 240, 8 },
};

#define NUM_PLATFORMS 5   // number of platforms


// ---------------------------------------------------------------------------
// Helper functions for main game loop
// ---------------------------------------------------------------------------

/**
 * @brief Clamp a signed 16-bit value.
 */
static int16_t BoundCheck(int16_t value, int16_t min, int16_t max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * @brief test to see if rectangles overlap
 */
static uint8_t RectangleOverlap(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                                int16_t bx, int16_t by, int16_t bw, int16_t bh)
{
    if ((ax + aw) <= bx) return 0;
    if (ax >= (bx + bw)) return 0;
    if ((ay + ah) <= by) return 0;
    if (ay >= (by + bh)) return 0;
    return 1;
}

/**
 * @brief Read joystick input and convert it into movement system
 */
static void HandleInput(State_t *g, UserInput input)
{
    g->player.state = CHAR_IDLE;

    // Horizontal movement.
    if ((input.direction == W) || (input.direction == NW) || (input.direction == SW))
    {
        g->player.x -= PLAYER_MOVE_STEP;
        g->player.state = CHAR_WALKING;
    }
    else if ((input.direction == E) || (input.direction == NE) || (input.direction == SE))
    {
        g->player.x += PLAYER_MOVE_STEP;
        g->player.state = CHAR_WALKING;
    }

    // Jump.
    // Jump from the ground only, but allow diagonal inputs to count as jump + move.
    if ((input.direction == N) || (input.direction == NE) || (input.direction == NW))
    {
        if (g->grounded)
        {
            g->player_vy = PLAYER_JUMP_VY;
            g->grounded = 0;
        }
    }

    // Drop through
    if (input.direction == S)
    {
        g->grounded = 1;
        g->player.y = g->player.y + 1;
        g->player_vy = -1;
    }

    // Keep the player inside the screen.
    g->player.x = BoundCheck(g->player.x, 0, SCREEN_W - PLAYER_W);
}

/**
 * @brief Apply gravity and resolve landing on top of platforms.
 */
static void ApplyPhysics(State_t *g)
{
    int16_t old_bottom = g->player.y + PLAYER_H;

    // Gravity.
    g->player_vy += PLAYER_GRAVITY;
    g->player_vy = BoundCheck(g->player_vy, -32, PLAYER_MAX_FALL_VY);

    // Vertical movement.
    g->player.y += g->player_vy;

    // Begin off ground until collision detected.
    g->grounded = 0;

    // Check against every platform in the level.
    for (uint8_t i = 0; i < NUM_PLATFORMS; i++)
    {
        const Coordinate_t *p = &g_level_platforms[i];

        // Resolve landing when falling or standing still.
        if (g->player_vy >= 0)
        {
            // Horizontal overlap with platform?
            if (RectangleOverlap(g->player.x, g->player.y, PLAYER_W, PLAYER_H,
                                 p->x, p->y, p->w, p->h))
            {
                // Did the player cross the top of the platform this frame?
                if ((old_bottom <= p->y) && ((g->player.y + PLAYER_H) >= p->y))
                {
                    g->player.y = p->y - PLAYER_H;
                    g->player_vy = 0;
                    g->grounded = 1;
                }
            }
        }
    }
}

/**
 * @brief Spawn the enemy offscreen onto random platform
 */
static void Spawnenemy(State_t *g, uint8_t enemy_index)
{
    Enemy_t *e;

    if (enemy_index >= MAX_ENEMIES)
    {
        return;
    }

    e = &g->enemies[enemy_index];

    // Spawn on a random side, offscreen.
    if ((rand() % 2) == 0)
    {
        e->enemy.x = -2;
        e->enemy_dir = 1;
    }
    else
    {
        e->enemy.x = SCREEN_W + 2;
        e->enemy_dir = -1;
    }

    // Spawn enemy on random platform.
    uint8_t platform_index = (rand() % (NUM_PLATFORMS - 1)) + 1;
    e->enemy.y = g_level_platforms[platform_index].y - PLAYER_H;

    e->enemy.type = CHAR_ENEMY;
    e->enemy.state = CHAR_WALKING;
    e->enemy.animation_frame = 0;
    e->enemy.frame_counter = 0;

    e->enemy_hp = ENEMY_MAX_HP;
    e->enemy_alive = 1;
    e->enemy_death_time = 0;
    e->enemy_last_attack_time = 0;
}

/**
 * @brief Patrol enemy AI.
 */
static void UpdateEnemy(State_t *g)
{
    uint32_t now = HAL_GetTick();

    // Spawn another enemy every 8 seconds until the maximum is reached.
    if ((g->enemy_count < MAX_ENEMIES) && ((int32_t)(now - g->next_enemy_spawn_time) >= 0))
    {
        Spawnenemy(g, g->enemy_count);
        g->enemy_count++;
        g->next_enemy_spawn_time = now + ENEMY_RESPAWN_TIME;
    }

    for (uint8_t i = 0; i < g->enemy_count; i++)
    {
        Enemy_t *e = &g->enemies[i];

        if (e->enemy_alive == 0)
        {
            if ((HAL_GetTick() - e->enemy_death_time) >= ENEMY_RESPAWN_TIME)
            {
                Spawnenemy(g, i);
            }

            continue;
        }

        // Movement step.
        e->enemy.x += e->enemy_dir * 2;

        // If the enemy reaches the edges, turn it around.
        if (e->enemy.x <= -12)
        {
            e->enemy.x = -12;
            e->enemy_dir = 1;
        }
        else if (e->enemy.x >= SCREEN_W)
        {
            e->enemy.x = SCREEN_W;
            e->enemy_dir = -1;
        }


        // If the enemy is close enough, attack the player.
        if (RectangleOverlap(g->player.x, g->player.y, PLAYER_W, PLAYER_H,
                            e->enemy.x - ENEMY_ATTACK_RANGE,
                            e->enemy.y,
                            PLAYER_W + (ENEMY_ATTACK_RANGE * 2),
                            PLAYER_H))
        {
            now = HAL_GetTick();

            if ((now - e->enemy_last_attack_time) >= ENEMY_ATTACK_COOLDOWN)
            {
                e->enemy_last_attack_time = now;

                if (g->player_hp > ENEMY_ATTACK_DAMAGE)
                {
                    g->player_hp -= ENEMY_ATTACK_DAMAGE;
                }
                else
                {
                    g->player_hp = 0;
                }

                if (g->player_hp == 0)
                {
                    if (g->lives > 0)
                    {
                        g->lives--;
                        g->player_hp = PLAYER_MAX_HP;

                        // Knock the player back to the start.
                        g->player.x = 18;
                        g->player.y = 170;
                        g->player_vy = 0;
                    }
                    else
                    {
                        g->mode = STATE_LOSE;
                    }
                }
            }
        }
    }
}

/**
 * @brief Deals damage to enemy if in range and button is pressed.
 */
static void HandleAttack(State_t *g, uint8_t attack_pressed)
{
    if (attack_pressed == 0u)
    {
        return;
    }

    for (uint8_t i = 0; i < g->enemy_count; i++)
    {
        Enemy_t *e = &g->enemies[i];

        if (e->enemy_alive == 0u)
        {
            continue;
        }

        // Simple attack box around the player.
        if (RectangleOverlap(g->player.x - 6, g->player.y - 4, PLAYER_W + 12, PLAYER_H + 8,
                             e->enemy.x, e->enemy.y, PLAYER_W, PLAYER_H))
        {
            if (e->enemy_hp > 20)
            {
                e->enemy_hp -= 20;
            }
            else
            {
                e->enemy_hp = 0;
            }

            if (e->enemy_hp == 0)
            {
                e->enemy_alive = 0;
                e->enemy_death_time = HAL_GetTick();

                // Drop a coin where the enemy was defeated.
                g->coin_active = 1;
                g->coin_x = e->enemy.x + 3;
                g->coin_y = e->enemy.y - 6;

                // Feedback.
                PWM_SetDuty(&pwm_cfg, 80);
            }

            return;
        }
    }
}

/**
 * @brief Check coins
 */
static void CheckCollectables(State_t *g)
{
    if (g->coin_active == 0)
    {
        return;
    }

    if (RectangleOverlap(g->player.x, g->player.y, PLAYER_W, PLAYER_H,
                         g->coin_x, g->coin_y, 6, 6))
    {
        g->coin_active = 0;
        g->score++;

        // LED at full brightness once max coins collected 
        PWM_SetDuty(&pwm_cfg, 20 + (g->score * 8));
    }
}

/**
 * @brief Check whether the player has collected enough coins to win
 */
static uint8_t CheckWin(const State_t *g)
{
    if (g->score >= REQUIRED_COINS)
    {
        return 1;
    }

    return 0;
}

/**
 * @brief Draw a single platform.
 */
static void DrawPlatform(const Coordinate_t *r)
{
    LCD_Draw_Rect((uint16_t)r->x, (uint16_t)r->y, (uint16_t)r->w, (uint16_t)r->h,
                  COLOUR_PLATFORM, 1);
}

/**
 * @brief Draw healthbar above characters
 */
static void DrawHealthBar(int16_t x, int16_t y, int16_t w, int16_t h,
                          int16_t hp, int16_t max_hp)
{
    if (hp < 0) hp = 0;
    if (hp > max_hp) hp = max_hp;

    int16_t fill_w = (w * hp) / max_hp;

    LCD_Draw_Rect((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h, 1, 0);
    LCD_Draw_Rect((uint16_t)x, (uint16_t)y, (uint16_t)fill_w, (uint16_t)h, 4, 1);
}

/**
 * @brief Draw pause menu.
 */
static void DrawPauseMenu(uint8_t selected_option)
{
    LCD_Fill_Buffer(0);

    LCD_printString("PAUSED", 74, 55, 1, 3);

    if (selected_option == 0)
    {
        LCD_printString("> RESUME", 60, 110, 1, 2);
        LCD_printString("  EXIT",   60, 140, 1, 2);
    }
    else
    {
        LCD_printString("  RESUME", 60, 110, 1, 2);
        LCD_printString("> EXIT",   60, 140, 1, 2);
    }

    LCD_printString("UP/DOWN SELECT", 36, 185, 1, 1);
    LCD_printString("PC3 CONFIRM",    48, 202, 1, 1);

    LCD_Refresh(&cfg0);
}

/**
 * @brief Pause menu shown when button is pressed
 */
static uint8_t ShowPauseMenu(State_t *g)
{
    uint8_t selected_option = 0;
    uint8_t prev_confirm = 1;
    uint8_t prev_pause = 1;
    Direction prev_direction = CENTRE;
    uint32_t pause_start = HAL_GetTick();

    DrawPauseMenu(selected_option);

    while (1)
    {
        Joystick_Read(&joystick_cfg, &joystick_data);
        UserInput input = Joystick_GetInput(&joystick_data);

        uint8_t confirm_now = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);
        uint8_t pause_now = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2);

        // Up/down selects resume or exit.
        if ((input.direction != prev_direction) &&
            ((input.direction == N) || (input.direction == S)))
        {
            if (selected_option == 0)
            {
                selected_option = 1;
            }
            else
            {
                selected_option = 0;
            }

            DrawPauseMenu(selected_option);
        }

        // PC2 closes the pause menu and resumes the game.
        if ((pause_now == GPIO_PIN_RESET) && (prev_pause == GPIO_PIN_SET))
        {
            uint32_t pause_time = HAL_GetTick() - pause_start;

            g->next_enemy_spawn_time += pause_time;

            for (uint8_t i = 0; i < g->enemy_count; i++)
            {
                if (g->enemies[i].enemy_alive == 0)
                {
                    g->enemies[i].enemy_death_time += pause_time;
                }
            }

            HAL_Delay(150);
            return 0;
        }

        // PC3 confirms the selected option.
        if ((confirm_now == GPIO_PIN_RESET) && (prev_confirm == GPIO_PIN_SET))
        {
            if (selected_option == 1)
            {
                HAL_Delay(150);
                return 1;
            }
            else
            {
                uint32_t pause_time = HAL_GetTick() - pause_start;

                g->next_enemy_spawn_time += pause_time;

                for (uint8_t i = 0; i < g->enemy_count; i++)
                {
                    if (g->enemies[i].enemy_alive == 0)
                    {
                        g->enemies[i].enemy_death_time += pause_time;
                    }
                }

                HAL_Delay(150);
                return 0;
            }
        }

        prev_confirm = confirm_now;
        prev_pause = pause_now;
        prev_direction = input.direction;

        HAL_Delay(30);
    }
}

/**
 * @brief Draw the whole screen then refresh frame in main loop
 */
static void DrawWorld(const State_t *g)
{
    char buffer[32];

    // Clear previous frame.
    LCD_Fill_Buffer(0);

    // Sky
    LCD_Draw_Rect(0, 0, SCREEN_W, 120, 5, 1);
    LCD_Draw_Rect(0, 120, SCREEN_W, 70, 2, 1);

    // Sun
    LCD_Draw_Rect(180, 18, 26, 26, 6, 1);
    LCD_Draw_Rect(186, 24, 14, 14, 2, 1);

    // Ground
    LCD_Draw_Rect(0, 190, SCREEN_W, 10, 3, 1);

    // Title / score line.
    snprintf(buffer, sizeof(buffer), "SCORE %u   LIVES %u", g->score, g->lives);
    LCD_printString(buffer, 8, 2, COLOUR_TEXT, 1);

    // Player healthbar.
    DrawHealthBar(8, 14, 80, 6, g->player_hp, PLAYER_MAX_HP);

    // Control 
    LCD_printString("Left/Right=move Up=jump PC3=attack", 6, 220, COLOUR_TEXT, 1);

    // Draw platforms.
    // ADD more platforms in the array above and they will automatically render.
    for (uint8_t i = 1; i < NUM_PLATFORMS; i++)
    {
        DrawPlatform(&g_level_platforms[i]);
    }

    // Draw collectible coin.
    if (g->coin_active)
    {
        LCD_Draw_Rect((uint16_t)g->coin_x, (uint16_t)g->coin_y,
                      6, 6, COLOUR_COIN, 1);
    }

    // Draw enemy.
    for (uint8_t i = 0; i < g->enemy_count; i++)
    {
        const Enemy_t *e = &g->enemies[i];

        if (e->enemy_alive)
        {
            Character_Draw(&e->enemy);

            // enemy healthbar above enemy.
            DrawHealthBar(e->enemy.x, e->enemy.y - 8,
                          24, 4,
                          e->enemy_hp, ENEMY_MAX_HP);
        }
    }

    // Draw player last so the player sits on top of the scenery.
    Character_Draw(&g->player);
}

/**
 * @brief Intro screen shown at the start of the game.
 * @note Good place to add a level title, story text, or controls.
 */
static void ShowIntroScreen(void)
{
    LCD_Fill_Buffer(0);
    LCD_printString("STEALTH RUN", 55, 60, 1, 3);
    LCD_printString("ARCADE PLATFORMER", 22, 95, 1, 2);
    LCD_printString("W/E = MOVE", 65, 140, 1, 2);
    LCD_printString("N = JUMP", 74, 162, 1, 2);
    LCD_printString("PC3 = ATTACK", 61, 184, 1, 2);
    LCD_Refresh(&cfg0);

    HAL_Delay(1500);
}

/**
 * @brief Win / lose screen.
 * @note Add sound effects or animation here later.
 */
static void ShowEndScreen(uint8_t win)
{
    LCD_Fill_Buffer(0);

    if (win)
    {
        LCD_printString("MISSION CLEAR", 30, 75, 1, 3);
    }
    else
    {
        LCD_printString("MISSION FAILED", 25, 75, 1, 3);
    }

    LCD_printString("PRESS PC3", 60, 125, 1, 2);
    LCD_printString("TO RETURN", 62, 150, 1, 2);
    LCD_Refresh(&cfg0);
}

/**
 * @brief Initialise game to base state.
 */
static void Level_init(State_t *g)
{
    // Seed random at start of game so enemy spawn is unpredictable
    srand(HAL_GetTick());

    // Reset LED brightness
    PWM_SetDuty(&pwm_cfg, 20);

    g->mode = STATE_INTRO;

    // Player starts near the left side of the screen.
    g->player.x = 18;
    g->player.y = 170;
    g->player.type = CHAR_PLAYER;
    g->player.state = CHAR_IDLE;
    g->player.animation_frame = 0;
    g->player.frame_counter = 0;
    g->player_vy = 0;
    g->grounded  = 0;
    g->player_hp = PLAYER_MAX_HP;

    // Score / lives.
    g->lives = START_LIVES;
    g->score = 0;

    // No enemies except the first enemy at the start.
    for (uint8_t i = 0; i < MAX_ENEMIES; i++)
    {
        g->enemies[i].enemy_alive = 0;
        g->enemies[i].enemy_hp = 0;
        g->enemies[i].enemy_death_time = 0;
        g->enemies[i].enemy_dir = 1;
        g->enemies[i].enemy_last_attack_time = 0;
    }

    // Spawn first enemy.
    g->enemy_count = 1;
    Spawnenemy(g, 0);
    g->next_enemy_spawn_time = HAL_GetTick() + ENEMY_RESPAWN_TIME;

    // No coin at the start.
    g->coin_active = 0;
    g->coin_x = 0;
    g->coin_y = 0;
}

/**
 * @brief Game 1 main loop.
 * @return MENU_STATE_HOME when the game ends or exits.
 *
 * Architecture:
 *   1. Reset local game state
 *   2. Show intro screen
 *   3. Run the 33 fps loop
 *   4. Update input, physics, AI, collectibles
 *   5. Draw the world once per frame
 *   6. Return to the menu on win / lose
 */
MenuState Game1_Run(void)
{
    State_t game;
    uint32_t frame_start;
    uint8_t prev_attack = 0;
    uint8_t prev_pause = 0;

    Level_init(&game);
    ShowIntroScreen();

    // Start in PLAYING after the intro.
    // This is where you could add a press-any-button-to-start screen later.
    game.mode = STATE_PLAYING;

    while (1)
    {
        frame_start = HAL_GetTick();

        // Read joystick input.
        Joystick_Read(&joystick_cfg, &joystick_data);
        UserInput input = Joystick_GetInput(&joystick_data);

        // Read attack button.
        uint8_t attack_now = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET);
        uint8_t attack_pressed = (attack_now != 0) && (prev_attack == 0);
        prev_attack = attack_now;

        // Read pause button
        uint8_t pause_now = (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_RESET);
        uint8_t pause_pressed = (pause_now != 0) && (prev_pause == 0);
        prev_pause = pause_now;

        if (game.mode == STATE_PLAYING)
        {
            // Check for pause
            if (pause_pressed)
            {
                if (ShowPauseMenu(&game))
                {
                    return MENU_STATE_HOME;
                }
            }
            // 1) Input
            HandleInput(&game, input);

            // 2) Attack
            HandleAttack(&game, attack_pressed);

            // 3) Physics
            ApplyPhysics(&game);

            // 4) AI
            UpdateEnemy(&game);

            // 5) Collectibles / score
            CheckCollectables(&game);

            // 6) Win condition
            if (CheckWin(&game))
            {
                game.mode = STATE_WIN;
            }
        }

        // Update player walking animation
        if (game.player.state == CHAR_WALKING)
        {
            Update_Animation(&game.player);
        }

        // Update guard walking animation
        for (uint8_t i = 0; i < game.enemy_count; i++)
        {
            if (game.enemies[i].enemy_alive)
            {
                Update_Animation(&game.enemies[i].enemy);
            }
        }

        // Draw the current frame.
        DrawWorld(&game);
        LCD_Refresh(&cfg0);

        // End state screens.
        if (game.mode == STATE_WIN)
        {
            ShowEndScreen(1);

            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) != GPIO_PIN_RESET)
            {
                HAL_Delay(10);
            }
            HAL_Delay(150);
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET)
            {
                HAL_Delay(10);
            }
            return MENU_STATE_HOME;
        }
        else if (game.mode == STATE_LOSE)
        {
            ShowEndScreen(0);

            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) != GPIO_PIN_RESET)
            {
                HAL_Delay(10);
            }
            HAL_Delay(150);
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == GPIO_PIN_RESET)
            {
                HAL_Delay(10);
            }
            return MENU_STATE_HOME;
        }

        // Frame timing: checks elapsed time to keep set fps.
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < FRAME_TICKS)
        {
            HAL_Delay(FRAME_TICKS - frame_time);
        }
    }
}