/**
 * @file main.c
 * @brief Main application with FreeRTOS tasks for Snake Game
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lcd_display.h"
#include "joystick.h"
#include "snake_game.h"

/* ============================================================================
   PERIPHERAL HANDLES
   ============================================================================ */

I2C_HandleTypeDef hi2c1;
ADC_HandleTypeDef hadc1;

/* ============================================================================
   FREERTOS HANDLES
   ============================================================================ */

QueueHandle_t input_queue = NULL;
SemaphoreHandle_t lcd_mutex = NULL;
SemaphoreHandle_t game_update_sem = NULL;

TaskHandle_t input_task_handle = NULL;
TaskHandle_t game_task_handle = NULL;
TaskHandle_t display_task_handle = NULL;

/* ============================================================================
   TASK PROTOTYPES
   ============================================================================ */

void InputTask(void *pvParameters);
void GameTask(void *pvParameters);
void DisplayTask(void *pvParameters);

/* ============================================================================
   PERIPHERAL INITIALIZATION PROTOTYPES
   ============================================================================ */

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);

/* ============================================================================
   MAIN FUNCTION
   ============================================================================ */

int main(void)
{
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock to 168MHz */
    SystemClock_Config();

    /* Initialize peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();

    /* Initialize application modules */
    LCD_Init();
    Joystick_Init();

    /* Display splash screen */
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("  SNAKE GAME  ");
    LCD_SetCursor(1, 0);
    LCD_Print(" Press Joystick");
    HAL_Delay(2000);

    /* Initialize game */
    Game_Init();

    /* Create FreeRTOS synchronization objects */
    input_queue = xQueueCreate(INPUT_QUEUE_SIZE, sizeof(Direction_t));
    lcd_mutex = xSemaphoreCreateMutex();
    game_update_sem = xSemaphoreCreateBinary();

    /* Create FreeRTOS tasks */
    xTaskCreate(InputTask, "Input", INPUT_TASK_STACK_SIZE,
                NULL, INPUT_TASK_PRIORITY, &input_task_handle);

    xTaskCreate(GameTask, "Game", GAME_TASK_STACK_SIZE,
                NULL, GAME_TASK_PRIORITY, &game_task_handle);

    xTaskCreate(DisplayTask, "Display", DISPLAY_TASK_STACK_SIZE,
                NULL, DISPLAY_TASK_PRIORITY, &display_task_handle);

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1)
    {
    }
}

/* ============================================================================
   FREERTOS TASKS
   ============================================================================ */

/**
 * @brief Input task - Read joystick and send direction to game task
 * @param pvParameters Task parameters (unused)
 */
void InputTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(INPUT_TASK_PERIOD_MS);

    while (1)
    {
        // Read joystick direction
        Direction_t dir = Joystick_GetDirection();

        // Send to game task if direction changed
        if (dir != DIR_NONE)
        {
            xQueueSend(input_queue, &dir, 0);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Game task - Update game logic
 * @param pvParameters Task parameters (unused)
 */
void GameTask(void *pvParameters)
{
    Direction_t input_dir = DIR_NONE;
    TickType_t game_speed;

    while (1)
    {
        // Check for input from joystick
        if (xQueueReceive(input_queue, &input_dir, 0) == pdTRUE)
        {
            // Input received, will be processed in Game_Update
        }

        // Update game state
        Game_Update(input_dir);
        input_dir = DIR_NONE;  // Clear input after processing

        // Signal display task to render
        xSemaphoreGive(game_update_sem);

        // Check if game over
        if (Game_IsOver())
        {
            // Show game over screen for 3 seconds
            vTaskDelay(pdMS_TO_TICKS(3000));

            // Reset game
            Game_Reset();

            // Clear input queue
            xQueueReset(input_queue);
        }

        // Variable speed based on game progression
        game_speed = pdMS_TO_TICKS(Game_GetSpeed());
        vTaskDelay(game_speed);
    }
}

/**
 * @brief Display task - Render game to LCD
 * @param pvParameters Task parameters (unused)
 */
void DisplayTask(void *pvParameters)
{
    while (1)
    {
        // Wait for game update signal
        if (xSemaphoreTake(game_update_sem, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            // Acquire LCD mutex
            if (xSemaphoreTake(lcd_mutex, portMAX_DELAY) == pdTRUE)
            {
                // Render game
                Game_Render();

                // Release LCD mutex
                xSemaphoreGive(lcd_mutex);
            }
        }
    }
}

/* ============================================================================
   SYSTEM CLOCK CONFIGURATION
   ============================================================================ */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ============================================================================
   PERIPHERAL INITIALIZATION
   ============================================================================ */

/**
 * @brief GPIO Initialization
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Configure GPIO pins for status LEDs */
    GPIO_InitStruct.Pin = LED_GREEN_Pin | LED_ORANGE_Pin | LED_RED_Pin | LED_BLUE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Turn on green LED to indicate system running */
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
}

/**
 * @brief I2C1 Initialization (for LCD)
 */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;  // 100kHz
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief ADC1 Initialization (for Joystick)
 */
static void MX_ADC1_Init(void)
{
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
}



/* ============================================================================
   UTILITY FUNCTIONS
   ============================================================================ */

/**
 * @brief Set status LEDs
 * @param game_active true if game is running
 * @param game_over true if game over
 */
void LED_SetStatus(bool game_active, bool game_over)
{
    // Green LED: System running
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);

    // Orange LED: Game active
    HAL_GPIO_WritePin(LED_ORANGE_GPIO_Port, LED_ORANGE_Pin,
                     game_active ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Red LED: Game over
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
                     game_over ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Error Handler
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        HAL_Delay(100);
    }
}

/* ============================================================================
   FREERTOS HOOK FUNCTIONS
   ============================================================================ */

/**
 * @brief Stack overflow hook
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    Error_Handler();
}

/**
 * @brief Malloc failed hook
 */
void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

#ifdef USE_FULL_ASSERT
/**
 * @brief Assert failed handler
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    Error_Handler();
}
#endif
