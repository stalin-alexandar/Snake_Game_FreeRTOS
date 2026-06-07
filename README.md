# FreeRTOS Snake Game

A classic Snake game running on the STM32F407G Discovery board with a 16x2 LCD screen and a joystick.

---

## What Is This?

This is a fully working Snake game on a microcontroller. You control a snake with a joystick, eat food to grow longer, and try to fill the entire screen. It uses FreeRTOS to handle input, game logic, and display all at the same time.

---

## How It Looks

```
+----------------+
|                |  <- 16 columns x 2 rows
|  ->***  @     |  <- Snake eating food
+----------------+
     ^
  Joystick
```

The 16x2 LCD gives you a tiny 16-column, 2-row playfield. The snake moves through this grid, and food appears at random spots.

---

## Game Rules

- Move the joystick to change the snake's direction
- Eat the food (shown as a small dot) to grow longer
- The snake **wraps around** the screen edges (no walls!)
- **Win condition:** Fill all 32 cells on the screen (the whole grid)!
- After winning, the game shows a victory screen for 3 seconds, then restarts
- The game gets faster every 5 points you score

---

## What You Need

| Item | What It's For |
|------|--------------|
| **STM32F407G-DISC1** board | The brain of the game |
| **16x2 LCD with I2C backpack** | The screen (connects to PB6/PB7) |
| **PS2 analog joystick** | The controller (connects to PA2/PA3) |
| **Jumper wires** | To connect everything |

### Wiring Diagram

```
LCD (I2C)              Joystick              STM32 Board
---------              --------              -----------
  SCL  ------------->  PB6
  SDA  ------------->  PB7
  VCC  ------------->  5V
  GND  ------------->  GND

  VRx  ------------->  PA2  (X axis)
  VRy  ------------->  PA3  (Y axis)
  VCC  ------------->  3.3V
  GND  ------------->  GND
```

---

## How to Build and Run

1. **Clone this repo:**
```bash
git clone https://github.com/stalin-alexandar/Snake_Game_FreeRTOS.git
```

2. **Open in STM32CubeIDE:**
   - File -> Import -> Existing Projects into Workspace
   - Select the Snake_Game_FreeRTOS folder

3. **Build:** Click Build (or Ctrl+B)

4. **Flash:** Click Run to upload to your board

5. **Play!** Move the joystick to control the snake

---

## How It Works (Behind the Scenes)

The game uses FreeRTOS to run three things at the same time:

| Task | Job | How Often |
|------|-----|-----------|
| **InputTask** | Reads the joystick | Every 50ms |
| **GameTask** | Moves the snake, checks food, updates score | Every 100-300ms |
| **DisplayTask** | Draws the game to the LCD | Only when something changes |

These three tasks talk to each other using:
- A **queue** to pass joystick commands
- A **semaphore** to tell the display "hey, redraw!"
- A **mutex** to make sure only one task uses the LCD at a time

### The Screen

The 16x2 LCD has 32 character cells. Each character can show a 5x8 pixel pattern, so the actual play area is **80 pixels wide x 16 pixels tall**.

The game uses custom characters for:
- **Snake head** - a solid block with eyes
- **Snake body** - a solid block
- **Food** - a small dot

### Speed Progression

The game starts slow and gets faster:

| Score | Speed (ms per tick) |
|-------|---------------------|
| 0-4 | 300ms (slow) |
| 5-9 | 280ms |
| 10-14 | 260ms |
| ... | keeps increasing |
| 25+ | 100ms (fast!) |

---

## Project Structure

```
Snake_Game_FreeRTOS/
├── STM32_SnakeGame/
│   ├── App/
│   │   ├── DisplayManager/    # LCD drawing code
│   │   ├── GameEngine/        # Snake movement, food, scoring
│   │   └── InputManager/      # Joystick reading code
│   ├── Core/
│   │   ├── Inc/               # Header files and FreeRTOS config
│   │   └── Src/               # main.c and system files
│   ├── Drivers/               # STM32 HAL library
│   └── Startup/               # Boot assembly code
└── README.md                  # This file
```

---

## Tips

- The LCD needs an I2C backpack (PCF8574 chip). Most "I2C LCD" modules have this built in.
- The I2C address is usually 0x27. If your LCD doesn't show anything, try 0x3F.
- The joystick uses analog input. Make sure it is connected to PA2 (X) and PA3 (Y).
- If the snake doesn't move, check your joystick wiring and make sure the game initialized properly.

---

## Author

**Stalin Alexandar** - [@stalin-alexandar](https://github.com/stalin-alexandar)
