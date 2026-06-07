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

### System Architecture

The game uses FreeRTOS to run three things at the same time:

```
+-------------------------------------+
|        FreeRTOS Scheduler            |
+----------+------------+------------+
| InputTask|  GameTask   | DisplayTask|
| Priority 3| Priority 2  | Priority 1 |
| (Highest)|             | (Lowest)   |
+----------+------------+------------+
| Joystick |  Snake     |  LCD I2C   |
| ADC      |  Engine    |  Driver    |
| PA2, PA3 |  Movement, |  16x2 at   |
|          |  Food,     |  0x27      |
|          |  Scoring   |            |
+-----+----+-----+------+-----+------+
      |          |            |
      | input_queue | game_update_sem | lcd_mutex
      +----------+------------+-------+
```

### Task Descriptions

| Task | Priority | Period | What It Does |
|------|----------|--------|-------------|
| **InputTask** | 3 (highest) | 50 ms | Reads joystick ADC values, applies deadzone filtering, detects direction changes, sends valid directions to input_queue |
| **GameTask** | 2 | 300-100 ms | Processes player input from queue, updates snake position, checks food collisions, manages score and speed progression |
| **DisplayTask** | 1 (lowest) | On signal | Waits on game_update_sem, redraws the entire 16x2 LCD grid with snake and food positions |

### Synchronization Primitives

| Primitive | Type | What It Does |
|-----------|------|-------------|
| input_queue | FreeRTOS Queue | Passes joystick direction commands from InputTask to GameTask |
| game_update_sem | Binary Semaphore | Signals DisplayTask that the game state has changed and needs redrawing |
| lcd_mutex | Mutex | Protects shared LCD I2C bus access from concurrent writes |

### The Screen

The 16x2 LCD has 32 character cells. Each character can show a 5x8 pixel pattern, so the actual play area is **80 pixels wide x 16 pixels tall**.

The game uses custom characters for:
| Index | Character | What It Looks Like |
|-------|-----------|-------------------|
| 1 | Snake Head | Solid filled block with eyes |
| 2 | Snake Body | Solid filled block |
| 3 | Food | Small diamond/dot pattern |

> **Important:** Custom character indices must be 1-7. Using index 0 causes C-string functions to treat it as a null terminator, making segments invisible.

### Speed Progression

The game starts slow and gets faster:

| Score | Speed (ms per tick) |
|-------|---------------------|
| 0-4 | 300ms (slow) |
| 5-9 | 280ms |
| 10-14 | 260ms |
| ... | keeps increasing |
| 25+ | 100ms (fast!) |

**Rule:** Speed increases by 20ms for every 5 points scored.

---

## Technical Specifications

### FreeRTOS Configuration

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

### Resource Usage

- **RAM:** ~10 KB (of 128 KB available)
- **Flash:** ~5% of 1 MB
- **Tasks:** 3 application + idle + timer
- **Synchronization:** 1 queue, 1 binary semaphore, 1 mutex

### Joystick Configuration

| Parameter | Value |
|-----------|-------|
| ADC Channels | X=CH2 (PA2), Y=CH3 (PA3) |
| Resolution | 12-bit (0-4095) |
| Center Value | 2048 |
| Deadzone | +/-200 from center |
| Left/Down Threshold | < 1800 |
| Right/Up Threshold | > 2300 |

### LCD Configuration

| Parameter | Value |
|-----------|-------|
| Type | 16x2 Character LCD |
| Interface | I2C via PCF8574 expander |
| I2C Address | 0x27 |
| I2C Pins | SCL=PB6, SDA=PB7 |
| Mode | 4-bit interface |

### Pin Configuration

| Pin | Function | Connection |
|-----|----------|------------|
| PA2 | ADC1 CH2 | Joystick X-axis |
| PA3 | ADC1 CH3 | Joystick Y-axis |
| PB6 | I2C1 SCL | LCD SCL |
| PB7 | I2C1 SDA | LCD SDA |
| PD13 | GPIO Output | LD3 Green LED |
| PD12 | GPIO Output | LD4 Orange LED |
| PD14 | GPIO Output | LD5 Red LED |
| PD15 | GPIO Output | LD6 Blue LED |

---

## Technical Notes

- The LCD driver uses **4-bit mode** via the PCF8574 I2C expander, bit-banging the enable pulse through software delays.
- Joystick readings use a **+/-200 deadzone** from the center value (2048) to filter noise.
- The random food position generator uses `HAL_GetTick()` as the seed for `srand()`.
- Display updates are **semaphore-triggered** (not periodic), so the LCD only redraws when the game state changes.
- The `lcd_mutex` ensures thread-safe LCD access if any ISR or higher-priority task attempts to use the display.

---

## Limitations

- **16x2 display** limits the visible game area (32 cells max)
- **No score persistence** - score resets on power cycle
- **No pause functionality**
- **No self-collision** - snake can cross itself (intentional design choice)
- **No sound** - no buzzer or audio feedback

---

## Future Enhancements

- [ ] Add EEPROM/Flash score persistence
- [ ] Implement pause/resume with button press
- [ ] Add self-collision detection for increased difficulty
- [ ] Support larger displays (LCD 20x4 or graphical OLED)
- [ ] Add sound effects via onboard buzzer
- [ ] Implement difficulty selection menu

---

## Project Structure

```
Snake_Game_FreeRTOS/
├── STM32_SnakeGame/
│   ├── App/
│   │   ├── DisplayManager/    # LCD drawing code
│   │   │   ├── lcd_display.c  # I2C LCD driver (4-bit mode via PCF8574)
│   │   │   └── lcd_display.h
│   │   ├── GameEngine/        # Snake movement, food, scoring
│   │   │   ├── snake_game.c   # Snake engine with custom characters
│   │   │   └── snake_game.h
│   │   └── InputManager/      # Joystick reading code
│   │       ├── joystick.c     # ADC joystick driver with deadzone
│   │       └── joystick.h
│   ├── Core/
│   │   ├── Inc/               # Header files and FreeRTOS config
│   │   │   ├── main.h
│   │   │   ├── FreeRTOSConfig.h
│   │   │   ├── stm32f4xx_hal_conf.h
│   │   │   └── stm32f4xx_it.h
│   │   └── Src/               # mai
