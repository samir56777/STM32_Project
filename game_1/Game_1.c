#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Joystick.h"
#include "PWM.h"
#include "Buzzer.h"
#include "MazeEscape.h"
#include "stm32l4xx_hal.h"

#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
extern PWM_cfg_t pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;

#define GAME1_FPS 60
#define GAME1_FRAME_TIME_MS (1000 / GAME1_FPS)

static MazeEscape_t maze_engine;

static void Game1_ShowIntro(void)
{
    LCD_Fill_Buffer(5);
    LCD_printString("MAZE ESCAPE", 54, 95, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(1500);

    LCD_Fill_Buffer(5);
    LCD_printString("Sneak past guards", 8, 70, 1, 2);
    LCD_printString("Use joystick", 35, 105, 1, 2);
    LCD_printString("BT3 = menu", 45, 140, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(1500);
}

MenuState Game1_Run(void)
{
    MazeEscape_Init(&maze_engine, 20, 20, 8, 8);

    PWM_SetDuty(&pwm_cfg, 0);
    Game1_ShowIntro();

    uint32_t last_tick = HAL_GetTick();

    while (1)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_tick) < GAME1_FRAME_TIME_MS) {
            continue;
        }

        last_tick = now;

        Input_Read();

        if (current_input.btn3_pressed) {
            buzzer_off(&buzzer_cfg);
            PWM_SetDuty(&pwm_cfg, 50);
            LCD_Fill_Buffer(0);
            LCD_Refresh(&cfg0);
            return MENU_STATE_HOME;
        }

        Joystick_Read(&joystick_cfg, &joystick_data);
        UserInput input = Joystick_GetInput(&joystick_data);

        MazeEscape_Update(&maze_engine, input);

        LCD_Fill_Buffer(0);
        MazeEscape_Draw(&maze_engine);

        char level_str[32];
        sprintf(level_str, "Level: %d", maze_engine.level);
        LCD_printString(level_str, 10, 10, 1, 1);

        LCD_printString("EXIT", 180, 220, 1, 2);

        LCD_Refresh(&cfg0);
    }
}