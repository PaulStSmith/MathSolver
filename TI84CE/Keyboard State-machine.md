# TI84 CE Keyboard State Machine

## Mode Definitions

```c
typedef enum {
    KB_MODE_NORMAL           = 0,    // 0000
    KB_MODE_2ND              = 1,    // 0001
    KB_MODE_ALPHA            = 2,    // 0010
    KB_MODE_LOWER            = 4,    // 0100
    KB_MODE_LOCK             = 8,    // 1000
    KB_MODE_ALPHA_LOWER      = 6,    // 0110 (ALPHA | LOWER)
    KB_MODE_ALPHA_LOCK       = 10,   // 1010 (ALPHA | LOCKED)
    KB_MODE_ALPHA_LOWER_LOCK = 14    // 1110 (ALPHA | LOWER | LOCKED)
} KeyboardMode;
```

## State Machine Overview

The keyboard state machine manages different input modes on the TI84 CE calculator. It tracks the current mode using bit flags and transitions between states based on key presses.

## States Description

1. **Normal Mode (0)**: Default state, no special modes active
2. **2nd Mode (1)**: 2nd function key is active, giving access to the yellow functions
3. **Alpha Mode (2)**: Alpha mode is active, entering uppercase letters
4. **Alpha+Lower Mode (6)**: Alpha lowercase mode, entering lowercase letters
5. **2nd+Lock Mode (9)**: 2nd function locked mode (not explicitly defined but possible)
6. **Alpha+Lock Mode (10)**: Alpha mode locked, uppercase letters locked
7. **Alpha+Lower+Lock Mode (14)**: Alpha lowercase mode locked

## State Transitions

### When 2ND key is pressed:
- Toggles the 2ND mode bit flag (`current_mode ^= KB_MODE_2ND`)
- If 2ND is already active, it deactivates; if inactive, it activates

### When ALPHA key is pressed:
1. If a mode other than ALPHA is active:
   - ALPHA mode is activated
2. If ALPHA mode is already active:
   - ALPHA_LOWER mode is activated (adds LOWER flag)
3. If LOWER mode is active:
   - Both LOWER and LOCK flags are cleared, returning to normal mode
4. If 2ND mode is active when ALPHA is pressed:
   - LOCK mode is set
   - 2ND mode is always cleared when ALPHA is pressed

## State Transition Flow Diagram

```
┌───────────────┐
│  Normal Mode  │
│      (0)      │
└───────┬───────┘
        │
  ┌─────┴─────┐
  │           │
  ▼           ▼
┌───────┐   ┌───────┐
│ 2nd   │   │ Alpha │
│ Mode  │   │ Mode  │
│ (1)   │   │ (2)   │
└───┬───┘   └───┬───┘
    │           │
    │           │ Press ALPHA
    │           ▼
    │       ┌───────────────┐
    │       │ Alpha+Lower   │
    │       │ Mode (6)      │
    │       └───────┬───────┘
    │               │
    │               │ Press ALPHA
    │               ▼
    │           ┌───────────────┐
    │           │  Normal Mode  │
    │           │      (0)      │
    │           └───────────────┘
    │
    │ Press ALPHA
    ▼
┌───────────────┐
│ Alpha+Lock    │
│ Mode (10)     │
└───────┬───────┘
        │
        │ Press ALPHA
        ▼
┌───────────────────────┐
│ Alpha+Lower+Lock      │
│ Mode (14)             │
└───────────┬───────────┘
            │
            │ Press ALPHA
            ▼
┌───────────────┐
│  Normal Mode  │
│      (0)      │
└───────────────┘
```

## State Transition Table

| Current State           | Press 2ND        | Press ALPHA                |
|-------------------------|------------------|----------------------------|
| Normal (0)              | 2nd (1)          | Alpha (2)                  |
| 2nd (1)                 | Normal (0)       | Alpha+Lock (10)            |
| Alpha (2)               | Alpha+2nd (3)*   | Alpha+Lower (6)            |
| Alpha+Lower (6)         | Alpha+Lower+2nd* | Normal (0)                 |
| Alpha+Lock (10)         | *                | Alpha+Lower+Lock (14)      |
| Alpha+Lower+Lock (14)   | *                | Normal (0)                 |

\* These states are valid but not explicitly handled in the provided code snippet.

## Code Analysis

```c
if (key == KEY_2ND) {
    // 2nd key pressed - toggle 2nd mode
    current_mode ^= KB_MODE_2ND;
    handled = true;
} 
else if (key == KEY_ALPHA) {
    // Alpha key pressed
    if (current_mode & ~KB_MODE_ALPHA) {
        current_mode |= KB_MODE_ALPHA;                      // Set alpha mode
    } else if (current_mode & KB_MODE_ALPHA) {
        // If we are in alpha mode, change to alpha lower
        current_mode |= KB_MODE_ALPHA_LOWER;                // Set alpha lower mode
    } else if (current_mode & KB_MODE_LOWER) {
        // If we are in lower mode, return to normal mode
        current_mode = current_mode & ~KB_MODE_ALPHA_LOWER; // Remove alpha lower mode
        current_mode = current_mode & ~KB_MODE_LOCK;        // Remove locked mode
    }
    
    if (current_mode & KB_MODE_2ND) {
        current_mode |= KB_MODE_LOCK;                       // Set locked mode
    } 
    current_mode = current_mode & ~KB_MODE_2ND;             // Remove 2nd mode
    handled = true;
}
```

### Key Observations

1. The 2ND key simply toggles the 2ND mode on/off
2. The ALPHA key has complex behavior:
   - In normal mode: Activates ALPHA mode
   - In ALPHA mode: Changes to ALPHA+LOWER mode
   - In LOWER mode: Returns to normal mode
   - When 2ND is active: Also sets LOCK mode
   - Always clears 2ND mode

3. The state machine has a potential issue in the ALPHA key logic:
   - The condition `else if (current_mode & KB_MODE_LOWER)` seems to assume ALPHA is not set, but LOWER is rarely set without ALPHA

## Implementation Notes

1. The current implementation uses bitwise operations to manage state:
   - `|=` to set flags
   - `&= ~` to clear flags
   - `^=` to toggle flags

2. The `KB_MODE_ALPHA_LOWER` (6) is a composite state combining `KB_MODE_ALPHA` (2) and `KB_MODE_LOWER` (4)

3. The LOCK mode (8) is a modifier that can be combined with other modes, primarily activated when transitioning from 2ND to ALPHA