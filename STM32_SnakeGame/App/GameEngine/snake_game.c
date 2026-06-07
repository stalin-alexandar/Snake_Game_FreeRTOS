/**
 * @file snake_game.c
 * @brief Snake game engine implementation
 */

#include "snake_game.h"
#include "lcd_display.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>

/* Custom LCD Characters - 2×2 pixel block patterns */
static uint8_t char_snake_head[8] = {
    0b00000,  // .....
    0b00000,  // .....
    0b00000,  // .....
    0b01100,  // .##..
    0b01100,  // .##..
    0b00000,  // .....
    0b00000,  // .....
    0b00000   // .....
};

static uint8_t char_snake_body[8] = {
    0b00000,  // .....
    0b00000,  // .....
    0b00000,  // .....
    0b01100,  // .##..
    0b01100,  // .##..
    0b00000,  // .....
    0b00000,  // .....
    0b00000   // .....
};

static uint8_t char_food[8] = {
    0b00000,  // .....
    0b00000,  // .....
    0b00000,  // .....
    0b01100,  // .##..
    0b01100,  // .##..
    0b00000,  // .....
    0b00000,  // .....
    0b00000   // .....
};

#define CHAR_SNAKE_HEAD  1
#define CHAR_SNAKE_BODY  2
#define CHAR_FOOD        3

/* ============================================================================
   PRIVATE VARIABLES
   ============================================================================ */

static Snake_t snake;
static GameState_t game_state;
static bool initialized = false;

/* ============================================================================
   PRIVATE FUNCTION PROTOTYPES
   ============================================================================ */

static void InitSnake(void);
static void SpawnFood(void);
//static bool CheckSelfCollision(Position_t pos);
static void MoveSnake(void);
static bool IsValidDirection(Direction_t new_dir, Direction_t current_dir);
static uint32_t GetRandomNumber(uint32_t max);

/* ============================================================================
   PUBLIC FUNCTIONS
   ============================================================================ */

void Game_Init(void) {
    // Create custom characters for 2-pixel snake
    LCD_CreateChar(CHAR_SNAKE_HEAD, char_snake_head);
    LCD_CreateChar(CHAR_SNAKE_BODY, char_snake_body);
    LCD_CreateChar(CHAR_FOOD, char_food);

    // Initialize random seed
    srand(HAL_GetTick());

    // Initialize snake
    InitSnake();

    // Initialize game state
    game_state.score = 0;
    game_state.game_over = false;
    game_state.victory = false;
    game_state.speed_ms = INITIAL_SPEED_MS;
    game_state.last_update = HAL_GetTick();

    // Spawn initial food
    SpawnFood();

    initialized = true;
}

void Game_Update(Direction_t input) {
    if (!initialized || game_state.game_over) {
        return;
    }

    // Buffer input direction (validate against 180° reversal)
    if (input != DIR_NONE && IsValidDirection(input, snake.direction)) {
        snake.next_direction = input;
    }

    // Update snake direction
    if (snake.next_direction != DIR_NONE) {
        snake.direction = snake.next_direction;
    }

    // Move snake
    MoveSnake();

    // Wall collision removed - wrap-around enabled
    /*
    // Check wall collision
    Position_t head = snake.body[0];
    if (head.x >= GAME_WIDTH || head.y >= GAME_HEIGHT) {
        game_state.game_over = true;
        return;
    }
    */
    Position_t head = snake.body[0];

    // Self-collision removed - snake can pass through itself
    /*
    // Check self collision
    if (CheckSelfCollision(head)) {
        game_state.game_over = true;
        return;
    }
    */

    // Check food collision
    if (head.x == game_state.food.x && head.y == game_state.food.y) {
        // Grow snake (don't remove tail)
        snake.length++;
        if (snake.length > MAX_SNAKE_LENGTH) {
            snake.length = MAX_SNAKE_LENGTH;
        }

        // Check victory condition (grid completely filled)
        if (snake.length >= GAME_WIDTH * GAME_HEIGHT) {
            game_state.game_over = true;
            game_state.victory = true;
            return;
        }

        // Increase score
        game_state.score++;

        // Increase speed every SPEED_INCREMENT points
        if (game_state.score % SPEED_INCREMENT == 0) {
            if (game_state.speed_ms > MIN_SPEED_MS) {
                game_state.speed_ms -= SPEED_DECREASE_MS;
                if (game_state.speed_ms < MIN_SPEED_MS) {
                    game_state.speed_ms = MIN_SPEED_MS;
                }
            }
        }

        // Spawn new food
        SpawnFood();
    }

    game_state.last_update = HAL_GetTick();
}

void Game_Render(void) {
    if (!initialized) {
        return;
    }

    // Clear display
    LCD_Clear();

    if (game_state.game_over) {
        // Display appropriate message
        LCD_SetCursor(0, 0);
        if (game_state.victory) {
            LCD_Print("   YOU  WIN!   ");
            LCD_SetCursor(1, 0);
            LCD_Printf(" Score: %3d/32 ", game_state.score);
        } else {
            LCD_Print("  GAME  OVER  ");
            LCD_SetCursor(1, 0);
            LCD_Printf("  Score: %3d  ", game_state.score);
        }
        return;
    }

    // Create display buffer
    char line1[GAME_WIDTH + 1] = {0};
    char line2[GAME_WIDTH + 1] = {0};

    // Initialize with spaces
    memset(line1, ' ', GAME_WIDTH);
    memset(line2, ' ', GAME_WIDTH);

    // Draw snake body using custom characters
    for (uint8_t i = 0; i < snake.length; i++) {
        Position_t pos = snake.body[i];

        // Clamp position to valid range (defensive programming)
        if (pos.x >= GAME_WIDTH) pos.x = GAME_WIDTH - 1;
        if (pos.y >= GAME_HEIGHT) pos.y = GAME_HEIGHT - 1;

        char symbol = (i == 0) ? CHAR_SNAKE_HEAD : CHAR_SNAKE_BODY;
        if (pos.y == 0) {
            line1[pos.x] = symbol;
        } else {
            line2[pos.x] = symbol;
        }
    }

    // Draw food using custom character
    if (game_state.food.x < GAME_WIDTH && game_state.food.y < GAME_HEIGHT) {
        if (game_state.food.y == 0) {
            line1[game_state.food.x] = CHAR_FOOD;
        } else {
            line2[game_state.food.x] = CHAR_FOOD;
        }
    }

    // Display on LCD
    LCD_SetCursor(0, 0);
    LCD_Print(line1);
    LCD_SetCursor(1, 0);
    LCD_Print(line2);
}

bool Game_IsOver(void) {
    return game_state.game_over;
}

uint16_t Game_GetScore(void) {
    return game_state.score;
}

uint32_t Game_GetSpeed(void) {
    return game_state.speed_ms;
}

void Game_Reset(void) {
    Game_Init();
}

GameState_t* Game_GetState(void) {
    return &game_state;
}

/* ============================================================================
   PRIVATE FUNCTIONS
   ============================================================================ */

/**
 * @brief Initialize snake to starting position
 */
static void InitSnake(void) {
    // Start in middle of display, moving right
    snake.length = INITIAL_SNAKE_LENGTH;
    snake.direction = DIR_RIGHT;
    snake.next_direction = DIR_RIGHT;

    // Position snake horizontally in center
    uint8_t start_x = GAME_WIDTH / 2 - INITIAL_SNAKE_LENGTH / 2;
    uint8_t start_y = GAME_HEIGHT / 2;

    for (uint8_t i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        snake.body[i].x = start_x + (INITIAL_SNAKE_LENGTH - 1 - i);
        snake.body[i].y = start_y;
    }
}

/**
 * @brief Spawn food at random empty position
 */
static void SpawnFood(void) {
    Position_t new_food;
    bool valid_position = false;
    uint8_t attempts = 0;

    while (!valid_position && attempts < 100) {
        new_food.x = GetRandomNumber(GAME_WIDTH);
        new_food.y = GetRandomNumber(GAME_HEIGHT);

        // Check if position is occupied by snake
        valid_position = true;
        for (uint8_t i = 0; i < snake.length; i++) {
            if (snake.body[i].x == new_food.x && snake.body[i].y == new_food.y) {
                valid_position = false;
                break;
            }
        }

        attempts++;
    }

    game_state.food = new_food;
}

/**
 * @brief Check if position collides with snake body
 * @param pos Position to check
 * @return true if collision with self
 */
//static bool CheckSelfCollision(Position_t pos) {
//    // Check against body (skip head at index 0)
//    for (uint8_t i = 1; i < snake.length; i++) {
//        if (snake.body[i].x == pos.x && snake.body[i].y == pos.y) {
//            return true;
//        }
//    }
//    return false;
//}


/**
 * @brief Move snake in current direction
 */
static void MoveSnake(void) {
    // Calculate new head position
    Position_t new_head = snake.body[0];

    switch (snake.direction) {
        case DIR_UP:
            if (new_head.y > 0) {
                new_head.y--;
            } else {
                new_head.y = GAME_HEIGHT - 1;  // Wrap around
            }
            break;

        case DIR_DOWN:
            new_head.y++;
            if (new_head.y >= GAME_HEIGHT) {
                new_head.y = 0;  // Wrap around
            }
            break;

        case DIR_LEFT:
            if (new_head.x == 0) {
                new_head.x = GAME_WIDTH - 1;  // Wrap to right edge
            } else {
                new_head.x--;
            }
            break;

        case DIR_RIGHT:
            new_head.x++;
            if (new_head.x >= GAME_WIDTH) {
                new_head.x = 0;  // Wrap around
            }
            break;

        case DIR_NONE:
        default:
            return;  // Don't move
    }

    // Clamp to valid range (safety check to prevent any out-of-bounds)
    if (new_head.x >= GAME_WIDTH) {
        new_head.x = GAME_WIDTH - 1;
    }
    if (new_head.y >= GAME_HEIGHT) {
        new_head.y = GAME_HEIGHT - 1;
    }

    // Shift body segments
    for (int8_t i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }

    // Set new head position
    snake.body[0] = new_head;
}

/**
 * @brief Check if direction change is valid (no 180° reversal)
 * @param new_dir New direction
 * @param current_dir Current direction
 * @return true if valid
 */
static bool IsValidDirection(Direction_t new_dir, Direction_t current_dir) {
    // Can't reverse 180 degrees
    if ((new_dir == DIR_UP && current_dir == DIR_DOWN) ||
        (new_dir == DIR_DOWN && current_dir == DIR_UP) ||
        (new_dir == DIR_LEFT && current_dir == DIR_RIGHT) ||
        (new_dir == DIR_RIGHT && current_dir == DIR_LEFT)) {
        return false;
    }
    return true;
}

/**
 * @brief Generate random number
 * @param max Maximum value (exclusive)
 * @return Random number [0, max)
 */
static uint32_t GetRandomNumber(uint32_t max) {
    if (max == 0) return 0;
    return (uint32_t)(rand() % max);
}
