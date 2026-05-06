#include "Game_2.h"

#include "main.h"
#include "LCD.h"
#include "Joystick.h"
#include "Buzzer.h"
#include "InputHandler.h"
#include "Menu.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern Joystick_t joystick_data;
extern Buzzer_cfg_t buzzer_cfg;
extern ST7789V2_cfg_t cfg0;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
extern Buzzer_cfg_t buzzer_cfg;

#define COL_BLACK   0
#define COL_WHITE   1
#define COL_RED     2
#define COL_GREEN   3
#define COL_BLUE    4
#define COL_ORANGE  5
#define COL_YELLOW  6
#define COL_PINK    7
#define COL_PURPLE  8
#define COL_NAVY    9
#define COL_GOLD    10
#define COL_VIOLET  11
#define COL_BROWN   12
#define COL_GREY    13
#define COL_CYAN    14
#define COL_MAGENTA 15

#define SHAPE_OUTLINE 0
#define SHAPE_FILLED  1

#define MAX_EXPLOSIONS 6
#define EXPLOSION_TIME 8

#define SOUND_NONE     0
#define SOUND_SHOOT    1
#define SOUND_HIT      2
#define SOUND_DAMAGE   3
#define SOUND_LEVELUP  4

typedef struct
{
    int x;
    int y;
    int timer;
    bool active;
} Explosion;

static Game_2 game;
static PlayerState player;
static Bullet bullets[MAX_BULLETS];
static Enemy enemies[MAX_ENEMIES];
static Star stars[STAR_COUNT];
static Explosion explosions[MAX_EXPLOSIONS];

static uint32_t score = 0;
static uint32_t level = 1;
static uint32_t previousLevel = 1;
static uint32_t spawnDelay = 35;
static uint32_t spawnCounter = 0;
static uint32_t shootCooldown = 0;
static uint32_t frameCounter = 0;
static uint32_t levelMessageTimer = 0;
static bool firePressedLast = false;

static uint32_t soundTimer = 0;
static uint32_t soundStep = 0;
static uint8_t soundMode = SOUND_NONE;

static bool rectOverlap(int ax, int ay, int aw, int ah,
                        int bx, int by, int bw, int bh)
{
    return (ax < bx + bw) &&
           (ax + aw > bx) &&
           (ay < by + bh) &&
           (ay + ah > by);
}

static bool isFirePressed(void)
{
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET;
}

// buzzer sound system 
static void startSound(uint8_t mode)
{
    soundMode = mode;
    soundStep = 0;
    soundTimer = 1;
}

static void updateSound(void)
{
    if (soundMode == SOUND_NONE)
    {
        buzzer_off(&buzzer_cfg);
        return;
    }

    if (soundTimer > 0)
    {
        soundTimer--;
        return;
    }

    switch (soundMode)
    {
        case SOUND_SHOOT:
            if (soundStep == 0)
            {
                buzzer_tone(&buzzer_cfg, 1800, 35);
                soundTimer = 2;
                soundStep++;
            }
            else if (soundStep == 1)
            {
                buzzer_tone(&buzzer_cfg, 2400, 30);
                soundTimer = 2;
                soundStep++;
            }
            else
            {
                buzzer_off(&buzzer_cfg);
                soundMode = SOUND_NONE;
            }
            break;

        case SOUND_HIT:
            if (soundStep == 0)
            {
                buzzer_tone(&buzzer_cfg, 900, 45);
                soundTimer = 2;
                soundStep++;
            }
            else if (soundStep == 1)
            {
                buzzer_tone(&buzzer_cfg, 500, 45);
                soundTimer = 3;
                soundStep++;
            }
            else
            {
                buzzer_off(&buzzer_cfg);
                soundMode = SOUND_NONE;
            }
            break;

        case SOUND_DAMAGE:
            if (soundStep == 0)
            {
                buzzer_tone(&buzzer_cfg, 220, 50);
                soundTimer = 5;
                soundStep++;
            }
            else
            {
                buzzer_off(&buzzer_cfg);
                soundMode = SOUND_NONE;
            }
            break;

        case SOUND_LEVELUP:
            if (soundStep == 0)
            {
                buzzer_tone(&buzzer_cfg, 800, 40);
                soundTimer = 3;
                soundStep++;
            }
            else if (soundStep == 1)
            {
                buzzer_tone(&buzzer_cfg, 1200, 40);
                soundTimer = 3;
                soundStep++;
            }
            else if (soundStep == 2)
            {
                buzzer_tone(&buzzer_cfg, 1600, 40);
                soundTimer = 4;
                soundStep++;
            }
            else
            {
                buzzer_off(&buzzer_cfg);
                soundMode = SOUND_NONE;
            }
            break;

        default:
            buzzer_off(&buzzer_cfg);
            soundMode = SOUND_NONE;
            break;
    }
}

static void playShootSound(void)
{
    startSound(SOUND_SHOOT);
}

static void playHitSound(void)
{
    startSound(SOUND_HIT);
}

static void playDamageSound(void)
{
    startSound(SOUND_DAMAGE);
}

static void playLevelUpSound(void)
{
    startSound(SOUND_LEVELUP);
}

// game helpers
static void updateDifficulty(void)
{
    uint32_t newLevel = (score / 10) + 1;

    if (newLevel > 6)
    {
        newLevel = 6;
    }

    if (newLevel > previousLevel)
    {
        levelMessageTimer = 40;
        playLevelUpSound();
    }

    previousLevel = newLevel;
    level = newLevel;

    spawnDelay = 35 - ((level - 1) * 5);

    if (spawnDelay < 12)
    {
        spawnDelay = 12;
    }
}

static void resetBullets(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].active = false;
    }
}

static void resetEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = false;
        enemies[i].speed = 1;
        enemies[i].color = COL_GREEN;
    }
}

static void resetExplosions(void)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        explosions[i].active = false;
        explosions[i].timer = 0;
    }
}

static void initStars(void)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].x = rand() % SCREEN_WIDTH;
        stars[i].y = rand() % SCREEN_HEIGHT;
        stars[i].speed = 1 + (rand() % 2);
        stars[i].color = COL_WHITE;
    }
}

static void spawnExplosion(int x, int y)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!explosions[i].active)
        {
            explosions[i].active = true;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].timer = EXPLOSION_TIME;
            return;
        }
    }
}

static void spawnEnemy(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            enemies[i].active = true;
            enemies[i].x = rand() % (SCREEN_WIDTH - ENEMY_WIDTH);
            enemies[i].y = 20;
            enemies[i].speed = ENEMY_SPEED_MIN + (rand() % (ENEMY_SPEED_MAX + 1)) + (level - 1);
            enemies[i].color = COL_GREEN;
            break;
        }
    }
}

static void fireBullet(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!bullets[i].active)
        {
            bullets[i].active = true;
            bullets[i].x = player.x + (PLAYER_WIDTH / 2) - (BULLET_WIDTH / 2);
            bullets[i].y = player.y - BULLET_HEIGHT;
            playShootSound();
            return;
        }
    }
}

//drawings

static void drawStars(void)
{
    for (int i = 0; i < STAR_COUNT; i++)
    {
        LCD_Set_Pixel((uint16_t)stars[i].x, (uint16_t)stars[i].y, stars[i].color);
    }
}

static void drawPlayer(void)
{
    LCD_Draw_Rect(player.x + 10, player.y + 8, 20, 28, COL_WHITE, SHAPE_FILLED);

    LCD_Draw_Rect(player.x + 15, player.y + 2, 10, 8, COL_RED, SHAPE_FILLED);

    LCD_Draw_Rect(player.x + 14, player.y + 16, 12, 10, COL_BLUE, SHAPE_FILLED);
    LCD_Draw_Rect(player.x + 17, player.y + 18, 6, 5, COL_CYAN, SHAPE_FILLED);

    LCD_Draw_Rect(player.x + 4, player.y + 28, 8, 12, COL_RED, SHAPE_FILLED);
    LCD_Draw_Rect(player.x + 28, player.y + 28, 8, 12, COL_RED, SHAPE_FILLED);

    if ((frameCounter / 4) % 2 == 0)
    {
        LCD_Draw_Rect(player.x + 16, player.y + 36, 8, 8, COL_ORANGE, SHAPE_FILLED);
        LCD_Draw_Rect(player.x + 18, player.y + 42, 4, 8, COL_YELLOW, SHAPE_FILLED);
    }
    else
    {
        LCD_Draw_Rect(player.x + 15, player.y + 36, 10, 10, COL_YELLOW, SHAPE_FILLED);
        LCD_Draw_Rect(player.x + 18, player.y + 42, 4, 5, COL_ORANGE, SHAPE_FILLED);
    }
}

static void drawBullets(void)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            LCD_Draw_Rect(bullets[i].x, bullets[i].y,
                          BULLET_WIDTH, BULLET_HEIGHT,
                          COL_YELLOW, SHAPE_FILLED);
        }
    }
}

static void drawAlien(int x, int y)
{
    LCD_Draw_Rect(x + 3, y + 6, 18, 16, COL_GREEN, SHAPE_FILLED);

    LCD_Draw_Rect(x + 6, y + 2, 3, 6, COL_GREEN, SHAPE_FILLED);
    LCD_Draw_Rect(x + 15, y + 2, 3, 6, COL_GREEN, SHAPE_FILLED);
    LCD_Draw_Rect(x + 5, y, 5, 4, COL_YELLOW, SHAPE_FILLED);
    LCD_Draw_Rect(x + 14, y, 5, 4, COL_YELLOW, SHAPE_FILLED);

    LCD_Draw_Rect(x + 7, y + 11, 4, 5, COL_BLUE, SHAPE_FILLED);
    LCD_Draw_Rect(x + 14, y + 11, 4, 5, COL_BLUE, SHAPE_FILLED);

    LCD_Draw_Rect(x + 10, y + 19, 6, 2, COL_BLUE, SHAPE_FILLED);

    LCD_Draw_Rect(x + 5, y + 22, 4, 4, COL_GREEN, SHAPE_FILLED);
    LCD_Draw_Rect(x + 15, y + 22, 4, 4, COL_GREEN, SHAPE_FILLED);
}

static void drawEnemies(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            drawAlien(enemies[i].x, enemies[i].y);
        }
    }
}

static void drawExplosions(void)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (explosions[i].active)
        {
            int x = explosions[i].x;
            int y = explosions[i].y;
            int size = EXPLOSION_TIME - explosions[i].timer + 3;

            LCD_Draw_Rect(x - size, y - size, size * 2, size * 2, COL_YELLOW, SHAPE_OUTLINE);
            LCD_Draw_Rect(x - 2, y - 2, 4, 4, COL_ORANGE, SHAPE_FILLED);
            LCD_Draw_Rect(x - size - 2, y, 4, 4, COL_RED, SHAPE_FILLED);
            LCD_Draw_Rect(x + size, y, 4, 4, COL_RED, SHAPE_FILLED);
            LCD_Draw_Rect(x, y - size - 2, 4, 4, COL_YELLOW, SHAPE_FILLED);
            LCD_Draw_Rect(x, y + size, 4, 4, COL_ORANGE, SHAPE_FILLED);
        }
    }
}

static void drawHUD(void)
{
    char text[32];

    sprintf(text, "S:%lu", score);
    LCD_printString(text, 4, 4, COL_WHITE, 2);

    sprintf(text, "Lv:%lu", level);
    LCD_printString(text, 85, 4, COL_YELLOW, 2);

    sprintf(text, "L:%d", player.lives);
    LCD_printString(text, 160, 4, COL_WHITE, 2);
}

//functions

void Game_2_Init(void)
{
    game.state = GAME_STATE_START;

    player.x = 95;
    player.y = PLAYER_Y;
    player.lives = 3;

    score = 0;
    level = 1;
    previousLevel = 1;
    spawnDelay = 35;
    spawnCounter = 0;
    shootCooldown = 0;
    frameCounter = 0;
    levelMessageTimer = 0;
    firePressedLast = false;

    soundMode = SOUND_NONE;
    soundStep = 0;
    soundTimer = 0;
    buzzer_off(&buzzer_cfg);

    resetBullets();
    resetEnemies();
    resetExplosions();
    initStars();
}

void Game_2_Reset(void)
{
    Game_2_Init();
    game.state = GAME_STATE_PLAYING;
}

void Game_2_Update(void)
{
    frameCounter++;

    updateSound();

    bool firePressed = isFirePressed();

    if (game.state == GAME_STATE_START)
    {
        if (firePressed && !firePressedLast)
        {
            Game_2_Reset();
        }

        firePressedLast = firePressed;
        return;
    }

    if (game.state == GAME_STATE_GAME_OVER)
    {
        if (firePressed && !firePressedLast)
        {
            Game_2_Init();
        }

        firePressedLast = firePressed;
        return;
    }

    updateDifficulty();

    if (levelMessageTimer > 0)
    {
        levelMessageTimer--;
    }

    if (joystick_data.coord_mapped.x < -0.20f)
    {
        player.x -= PLAYER_SPEED;
    }
    else if (joystick_data.coord_mapped.x > 0.20f)
    {
        player.x += PLAYER_SPEED;
    }

    if (player.x < 0)
    {
        player.x = 0;
    }

    if (player.x > SCREEN_WIDTH - PLAYER_WIDTH)
    {
        player.x = SCREEN_WIDTH - PLAYER_WIDTH;
    }

    if (shootCooldown > 0)
    {
        shootCooldown--;
    }

    if (firePressed && !firePressedLast && shootCooldown == 0)
    {
        fireBullet();
        shootCooldown = 8;
    }

    firePressedLast = firePressed;

    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            bullets[i].y -= BULLET_SPEED;

            if (bullets[i].y < 0)
            {
                bullets[i].active = false;
            }
        }
    }

    spawnCounter++;

    if (spawnCounter >= spawnDelay)
    {
        spawnCounter = 0;
        spawnEnemy();
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            enemies[i].y += enemies[i].speed;

            if (enemies[i].y > SCREEN_HEIGHT)
            {
                enemies[i].active = false;
                player.lives--;
                playDamageSound();
            }
        }
    }

    for (int i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (explosions[i].active)
        {
            explosions[i].timer--;

            if (explosions[i].timer <= 0)
            {
                explosions[i].active = false;
            }
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!enemies[i].active)
        {
            continue;
        }

        for (int j = 0; j < MAX_BULLETS; j++)
        {
            if (!bullets[j].active)
            {
                continue;
            }

            if (rectOverlap(bullets[j].x, bullets[j].y, BULLET_WIDTH, BULLET_HEIGHT,
                            enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT))
            {
                spawnExplosion(enemies[i].x + ENEMY_WIDTH / 2,
                               enemies[i].y + ENEMY_HEIGHT / 2);

                bullets[j].active = false;
                enemies[i].active = false;
                score++;
                playHitSound();
                break;
            }
        }

        if (enemies[i].active &&
            rectOverlap(player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                        enemies[i].x, enemies[i].y, ENEMY_WIDTH, ENEMY_HEIGHT))
        {
            spawnExplosion(enemies[i].x + ENEMY_WIDTH / 2,
                           enemies[i].y + ENEMY_HEIGHT / 2);

            enemies[i].active = false;
            player.lives--;
            playDamageSound();
        }
    }

    if (player.lives <= 0)
    {
        game.state = GAME_STATE_GAME_OVER;
    }

    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].y += stars[i].speed;

        if (stars[i].y >= SCREEN_HEIGHT)
        {
            stars[i].y = 0;
            stars[i].x = rand() % SCREEN_WIDTH;
        }
    }
}

void Game_2_Render(void)
{
    LCD_Fill_Buffer(0);
    drawStars();

    if (game.state == GAME_STATE_START)
    {
        int offset = (frameCounter / 3) % SCREEN_WIDTH;

        drawAlien(offset % 220, 35);
        drawAlien((SCREEN_WIDTH - offset + 60) % 220, 75);

        LCD_Draw_Rect(30 + (offset % 80), 205, 20, 8, COL_WHITE, SHAPE_FILLED);
        LCD_Draw_Rect(38 + (offset % 80), 198, 6, 8, COL_RED, SHAPE_FILLED);
        LCD_Draw_Rect(36 + (offset % 80), 207, 6, 5, COL_BLUE, SHAPE_FILLED);
        LCD_Draw_Rect(25 + (offset % 80), 213, 6, 5, COL_RED, SHAPE_FILLED);
        LCD_Draw_Rect(50 + (offset % 80), 213, 6, 5, COL_RED, SHAPE_FILLED);
        LCD_Draw_Rect(38 + (offset % 80), 214, 6, 6, COL_ORANGE, SHAPE_FILLED);

        LCD_printString("GALAXY", 55, 105, COL_CYAN, 3);
        LCD_printString("DEFENDER", 35, 140, COL_MAGENTA, 3);

        if ((frameCounter / 20) % 2 == 0)
        {
            LCD_printString("PRESS JOYSTICK", 32, 180, COL_WHITE, 2);
            LCD_printString("TO START", 68, 205, COL_YELLOW, 2);
        }

        return;
    }

    if (game.state == GAME_STATE_GAME_OVER)
    {
        char text[32];

        LCD_printString("GAME OVER", 45, 90, COL_RED, 3);

        sprintf(text, "Score:%lu", score);
        LCD_printString(text, 70, 140, COL_WHITE, 2);

        LCD_printString("PRESS TO", 65, 190, COL_YELLOW, 2);
        LCD_printString("RESTART", 70, 220, COL_YELLOW, 2);
        return;
    }

    drawPlayer();
    drawBullets();
    drawEnemies();
    drawExplosions();

    if (levelMessageTimer > 0)
    {
        LCD_printString("LEVEL UP!", 55, 105, COL_YELLOW, 3);
    }

    drawHUD();
}

MenuState Game2_Run(void)
{
    Game_2_Init();

    while (1)
    {
        uint32_t frame_start = HAL_GetTick();

        Input_Read();

        if (current_input.btn3_pressed)
        {
            buzzer_off(&buzzer_cfg);
            LCD_Fill_Buffer(0);
            LCD_Refresh(&cfg0);
            return MENU_STATE_HOME;
        }

        Joystick_Read(&joystick_cfg, &joystick_data);

        Game_2_Update();
        Game_2_Render();

        LCD_Refresh(&cfg0);

        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < 33)
        {
            HAL_Delay(33 - frame_time);
        }
    }
}
