# FreeRTOS Snake Game — STM32F407G Discovery

A classic Snake game running on the STM32F407G Discovery board, built with FreeRTOS for real-time multitasking. The game renders on a 16×2 character LCD via I2C and accepts input from an analog joystick via ADC.

---

## Overview

This project implements a complete Snake game on bare-metal STM32 hardware using FreeRTOS as the operating system. The architecture is split into three concurrent tasks — input reading, game logic, and display rendering — communicating through FreeRTOS queues, semaphores, and mutexes.

---

## Key Features

- **FreeRTOS Multitasking** — three tasks with distinct priorities for input, game logic, and display
- **16×2 LCD Display** — renders the snake and food using custom character bitmaps via I2C (PCF8574 expander)
- **Analog Joystick Input** — reads X/Y axes via ADC with deadzone filtering and direction detection
- **Wrap-around Movement** — snake wraps around screen edges (no wall collisions)
- **Progressive Difficulty** — game speed increases every 5 points scored
- **Victory Condition** — win when the snake fills all 32 grid cells
- **Custom LCD Characters** — snake head, body, and food rendered as 5×8 pixel glyphs
- **Onboard LED Status** — four LEDs used for visual feedback

---

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                    FreeRTOS Scheduler                │
├──────────────┬──────────────────┬───────────────────┤
│  InputTask   │    GameTask      │   DisplayTask     │
│  Priority 3  │   Priority 2     │   Priority 1      │
│  (Highest)   │                  │   (Lowest)        │
├──────────────┼──────────────────┼───────────────────┤
│ Joystick ADC │  Snake Engine    │  LCD I2C Driver   │
│  PA2 (X)     │  Movement, food  │  16×2 character   │
│  PA3 (Y)     │  collision, score│  display at 0x27  │
└──────┬───────┴────────┬─────────┴─────────┬─────────┘
       │                │                   │
       │   input_queue  │  game_update_sem  │ lcd_mutex
       └────────────────┴───────────────────┘
```

### Task Descriptions

| Task | Priority | Period | Description |
|------|----------|--------|-------------|
| **InputTask** | 3 (highest) | 50 ms | Reads joystick ADC values, applies deadzone filtering, detects direction changes, sends valid directions to `input_queue` |
| **GameTask** | 2 | 300→100 ms | Processes player input from queue, updates snake position, checks food collisions, manages score and speed progression |
| **DisplayTask** | 1 (lowest) | On signal | Waits on `game_update_sem`, redraws the entire 16×2 LCD grid with snake and food positions |

### Synchronization Primitives

| Primitive | Type | Purpose |
|-----------|------|--------|
| `input_queue` | FreeRTOS Queue | Passes joystick direction commands from InputTask to GameTask |
| `game_update_sem` | Binary Semaphore | Signals DisplayTask that the game state has changed and needs redrawing |
| `lcd_mutex` | Mutex | Protects shared LCD I2C bus access from concurrent writes |

---

## Hardware Requirements

- **MCU**: STM32F407G-DISC1 Discovery Board
- **Display**: 16×2 Character LCD with PCF8574 I2C backpack (address `0x27`)
- **Input**: PS2 analog joystick module
- **Connections**:

| Signal | Pin | Connection |
|--------|-----|------------|
| LCD SCL | PB6 | I2C1 Clock → LCD SCL |
| LCD SDA | PB7 | I2C1 Data → LCD SDA |
| Joystick X | PA2 | ADC1 Channel 2 |
| Joystick Y | PA3 | ADC1 Channel 3 |
| LED LD3 | PD13 | Onboard Green LED |
| LED LD4 | PD12 | Onboard Orange LED |
| LED LD5 | PD14 | Onboard Red LED |
| LED LD6 | PD15 | Onboard Blue LED |

---

## Game Mechanics

### Grid
- **16 columns × 2 rows** = 32 total cells
- Snake head starts at center, facing right
- Initial snake length: 3 segments

### Controls
- **Joystick Left** → move snake left
- **Joystick Right** → move snake right
- **Joystick Up** → move snake up
- **Joystick Down** → move snake down
- **No input** → snake continues in current direction
- U-turns are blocked (cannot reverse direction instantly)

### Rules
- **No wall collision** — snake wraps around edges
- **No self-collision** — snake can cross itself
- **Food** spawns at random unoccupied cells
- **Score** increases by 1 for each food eaten
- **Speed** increases by 20 ms every 5 points (from 300 ms down to 100 ms minimum)
- **Win condition** — snake reaches 32 segments (fills entire grid)
- After winning, a 3-second pause displays the victory screen, then the game auto-resets

### Custom LCD Characters

| Index | Character | Bitmap |
|-------|-----------|--------|
| 1 | Snake Head | Solid filled block with eyes |
| 2 | Snake Body | Solid filled block |
| 3 | Food | Small diamond/dot pattern |

> **Important**: Custom character indices must be 1–7. Using index 0 causes C-string functions to treat it as a null terminator, making segments invisible.

---

## Project Structure

```
Snake_Game_FreeRTOS/
├── STM32_SnakeGame/
│   ├── App/
│   │   ├── DisplayManager/
│   │   │   ├── lcd_display.c       # I2C LCD driver (4-bit mode via PCF8574)
│   │   │   └── lcd_display.h
│   │   ├── GameEngine/
│   │   │   ├── snake_game.c        # Snake movement, food, scoring logic
│   │   │   └── snake_game.h
│   │   └── InputManager/
│   │       ├── joystick.c          # ADC joystick driver with deadzone
│n   │       └── joystick.h
│   ├── Core/
│   │   ├── Inc/
│   │   │   ├── main.h
│   │   │   ├── FreeRTOSConfig.h    # RTOS configuration (168 MHz, 20KB heap)
│   │   │   ├── stm32f4xx_hal_conf.h
│   │   │   └── stm32f4xx_it.h
│   │   └── Src/
│   │       ├── main.c              # Entry point, task creation, peripherals
│   │       ├── stm32f4xx_hal_msp.c
│   │       ├── stm32f4xx_it.c
│   │       └── system_stm32f4xx.c
│   ├── Drivers/                     # STM32 HAL and CMSIS drivers
│   └── Startup/
│       └── startup_stm32f407vgtx.s
├── GAME_LOGIC.md                    # Detailed game logic documentation
├── PROJECT_SUMMARY.md               # Project summary and status
└── README.md                        # This file
```

---

## Getting Started

### Prerequisites

- **STM32CubeIDE** 1.18.0 or later
- **ST-LINK** programmer/debugger (onboard on Discovery board)
- **Hardware**: 16×2 I2C LCD + PS2 joystick module

### Build and Flash

1. Clone the repository:
```bash
git clone https://github.com/stalin-alexandar/Snake_Game_FreeRTOS.git
cd Snake_Game_FreeRTOS
```

2. Open `STM32_SnakeGame` in STM32CubeIDE:
   - File → Import → Existing Projects into Workspace
   - Select the `STM32_SnakeGame` folder

3. Build the project:
   - Right-click project → Build Project (or `Ctrl+B`)

4. Flash to the board:
   - Right-click project → Run As → STM32 C/C++ Application
   - Or use the Debug configuration for step-through debugging

### Wiring

1. Connect the I2C LCD backpack:
   - SCL → PB6
   - SDA → PB7
   - VCC → 5V
   - GND → GND

2. Connect the PS2 joystick module:
   - VRx → PA2 (ADC1_CH2)
   - VRy → PA3 (ADC1_CH3)
   - VCC → 3.3V
   - GND → GND

3. Power the Discovery board via USB.

4. The game starts automatically — move the joystick to control the snake!

---

## FreeRTOS Configuration

| Parameter | Value |
|-----------|-------|
| CPU Clock | 168 MHz |
| Tick Rate | 1000 Hz |
| Total Heap | 20 KB |
| Max Priorities | 5 |
| Preemption | Enabled |
| Stack Overflow Check | Mode 2 |
| Software Timers | Enabled |
| Mutexes | Enabled |

---

## Resource Usage

- **RAM**: ~10 KB (of 128 KB available)
- **Flash**: ~5% of 1 MB
- **Tasks**: 3 application + idle + timer
- **Synchronization**: 1 queue, 1 binary semaphore, 1 mutex

---

## Technical Notes

- The LCD driver uses **4-bit mode** via the PCF8574 I2C expander, bit-banging the enable pulse through software delays.
- Joystick readings use a **±200 deadzone** from the center value (2048) to filter noise.
- The random food position generator uses `HAL_GetTick()` as the seed for `srand()`.
- Display updates are **semaphore-triggered** (not periodic), so the LCD only redraws when the game state changes.
- The `lcd_mutex` ensures thread-safe LCD access if any ISR or higher-p
