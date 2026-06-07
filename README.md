# 🐍 FreeRTOS Snake Game

> A classic Snake game on STM32F407G Discovery with FreeRTOS multitasking, 16x2 LCD display, and joystick control

[![Platform](https://img.shields.io/badge/Platform-STM32F407-blue.svg)](https://www.st.com/)
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green.svg)](https://www.freertos.org/)
[![Display](https://img.shields.io/badge/Display-16x2_LCD-orange.svg)](https://www.adafruit.com/)
[![Input](https://img.shields.io/badge/Input-Analog_Joystick-red.svg)](https://www.adafruit.com/)

---

## 📋 Overview

A fully functional Snake game on the STM32F407G Discovery board using FreeRTOS for real-time multitasking. Control a snake with an analog joystick, eat food to grow longer, and try to fill the entire 16x2 LCD screen. The game features wrap-around movement, progressive difficulty, and a victory condition.

**Built as a portfolio project to demonstrate FreeRTOS task architecture and embedded game development.**

### Key Features
- 🐍 **Classic Snake Gameplay** - Move, eat, grow, and win
- 📺 **16x2 LCD Display** - Custom character bitmaps for snake and food
- 🎮 **Analog Joystick Control** - Smooth directional input with deadzone filtering
- ⚡ **FreeRTOS Multitasking** - Three concurrent tasks for input, game, and display
- 🔄 **Wrap-around Movement** - Snake wraps around screen edges
- 📈 **Progressive Difficulty** - Speed increases every 5 points
- 🏆 **Victory Condition** - Fill all 32 cells to win

---

## 🔧 Hardware Requirements

### Your Components
- **STM32F407G-DISC1** - Main board (168MHz Cortex-M4)
- **16x2 LCD with I2C backpack** - Display (address 0x27)
- **PS2 Analog Joystick** - Controller module
- **Jumper Wires** - For connections

### Pin Configuration

| Pin | Function | Connection |
|-----|----------|------------|
| PA2 | ADC1 CH2 | Joystick X-axis |
| PA3 | ADC1 CH3 | Joystick Y-axis |
| PB6 | I2C1 SCL | LCD SCL |
| PB7 | I2C1 SDA | LCD SDA |
| PD12-PD15 | LEDs | Status indicators |

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────┐
│        FreeRTOS Scheduler               │
├──────────┬──────────────┬───────────────┤
│ InputTask│   GameTask    │ DisplayTask   │
│ Priority 3│  Priority 2   │ Priority 1    │
│ (Highest)│               │ (Lowest)      │
├──────────┼──────────────┼───────────────┤
│ Joystick │  Snake       │  LCD I2C      │
│ ADC      │  Engine      │  Driver       │
│ PA2, PA3 │  Movement,   │  16x2 at      │
│          │  Food, Score │  0x27         │
└────┬─────┴──────┬───────┴───────┬───────┘
     │            │               │
     │ input_queue│ game_update_sem│ lcd_mutex
     └────────────┴───────────────┘
```

---

## 🎮 Game Rules

- 🕹️ Move the joystick to change the snake's direction
- 🍎 Eat food to grow longer
- 🔄 Snake wraps around screen edges (no walls!)
- 🏆 Win by filling all 32 cells
- ⚡ Game gets faster every 5 points
- 🎯 Victory screen shows for 3 seconds, then restarts

---

## 📊 Technical Specifications

### Performance
- **Display**: 16x2 LCD (80x16 pixel virtual grid)
- **Input**: 50ms polling with deadzone filtering
- **Game Speed**: 300ms → 100ms per tick
- **LCD Update**: Event-driven (semaphore-triggered)

### FreeRTOS Tasks

| Task | Priority | Period | Description |
|------|----------|--------|-------------|
| InputTask | 3 (highest) | 50ms | Reads joystick, filters input |
| GameTask | 2 | 300-100ms | Updates snake, checks food, scoring |
| DisplayTask | 1 (lowest) | On signal | Renders LCD grid |

### Resource Usage
- **Flash**: ~5% of 1MB
- **RAM**: ~10KB of 128KB
- **FreeRTOS Heap**: 20KB
- **Tasks**: 3 application + idle + timer

---

## 📁 Project Structure

```
Snake_Game_FreeRTOS/
├── STM32_SnakeGame/
│   ├── App/
│   │   ├── DisplayManager/    # LCD driver
│   │   ├── GameEngine/        # Snake logic
│   │   └── InputManager/      # Joystick driver
│   ├── Core/
│   │   ├── Inc/               # Headers + FreeRTOS config
│   │   └── Src/               # main.c + system files
│   └── Drivers/               # STM32 HAL library
└── README.md
```

---

## 🚀 Getting Started

### Prerequisites
- STM32CubeIDE (v1.18.0+)
- ST-LINK programmer (built into Discovery board)
- Hardware components (LCD + Joystick)

### Quick Start
1. **Clone repository**
   ```bash
   git clone https://github.com/stalin-alexandar/Snake_Game_FreeRTOS.git
   ```

2. **Open in STM32CubeIDE**
   - File → Import → Existing Projects
   - Select project folder

3. **Build and Flash**
   - Build (Ctrl+B) → Run

4. **Wire Hardware**
   - LCD: SCL→PB6, SDA→PB7
   - Joystick: X→PA2, Y→PA3

5. **Play!**
   - Move joystick to control the snake

---

## 🎓 Skills Demonstrated

### Embedded Systems
- ✅ STM32F407 programming (Cortex-M4)
- ✅ FreeRTOS real-time scheduling
- ✅ I2C LCD communication
- ✅ ADC for analog input
- ✅ Custom character bitmaps
- ✅ Timer-based game loop

### Game Development
- ✅ State machine design
- ✅ Collision detection
- ✅ Progressive difficulty system
- ✅ Input handling with deadzone
- ✅ Event-driven rendering

### Software Architecture
- ✅ Multi-tasking with priorities
- ✅ Producer-consumer pattern (queue)
- ✅ Mutual exclusion (mutex)
- ✅ Synchronization (semaphore)

---

## 💼 For Employers

This project demonstrates:
- **Real-Time Systems**: FreeRTOS task scheduling with priorities
- **Input/Output**: ADC joystick, I2C LCD, GPIO LEDs
- **Game Logic**: State machine, collision detection, scoring
- **Embedded Architecture**: Clean separation of concerns (Input/Game/Display)

**Interview Pitch**: "I built a Snake game on STM32F407 using FreeRTOS with three concurrent tasks for input, game logic, and display. The system demonstrates real-time scheduling, I2C communication, and embedded game development."

---

## 📝 License

MIT License - Free for educational and portfolio use

---

## 👤 Author

**Stalin Alexandar**
- GitHub: [@stalin-alexandar](https://github.com/stalin-alexandar)

---

**Built with ❤️ for embedded systems enthusiasts**
