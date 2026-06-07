# Snake Game FreeRTOS - STM32F407G Discovery

A classic Snake game implementation for STM32F407G Discovery board using FreeRTOS, featuring a 16x2 LCD display and PS2 joystick control.

## Hardware Requirements

- **STM32F407G Discovery Board** (STM32F407VGT6)
- **16x2 LCD Display** with PCF8574 I2C adapter
- **2-Axis PS2 Joystick Module** (analog output)
- Jumper wires for connections

## Hardware Connections

### LCD Display (I2C)
- **VCC** → 5V
- **GND** → GND
- **SCL** → PB8 (I2C1_SCL)
- **SDA** → PB9 (I2C1_SDA)

### PS2 Joystick
- **VCC** → 3.3V
- **GND** → GND
- **VRx** → PA2 (ADC1_IN2) - X-axis
- **VRy** → PA3 (ADC1_IN3) - Y-axis
- **SW** → PA4 (optional button)

### Status LEDs (Onboard)
- **Green (PD12)** - System running
- **Orange (PD13)** - Game active
- **Red (PD14)** - Game over
- **Blue (PD15)** - Available

## Project Structure

```
STM32_SnakeGame/
├── Core/
│   ├── Inc/                    # Header files
│   │   ├── main.h
│   │   ├── FreeRTOSConfig.h
│   │   ├── stm32f4xx_hal_conf.h
│   │   └── stm32f4xx_it.h
│   ├── Src/                    # Source files
│   │   ├── main.c
│   │   ├── stm32f4xx_it.c
│   │   ├── stm32f4xx_hal_msp.c
│   │   ├── system_stm32f4xx.c
│   │   ├── syscalls.c
│   │   └── sysmem.c
│   └── Startup/
│       └── startup_stm32f407vgtx.s
├── App/
│   ├── DisplayManager/         # LCD driver
│   │   ├── lcd_display.h
│   │   └── lcd_display.c
│   ├── InputManager/           # Joystick driver
│   │   ├── joystick.h
│   │   └── joystick.c
│   └── GameEngine/             # Snake game logic
│       ├── snake_game.h
│       └── snake_game.c
├── FreeRTOS/                   # FreeRTOS kernel
├── Drivers/                    # STM32 HAL drivers
└── STM32F407VGTX_FLASH.ld     # Linker script
```

## Features

- **Classic Snake Gameplay**: Control the snake to eat food and grow
- **16x2 LCD Display**: Compact game rendering on character display
- **Joystick Control**: Smooth 4-direction control
- **Progressive Difficulty**: Game speed increases with score
- **FreeRTOS Tasks**: Multi-threaded architecture
  - Input Task (50ms) - Joystick reading
  - Game Task (100-300ms) - Game logic
  - Display Task (100ms) - LCD rendering
- **Collision Detection**: Wall and self-collision
- **Score Tracking**: Real-time score display
- **Auto Reset**: Game restarts after game over

## Game Controls

- **Joystick Up** - Move snake up
- **Joystick Down** - Move snake down
- **Joystick Left** - Move snake left
- **Joystick Right** - Move snake right

## Game Rules

1. Control the snake to eat food (*)
2. Snake grows longer with each food eaten
3. Score increases by 1 per food
4. Game speed increases every 5 points
5. Game over if snake hits itself or wall
6. Game automatically resets after 3 seconds

## Display Layout

**Line 1 (16 chars):** Game field
- Snake head: `O`
- Snake body: `#`
- Food: `*`
- Empty: ` ` (space)

**Line 2 (16 chars):** Score display
- Format: "Score: XXX"

## Building the Project

### Using STM32CubeIDE

1. Open STM32CubeIDE
2. File → Import → Existing Projects into Workspace
3. Select `Snake_Game_FreeRTOS/STM32_SnakeGame`
4. Project → Build All (Ctrl+B)
5. Run → Debug (F11) or Run (Ctrl+F11)

### Build Configuration

- **Toolchain**: GNU ARM Embedded Toolchain
- **Optimization**: -O0 (Debug) / -O2 (Release)
- **FPU**: Hardware floating point (fpv4-sp-d16)
- **Heap Size**: 20KB
- **Stack Size**: 1KB

## Memory Usage

- **Flash**: ~50KB (code + constants)
- **RAM**: ~10KB (heap + stacks + data)
- **Available**: 974KB Flash, 118KB RAM remaining

## FreeRTOS Configuration

- **CPU Clock**: 168MHz
- **Tick Rate**: 1000Hz (1ms)
- **Heap Size**: 20KB
- **Task Priorities**:
  - Input Task: 3 (High)
  - Game Task: 2 (Normal)
  - Display Task: 1 (Low)

## Troubleshooting

### LCD not displaying
- Check I2C connections (SCL, SDA)
- Verify I2C address (default 0x27, try 0x3F)
- Adjust LCD contrast potentiometer
- Check 5V power supply

### Joystick not responding
- Verify ADC connections (PA2, PA3)
- Check 3.3V power supply
- Test joystick with multimeter (should read ~1.65V center)
- Adjust deadzone in `joystick.h` if needed

### Game runs too fast/slow
- Adjust `INITIAL_SPEED_MS` in `snake_game.h`
- Modify `SPEED_DECREASE_MS` for difficulty curve

### Build errors
- Ensure all FreeRTOS and HAL driver files are present
- Check include paths in project settings
- Verify linker script is correct

## Customization

### Change Game Speed
Edit `snake_game.h`:
```c
#define INITIAL_SPEED_MS    300     // Starting speed
#define MIN_SPEED_MS        100     // Maximum speed
#define SPEED_DECREASE_MS   20      // Speed increase per level
```

### Adjust Joystick Sensitivity
Edit `joystick.h`:
```c
#define JOYSTICK_DEADZONE           200     // Center deadzone
#define JOYSTICK_THRESHOLD_LOW      1800    // Left/Down threshold
#define JOYSTICK_THRESHOLD_HIGH     2300    // Right/Up threshold
```

### Change LCD Address
Edit `lcd_display.h`:
```c
#define PCF8574_I2C_ADDRESS     0x27    // Try 0x3F if not working
```

## Technical Details

- **MCU**: STM32F407VGT6 (Cortex-M4F @ 168MHz)
- **RTOS**: FreeRTOS v11.1.0
- **HAL**: STM32F4 HAL Driver
- **I2C**: 100kHz standard mode
- **ADC**: 12-bit resolution, 84 cycle sampling
- **Display**: 16x2 character LCD (HD44780 compatible)

## License

MIT License - Free for educational and personal use

## Credits

Based on the Wireless Robot FreeRTOS project LCD driver.

## Version History

- **v1.0** (2026-05-14) - Initial release
  - Basic snake game functionality
  - Joystick control
  - LCD display
  - FreeRTOS task architecture

## Future Enhancements

- [ ] High score tracking (EEPROM)
- [ ] Multiple difficulty levels
- [ ] Sound effects (buzzer)
- [ ] Pause functionality
- [ ] Custom LCD characters for better graphics
- [ ] Obstacles and power-ups
- [ ] Two-player mode

---

**Enjoy the game!** 🐍
