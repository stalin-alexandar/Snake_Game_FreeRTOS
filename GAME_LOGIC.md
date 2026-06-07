# 🐍 Snake Game Logic

**How this game works — explained simply, straight from the source code.**

---

## 🧱 The Hardware

| Part | What it does | Wiring |
|------|-------------|--------|
| **STM32F407G Discovery** | The brain (168 MHz ARM Cortex-M4) | — |
| **16×2 LCD** (I2C adapter) | Shows the game | PB9 (SDA), PB8 (SCL), address `0x27` |
| **Analog joystick** | You control the snake | PA2 (X-axis), PA3 (Y-axis) |

> There are also 4 LEDs on the board: Green = system on, Orange = game active, Red = game over, Blue = unused.

---

## 🧠 The Software (3 FreeRTOS Tasks)

The game runs **3 tasks** that talk to each other. Think of them as 3 workers:

```
┌──────────────────────────────────────────────────────────────┐
│                     FREERTOS SCHEDULER                        │
│  (decides which task runs and when)                          │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────────┐      queue       ┌─────────────────┐   │
│  │  InputTask       │ ──── direction ──▶  GameTask        │   │
│  │  (Priority 3)    │                  │  (Priority 2)    │   │
│  │  Runs every 50ms │                  │  Speed: 300-100ms│   │
│  └─────────────────┘                  └────────┬──────────┘   │
│                                                 │             │
│                                          semaphore            │
│                                                 │             │
│                                                 ▼             │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  DisplayTask (Priority 1)                                │  │
│  │  Waits for signal, then draws to LCD                     │  │
│  │  🛡️ Uses a mutex so no two tasks write to LCD at once   │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### What each task does:

**1. InputTask** — "The Listener"
- Reads the joystick every **50ms** (20 times/second)
- Sends the direction (UP/DOWN/LEFT/RIGHT) to GameTask via a **queue**
- Ignores tiny joystick wiggles (**100ms debounce**)

**2. GameTask** — "The Brain"
- Receives direction from the queue
- Updates the snake's position
- Checks if food was eaten
- Signals DisplayTask to redraw
- Runs at variable speed: **300ms → 100ms** (faster as you score)

**3. DisplayTask** — "The Artist"
- Waits for a **semaphore** signal from GameTask
- Draws the game to the LCD
- Uses a **mutex** (lock) so it never fights with other tasks over the LCD

---

## 🎮 How the Game Starts

When you power on:

1. LCD shows **"SNAKE GAME"** splash screen for 2 seconds
2. Snake appears centered on the **bottom row** (row 1), 3 segments long, moving RIGHT
3. One food item appears at a random empty spot
4. Game starts!

---

## 🐍 How the Snake Moves

The snake's body is an **array of (x, y) positions**. On each game tick:

### Step-by-step:
1. **Calculate new head position** based on direction
2. **Shift body**: tail falls off, each segment moves forward one spot
3. **Place new head** at the front

```
BEFORE (moving RIGHT):
Row 0: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Row 1: [ ][ ][■][■][■][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
                  ↑  ↑  ↑
              tail=2 body=3 head=4      

AFTER (one step right):
Row 0: [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Row 1: [ ][ ][ ][■][■][■][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
                     ↑  ↑  ↑
                 tail=3 body=4 head=5
```

### Wrapping (no walls!)

The snake wraps around edges instead of dying:

```
Moving RIGHT off the right edge:
[ ][ ][ ][ ][■][■][■]   → [■][■][■][ ][ ][ ][ ]
                       head wraps to x=0

Moving LEFT off the left edge:
[■][■][■][ ][ ][ ][ ]   → [ ][ ][ ][■][■][■]
                       head wraps to x=15
```

---

## 🍎 Eating Food (Growing!)

When the snake's head lands on the food:

```
BEFORE:
Row 1: [ ][ ][■][■][■][ ][*][ ][ ][ ][ ][ ][ ][ ][ ][ ]
                     ↑      ↑
                   head    food
                  (x=4)   (x=6)

Snake moves RIGHT twice...

AFTER (ate food, grew to 4):
Row 1: [ ][ ][ ][ ][■][■][■][■][ ][ ][ ][ ][ ][ ][ ][ ]
                           ↑
                         head at x=6 (where food was!)
```

**What happens:**
1. ✅ Snake length +1
2. ✅ Score +1
3. ✅ New food spawns at a random spot NOT on the snake
4. ✅ Every 5 points: game gets 20ms faster (300ms → 100ms minimum)

---

## 🏆 How You Win (or Lose)

### Winning
- **Fill all 32 grid positions** — snake length reaches 32
- LCD shows: `"   YOU  WIN!   "` and `" Score: 32/32 "`

### Losing
- **There is no losing!** 🎉
- No wall collision (snake wraps around)
- No self-collision (snake passes through itself)
- The only end is victory

### After game over
- **3-second pause** showing the result
- Game **auto-resets** back to the start

---

## 🎨 What You See (LCD Rendering)

Each game tick, DisplayTask:

1. **Clears** the LCD
2. Creates two **16-character strings** (one per row), filled with spaces
3. **Draws the snake**: places custom block characters at each segment's (x,y)
4. **Draws the food**: places a food character (same block icon)
5. **Sends both strings** to the LCD

> All three game characters (snake head, snake body, food) look the same: a **2×2 pixel block** centered in the character cell.

### Game Over Display
```
YOU WIN!:
Row 0: [   YOU  WIN!   ]
Row 1: [  Score: 32/32 ]

GAME OVER (shouldn't happen, but code handles it):
Row 0: [  GAME  OVER   ]
Row 1: [   Score:  N   ]
```

---

## 🕹️ Joystick Controls

### How the joystick is read
- ADC reads X (PA2) and Y (PA3) as 12-bit values (0–4095)
- **Center**: ~2048 → no movement
- **Deadzone**: ±200 → ignores tiny movements
- **Thresholds**: below 1800 = LEFT/DOWN, above 2300 = RIGHT/UP

```
             UP (Y > 2300)
                ↑
        LEFT ←──┼──→ RIGHT
       (X<1800) │  (X>2300)
                ↓
            DOWN (Y < 1800)
```

### What you CAN'T do
- **No 180° U-turns**: can't reverse instantly (RIGHT→LEFT, UP→DOWN, etc.)
- Why? Prevents weird behavior. You have to turn sideways first.

---

## ⚡ Speed Progression

| Score | Speed | Feel |
|-------|-------|------|
| 0–4   | 300ms | Slow, easy |
| 5–9   | 280ms | Slightly faster |
| 10–14 | 260ms | Medium |
| 15–19 | 240ms | Getting tough |
| 20–24 | 220ms | Fast |
| 25–29 | 200ms | Very fast |
| 30+   | 100ms | Maximum speed! |

**Formula**: `speed = max(100, 300 - (score ÷ 5) × 20)`

---

## ⚠️ One Critical Rule: Don't Use Character 0!

```c
// ✅ CORRECT
#define CHAR_SNAKE_HEAD  1
#define CHAR_SNAKE_BODY  2
#define CHAR_FOOD        3

// ❌ WRONG — index 0 acts like a C string terminator!
// Snake segments become invisible because the LCD stops printing early.
```

**Why?** In C, `0` marks the end of a string. Putting character `0` in the middle tells the LCD "stop here." Always use indices 1–7 for custom characters.

---

## 🗺️ Coordinate System (Quick Ref)

```
     x: 0   1   2   3  ...  13  14  15
Row 0: [  ][  ][  ][  ] ... [  ][  ][  ]
Row 1: [  ][  ][  ][  ] ... [  ][  ][  ]

16 columns × 2 rows = 32 positions total
```

---

## 📋 Quick Reference

| Setting | Value |
|---------|-------|
| Grid size | 16 × 2 = **32 cells** |
| Snake start length | **3** segments |
| Start speed | **300ms** per step |
| Fastest speed | **100ms** per step |
| Speed increases | Every **5** points, by **20ms** |
| Win at | **32** segments (full grid) |
| Walls? | **No** — snake wraps around |
| Self-collision? | **No** — snake passes through itself |
| Game over auto-reset | After **3 seconds** |
| Custom char indices | **1, 2, 3** (never 0!) |
| Input period | Every **50ms** (20 Hz) |
| Debounce | **100ms** |

---

*Built with STM32F407G + FreeRTOS + 16×2 LCD. For technical implementation details, see the source files in `App/` and `Core/Src/`.*
