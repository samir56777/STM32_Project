#include "RunnerEngine.h"// included the header file

//included all the libraries used 
#include "LCD.h" 
#include "main.h"
#include "stm32l4xx_hal.h"

#include "rng.h"
#include <stdint.h>

// values defined here
#define SCREEN_WIDTH        240
#define SCREEN_HEIGHT       240

#define GROUND_LINE_Y       200
#define PLAYER_MAX_X        (SCREEN_WIDTH / 2)

#define PLAYER_WIDTH        12
#define PLAYER_HEIGHT       22
#define PLAYER_GROUND_Y     (GROUND_LINE_Y - PLAYER_HEIGHT)

#define PLAYER_STEP         5

#define OBSTACLE_WIDTH      14
#define OBSTACLE_HEIGHT     18
#define OBSTACLE_START_X    220
#define OBSTACLE_Y          (GROUND_LINE_Y - OBSTACLE_HEIGHT)
#define OBSTACLE_SPEED      3
#define OBSTACLE_MIN_GAP 28
#define OBSTACLE_MAX_GAP 38

#define BULL_WIDTH          40
#define BULL_HEIGHT         32
#define BULL_START_X        0
#define BULL_Y              (GROUND_LINE_Y - BULL_HEIGHT)

#define JUMP_UP_TIME        500
#define JUMP_TOTAL_TIME     1000
#define JUMP_DIVISOR        5

#define HOUSE_COUNT 3
#define HOUSE_Y 145
#define HOUSE_WIDTH 28
#define HOUSE_HEIGHT 28

static int16_t houseX[HOUSE_COUNT] = {30, 120, 210};




static uint8_t bullCounter = 0;

static void RunnerEngine_UpdateRunAnimation(RunnerEngine_t* engine) {
    if (engine->isRunning) {// check if player is running
         // HAL_GetTick gives the current time in milliseconds
         // this checks if 120ms passed since the last animation update
         // so the animation does not move too fast
        if ((HAL_GetTick() - engine->lastRunAnimTick) >= 120) {
            engine->runFrame ^= 1;// if player is moving and the difference between the previous animaiton and the new time is more than 120ms toggle between 1 and o
            engine->lastRunAnimTick = HAL_GetTick();// new values becomes previous value
        }
    } else {
        engine->runFrame = 0; // if player stops moving 
    }
}

static uint8_t RunnerEngine_UpdatePlayer(RunnerEngine_t* engine, UserInput input) {
    uint8_t moveWorld = 0; // 
    engine->isRunning = 0; // player is not running yet

    if (input.direction == E || input.direction == NE || input.direction == SE) {// makes player move right if joystick is E, NE or SE
        engine->isRunning = 1;// player starts running

        if (engine->player.x < PLAYER_MAX_X) { // if player is less than middle of the screen 
            engine->player.x += PLAYER_STEP; // player position keep increasing by player step

            if (engine->player.x > PLAYER_MAX_X) {// if player position is bigger than middle of the screen
                engine->player.x = PLAYER_MAX_X; // make player position same as the position in the middle of the screen
            }
        } else {
            moveWorld = 1;// if none of the conditions above happen then world moves
        }
    }
    else if (input.direction == W || input.direction == NW || input.direction == SW) {// this one makes player move if the joystick points to W ,NW,SW
        engine->isRunning = 1;
        engine->player.x -= PLAYER_STEP;// player goes back by PLAYER_STEP

        if (engine->player.x < 0) { // does not allow player position to be less than 0
            engine->player.x = 0;
        }
    }

    return moveWorld;// return 0
}

static void RunnerEngine_UpdateJump(RunnerEngine_t* engine, uint8_t jumpPressed) {
    if (jumpPressed && !engine->isJumping) {
        engine->isJumping = 1;
        engine->jumpStartTick = HAL_GetTick();
    }

    if (engine->isJumping) {
        uint32_t t = HAL_GetTick() - engine->jumpStartTick;

        if (t < JUMP_UP_TIME) {
            engine->player.y = PLAYER_GROUND_Y - (t / JUMP_DIVISOR);
        }
        else if (t < JUMP_TOTAL_TIME) {
            engine->player.y = PLAYER_GROUND_Y - ((JUMP_TOTAL_TIME - t) / JUMP_DIVISOR);
        }
        else {
            engine->player.y = PLAYER_GROUND_Y;
            engine->isJumping = 0;
        }
    }
}

// creates the collision box around the player
static AABB RunnerEngine_GetPlayerAABB(RunnerEngine_t* engine) {
    AABB box;// creates a box variable
    box.x = engine->player.x; // box starts at the same x position as the player
    box.y = engine->player.y;  // box starts at the same y position as the player
    box.width = engine->player.width;  // box has the same width as the player
    box.height = engine->player.height;   // box has the same height as the player
    return box;
}

// checks if the player collides with any obstacle
static uint8_t RunnerEngine_CheckObstacleCollision(RunnerEngine_t* engine) {

    AABB playerBox = RunnerEngine_GetPlayerAABB(engine);   // gets the player's collision box

    for (uint8_t i = 0; i < engine->obstacleCount; i++) {  // checks every obstacle currently active
        AABB obstacleBox = Obstacle_GetAABB(&engine->obstacles[i]); // gets the current obstacle collision box

        if (!AABB_Collides(&playerBox, &obstacleBox)) {      // if player is not touching this obstacle, skip to the next one
            continue;
        }

        int16_t playerBottom = engine->player.y + engine->player.height;        // calculates the bottom of the player

        int16_t obstacleTop = engine->obstacles[i].y;   // gets the top position of the obstacle

               // checks if player and obstacle overlap horizontally
        uint8_t horizontalOverlap =
            (engine->player.x < engine->obstacles[i].x + engine->obstacles[i].width) &&
            (engine->player.x + engine->player.width > engine->obstacles[i].x);

        // Land on top of obstacle
        if (horizontalOverlap &&
        playerBottom >= obstacleTop &&
        engine->player.y < obstacleTop &&
        engine->isJumping) {
            engine->player.y = obstacleTop - engine->player.height;  // places player on top of obstacle
            engine->isJumping = 0;     // stops the jump
            return 1; // returns 1 because player is standing on obstacle
        }

  
    if (!engine->isJumping) {    // checks side collision only when not jumping
        if (engine->player.x < engine->obstacles[i].x) {    // checks side collision only when not jumping
            engine->player.x = engine->obstacles[i].x - engine->player.width;    // checks side collision only when not jumping
        } else {
            engine->player.x = engine->obstacles[i].x + engine->obstacles[i].width;   // blocks player on right side
        }
    }

    if (engine->player.x < 0) { // stops player going off left side of screen
        engine->player.x = 0;
    }

    if (engine->player.x > PLAYER_MAX_X) {   // stops player going past middle of screen
        engine->player.x = PLAYER_MAX_X;
    }

    return 0;
   }

    return 0;
}

static void RunnerEngine_CheckBullCollision(RunnerEngine_t* engine) {
    if (!engine->bullActive) {
        return;
    }

    AABB playerBox = RunnerEngine_GetPlayerAABB(engine);
    AABB bullBox = Obstacle_GetAABB(&engine->bull);

    if (AABB_Collides(&playerBox, &bullBox)) {
        engine->gameOver = 1;
    }
}

static void RunnerEngine_CheckObstaclePassed(RunnerEngine_t* engine) {
    uint8_t allOffScreen = 1;

    for (uint8_t i = 0; i < engine->obstacleCount; i++) {

        // Count score once when player passes obstacle
        if (!engine->obstacles[i].passed &&
            (engine->obstacles[i].x + engine->obstacles[i].width) < engine->player.x) {

            engine->score++;
            engine->obstacles[i].passed = 1;
        }

        // Check if obstacle is still visible
        if ((engine->obstacles[i].x + engine->obstacles[i].width) >= 0) {
            allOffScreen = 0;
        }
    }

    // Spawn new set only after all current obstacles leave screen
    if (allOffScreen) {
        RunnerEngine_SpawnObstacles(engine);
    }
}




// gets a random number from the STM32 random number generator
static uint32_t RunnerEngine_GetRandom(void) {
    uint32_t value = 0;// stores random value
 // tries to generate a random number using the STM32 RNG
    if (HAL_RNG_GenerateRandomNumber(&hrng, &value) != HAL_OK) {
        value = HAL_GetTick(); // fallback if RNG fails
    }

    return value; // return the random number

}
// function used to generate a random number between min and max
static uint8_t RunnerEngine_RandomRange(uint8_t min, uint8_t max) {
    uint32_t r = RunnerEngine_GetRandom();// store a random number from the RNG function
    return min + (r % (max - min + 1));// keeps the random number inside the chosen range
}

// function used to create/spawn obstacles
static void RunnerEngine_SpawnObstacles(RunnerEngine_t* engine) {
    // Randomly choose 1, 2, or 3 obstacles
    engine->obstacleCount = RunnerEngine_RandomRange(1, MAX_OBSTACLES);
    
    // starting x position for the first obstacle
    // starts near the right side of the screen
    int16_t x = OBSTACLE_START_X;

      // loop through all obstacles that need to be created
    for (uint8_t i = 0; i < engine->obstacleCount; i++) {
        Obstacle_Init(
            &engine->obstacles[i],// current obstacle in the array
            x,//x position
            OBSTACLE_Y,// y position on the ground
            OBSTACLE_WIDTH, // obstacle width
            OBSTACLE_HEIGHT,// obstacle height
            OBSTACLE_SPEED // movement speed
        );
        engine->obstacles[i].passed = 0;  // obstacle has not been passed by the player yet

        // Random safe gap for next obstacle
        uint8_t gap = RunnerEngine_RandomRange(OBSTACLE_MIN_GAP, OBSTACLE_MAX_GAP);
        x += OBSTACLE_WIDTH + gap;
        // moves next obstacle further right
    }
}


void RunnerEngine_Init(RunnerEngine_t* engine) {
  
    engine->player.x = 20; // start at 20 pixels x axis
    engine->player.y = PLAYER_GROUND_Y; // stays on the cround line created 
    engine->player.width = PLAYER_WIDTH; // sets player width 
    engine->player.height = PLAYER_HEIGHT; // sets player height

    engine->isJumping = 0;
    engine->jumpStartTick = 0;

    engine->isRunning = 0;
    engine->runFrame = 0;
    engine->lastRunAnimTick = 0;

    engine->score = 0;
    engine->gameOver = 0;

    engine->bullActive = 0;
    bullCounter = 0;

   RunnerEngine_SpawnObstacles(engine);

    Obstacle_Init(&engine->bull,
                  BULL_START_X,
                  BULL_Y,
                  BULL_WIDTH,
                  BULL_HEIGHT,
                  1);
}

// updates the main game systems
uint8_t RunnerEngine_Update(RunnerEngine_t* engine, UserInput input, uint8_t jumpPressed) {

    // stops updating if game is over
    if (engine->gameOver) {
        return 0;
    }

    // updates player movement and checks if world should move
    uint8_t moveWorld = RunnerEngine_UpdatePlayer(engine, input);

    // updates jump movement
    RunnerEngine_UpdateJump(engine, jumpPressed);

    // updates running animation
    RunnerEngine_UpdateRunAnimation(engine);

    // activates bull when player reaches middle of screen
    if (engine->player.x >= PLAYER_MAX_X) {
        engine->bullActive = 1;
    }

    // moves world objects if player keeps progressing
    if (moveWorld) {

        // loops through all obstacles
        for (uint8_t i = 0; i < engine->obstacleCount; i++) {

            // moves obstacles faster while player is jumping
            if (engine->isJumping) {
                engine->obstacles[i].x -= engine->obstacles[i].speed + 2;
            } else {

                // normal obstacle movement
                engine->obstacles[i].x -= engine->obstacles[i].speed;
            }
        }

        // loops through all houses
        for (uint8_t i = 0; i < HOUSE_COUNT; i++) {

            // moves house left
            houseX[i] -= OBSTACLE_SPEED;

            // resets house position when it leaves screen
            if (houseX[i] + HOUSE_WIDTH < 0) {
                houseX[i] = SCREEN_WIDTH + (i * 70);
            }
        }
    }

    // checks collision between player and obstacles
    uint8_t standingOnObstacle = RunnerEngine_CheckObstacleCollision(engine);

    // keeps player on ground if not jumping or standing on obstacle
    if (!engine->isJumping && !standingOnObstacle) {
        engine->player.y = PLAYER_GROUND_Y;
    }

    // only updates bull movement if bull is active
    if (engine->bullActive) {

        // if player is progressing normally
        if (moveWorld) {

            // keeps bull at same distance
            bullCounter = 0;
        }

        // if player is slowed or blocked
        else if (engine->isRunning) {

            // increases counter
            bullCounter++;

            // moves bull every 2 update cycles
            if (bullCounter >= 2) {

                // moves bull closer
                engine->bull.x += 5;

                // resets counter
                bullCounter = 0;
            }
        }

        // if player completely stops moving
        else {

            // increases counter
            bullCounter++;

            // moves bull every 2 update cycles
            if (bullCounter >= 2) {

                // moves bull closer
                engine->bull.x += 5;

                // resets counter
                bullCounter = 0;
            }
        }
    }

    // checks if player passed obstacles for score system
    RunnerEngine_CheckObstaclePassed(engine);

    // checks collision between player and bull
    RunnerEngine_CheckBullCollision(engine);

    // returns 0 if game over, otherwise returns 1
    return engine->gameOver ? 0 : 1;
}

void RunnerEngine_Draw(RunnerEngine_t* engine) {
    LCD_Draw_Line(0, GROUND_LINE_Y, SCREEN_WIDTH - 1, GROUND_LINE_Y, 0);

    LCD_Draw_Circle(200,  40,  20, 7,  1);


// loops through all houses
    for (uint8_t i = 0; i < HOUSE_COUNT; i++) {
    int16_t x = houseX[i];  // stores current house x position
    int16_t y = HOUSE_Y;   // stores house y position

    // House body
    LCD_Draw_Rect(x, y + 10, HOUSE_WIDTH, HOUSE_HEIGHT, 8, 1);

    // Roof
    int16_t centerX = x + HOUSE_WIDTH / 2;  // finds middle of house for roof positioning
    int16_t halfWidth = HOUSE_WIDTH / 2;   // stores half the roof width
    int16_t roofHeight = 10;// stores roof height

for (int16_t row = 0; row < roofHeight; row++) { // loops through each roof row

    int16_t spread = (row * halfWidth) / roofHeight;        // increases roof width each row


    int16_t startX = centerX - spread; // calculates left side of roof line
    int16_t endX   = centerX + spread; // calculates right side of roof line

    LCD_Draw_Line(startX, y + row, endX, y + row, 5);  // draws one roof line
}
    // Door
    LCD_Draw_Rect(x + 11, y + 24, 7, 14, 0, 1);

    // Windows
    LCD_Draw_Rect(x + 4, y + 16, 6, 6, 1, 1);
    LCD_Draw_Rect(x + 18, y + 16, 6, 6, 1, 1);
}

// Ground / road area
LCD_Draw_Rect(
    0,
    GROUND_LINE_Y-20,
    SCREEN_WIDTH,
    SCREEN_HEIGHT -( GROUND_LINE_Y-20),
    10,   // ground colour
    1     // filled
);
//stores the player position
 int16_t px = engine->player.x;
int16_t py = engine->player.y;

// Head
LCD_Draw_Rect(px + 3, py, 8, 7, 5, 1);

// Hair 
LCD_Draw_Rect(px + 2, py, 10, 2, 0, 1);
LCD_Draw_Rect(px + 8, py + 2, 5, 2, 0, 1);

// Face visor
LCD_Draw_Rect(px + 7, py + 3, 4, 2, 2, 1);

// shirt
LCD_Draw_Rect(px + 2, py + 8, 10, 8, 3, 1);

// Arm
if (engine->runFrame == 0) {// when runframe is 0 arms are at this position
    LCD_Draw_Line(px + 2, py + 10, px - 2, py + 14, 5);
    LCD_Draw_Line(px + 12, py + 10, px + 15, py + 8, 5);
} else {
    LCD_Draw_Line(px + 2, py + 10, px - 1, py + 8, 5); //when run frame is 1 arms position change
    LCD_Draw_Line(px + 12, py + 10, px + 16, py + 14, 5);
}

// Shorts
LCD_Draw_Rect(px + 3, py + 16, 8, 4, 7, 1);


// loop through all the obstacles currently active in the game
    for (uint8_t i = 0; i < engine->obstacleCount; i++) {
    LCD_Draw_Rect(
        engine->obstacles[i].x, //position in x plane
        engine->obstacles[i].y,//position in y plane
        engine->obstacles[i].width, //width of the obstacle
        engine->obstacles[i].height, //height of obstacle
        6, //collour
        1 //filled obstacle
    );
}
    
    
    //legs
    if (engine->runFrame == 0) {//runframe at 0 legs in one direction
        //leg1
        LCD_Draw_Line(engine->player.x + 2,
                      engine->player.y + engine->player.height,
                      engine->player.x,
                      engine->player.y + engine->player.height + 1,
                      0);
        //leg2
        LCD_Draw_Line(engine->player.x + engine->player.width - 3,
                      engine->player.y + engine->player.height,
                      engine->player.x + engine->player.width,
                      engine->player.y + engine->player.height + 1,
                      0);
    } else { // runframe is one legs in another direction
        //leg1
        LCD_Draw_Line(engine->player.x + 2,
                      engine->player.y + engine->player.height,
                      engine->player.x + 4,
                      engine->player.y + engine->player.height + 1,
                      0);
        //leg2
        LCD_Draw_Line(engine->player.x + engine->player.width - 3,
                      engine->player.y + engine->player.height,
                      engine->player.x + engine->player.width - 5,
                      engine->player.y + engine->player.height + 1,
                      0);
    }




 
    
  if (engine->bullActive) {
    int16_t x = engine->bull.x;
    int16_t y = engine->bull.y;

    // Bull body
    LCD_Draw_Rect(x, y + 10, 32, 14, 5, 1);

    // Bull head
    LCD_Draw_Rect(x + 26, y + 6, 14, 12, 5, 1);

    // Horns
    LCD_Draw_Line(x + 28, y + 6, x + 22, y + 1, 1);
    LCD_Draw_Line(x + 38, y + 6, x + 44, y + 1, 1);

    // Eye
    LCD_Draw_Rect(x + 34, y + 9, 2, 2, 2, 1);

    // Nose
    LCD_Draw_Rect(x + 37, y + 14, 3, 2, 0, 1);

    // Legs
   // Animated bull legs
if (engine->runFrame == 0) {
    LCD_Draw_Rect(x + 5,  y + 24, 4, 9, 5, 1);
    LCD_Draw_Rect(x + 16, y + 24, 4, 6, 5, 1);
    LCD_Draw_Rect(x + 27, y + 24, 4, 9, 5, 1);
} else {
    LCD_Draw_Rect(x + 5,  y + 24, 4, 6, 5, 1);
    LCD_Draw_Rect(x + 16, y + 24, 4, 9, 5, 1);
    LCD_Draw_Rect(x + 27, y + 24, 4, 6, 5, 1);
}

    // Tail
    LCD_Draw_Line(x, y + 14, x - 7, y + 9, 12);
}
}


uint16_t RunnerEngine_GetScore(RunnerEngine_t* engine) {
    return engine->score;
}

uint8_t RunnerEngine_IsGameOver(RunnerEngine_t* engine) {
    return engine->gameOver;
}






// ===== RUNNER GAME VARIABLES =====
RunnerEngine_t runner_engine; // stores the runner game data

volatile uint8_t game_over = 0; // becomes 1 when the game ends

#define FPS 60
#define FRAME_TIME_MS (1000 / FPS)

void update_runner(UserInput input, uint8_t jumpPressed);
void render_runner(void);

int main(void)
{
    HAL_Init(); // starts STM32 HAL
    SystemClock_Config(); // sets up system clock
    PeriphCommonClock_Config(); // sets up peripheral clocks

    MX_GPIO_Init(); // starts GPIO pins
    MX_USART2_UART_Init(); // starts UART debugging
    MX_ADC1_Init(); // starts ADC for joystick
    MX_RNG_Init(); // starts random number generator

    LCD_init(&cfg0); // starts LCD screen
    LCD_Set_Palette(PALETTE_VINTAGE); // sets LCD colours

    MX_TIM2_Init(); // starts timer 2
    buzzer_init(&buzzer_cfg); // starts buzzer

    MX_TIM4_Init(); // starts timer 4 for PWM

    Joystick_Init(&joystick_cfg); // starts joystick

    RunnerEngine_Init(&runner_engine); // starts the runner game

    LCD_Fill_Buffer(0); // clears screen
    LCD_Refresh(&cfg0); // updates LCD

    LCD_printString("EXTREME", 45, 40, 1, 3); // title text
    LCD_printString("RUNNER", 35, 75, 1, 3); // title text
    LCD_Refresh(&cfg0);
    HAL_Delay(1000);

    LCD_Fill_Buffer(0); // clears title screen
    LCD_printString("Joystick L/R", 45, 30, 1, 2); // control text
    LCD_printString("to Run", 75, 60, 1, 2);
    LCD_printString(" Btn", 65, 95, 1, 2);
    LCD_printString("to Jump", 70, 120, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(2000);

    PWM_Init(&pwm_cfg); // starts PWM
    PWM_SetFreq(&pwm_cfg, 1000);
    PWM_SetDuty(&pwm_cfg, 0);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // turns LED off

    printf("Runner Game Engine initialized.\n");

    uint32_t last_tick = HAL_GetTick(); // stores time for frame timing

    while (!game_over) // main game loop
    {
        uint32_t now = HAL_GetTick(); // gets current time

        if ((now - last_tick) < FRAME_TIME_MS) { // waits for next frame
            continue;
        }

        last_tick = now; // updates frame timer

        Joystick_Read(&joystick_cfg, &joystick_data); // reads joystick
        UserInput input = Joystick_GetInput(&joystick_data); // gets direction

        uint8_t jumpPressed =
        (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_RESET); // checks jump button

        update_runner(input, jumpPressed); // updates game logic

        render_runner(); // draws game
    }

    int16_t line_offset = 0; // moves game over text

    while (1) // game over screen
    {
        LCD_Fill_Buffer(0);

        LCD_printString("Game Over!", 20, 0 + line_offset, 1, 3);

        char score_str[32];
        sprintf(score_str, "Score: %d", RunnerEngine_GetScore(&runner_engine));

        LCD_printString(score_str, 20, 20 + line_offset, 1, 2);

        LCD_Refresh(&cfg0);
        HAL_Delay(500);

        line_offset += 10;

        if (line_offset > 220) {
            line_offset = 0;
        }
    }
}

// updates the runner game
void update_runner(UserInput input, uint8_t jumpPressed) {
    uint8_t running = RunnerEngine_Update(&runner_engine, input, jumpPressed);

    if (running == 0) {
        printf("Game Over! Final Score: %d\n", RunnerEngine_GetScore(&runner_engine));
        game_over = 1;
    }
}

// draws the runner game
void render_runner(void) {
    LCD_Fill_Buffer(14);

    RunnerEngine_Draw(&runner_engine);

    char info_str[32];
    sprintf(info_str, "Score: %d", RunnerEngine_GetScore(&runner_engine));

    LCD_printString(info_str, 130, 10, 1, 2);

    LCD_Refresh(&cfg0);
}
// ===== Interrupt Callback