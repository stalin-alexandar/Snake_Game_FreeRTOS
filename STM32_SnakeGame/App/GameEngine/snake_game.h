/**
 * @file snake_game.h
 * @brief Snake game engine for 16x2 LCD display
 */

#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include "main.h"
#include "joystick.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
   GAME CONFIGURATION
   ============================================================================ */

#define GAME_WIDTH          16      // LCD columns
#define GAME_HEIGHT         2       // LCD rows
#define MAX_SNAKE_LENGTH    32      // Maximum snake length
#define INITIAL_SNAKE_LENGTH 3      // Starting length
#define INITIAL_SPEED_MS    300     // Initial game speed (ms per move)
#define MIN_SPEED_MS        100     // Minimum speed (fastest)
#define SPEED_INCREMENT     5       // Score points before speed increase
#define SPEED_DECREASE_MS   20      // Speed decrease per increment

/* ============================================================================
   TYPE DEFINITIONS
   ============================================================================ */

typedef struct {
    uint8_t x;
    uint8_t y;
} Position_t;

typedef struct {
    Position_t body[MAX_SNAKE_LENGTH];
    uint8_t length;
    Direction_t direction;
    Direction_t next_direction;     // Buffered input
} Snake_t;

typedef struct {
    Position_t food;
    uint16_t score;
    bool game_over;
    bool victory;                   // true if won, false if lost
    uint32_t speed_ms;              // Current movement interval
    uint32_t last_update;           // Last update timestamp
} GameState_t;

/* ============================================================================
   PUBLIC FUNCTIONS
   ============================================================================ */

/**
 * @brief Initialize game state
 */
void Game_Init(void);

/**
 * @brief Update game state with input direction
 * @param input Direction from joystick
 */
void Game_Update(Direction_t input);

/**
 * @brief Render game to LCD
 */
void Game_Render(void);

/**
 * @brief Check if game is over
 * @return true if game over
 */
bool Game_IsOver(void);

/**
 * @brief Get current score
 * @return Current score
 */
uint16_t Game_GetScore(void);

/**
 * @brief Get current game speed
 * @return Speed in milliseconds
 */
uint32_t Game_GetSpeed(void);

/**
 * @brief Reset game to initial state
 */
void Game_Reset(void);

/**
 * @brief Get game state (for debugging)
 * @return Pointer to game state
 */
GameState_t* Game_GetState(void);

#endif /* SNAKE_GAME_H */
