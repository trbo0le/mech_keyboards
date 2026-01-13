# Rotary Encoder Fix - Steps Taken

**Date:** January 13, 2026
**Issue:** Both halves of split keyboard have rotary encoders that appear to have the same index/GPIO and perform the same function instead of their individual functions.

---

## Problem Diagnosis

### Initial Issue
The user reported that both rotary encoders on the split keyboard (left and right halves) appeared to have the same index/GPIO and were performing the same function instead of their individual assigned functions.

### Root Cause Analysis
After analyzing the codebase, we identified the issue:

1. **Hardware configuration was correct** in `rev1/keyboard.json`:
   - Left encoder: pins C6 (pin_a) and D4 (pin_b) → Index 0
   - Right encoder: pins F7 (pin_a) and F6 (pin_b) → Index 1

2. **Software handler was missing**: The custom keymap `keymaps/trbo0le/keymap.c` did NOT have an `encoder_update_user()` function, so it was falling through to the keyboard-level handler in `lily58.c`

3. **Split sync not enabled**: Encoder synchronization between split halves was not enabled in the configuration

## Changes Implemented

### 1. Added Encoder Handler Function
**File:** `lily58/keymaps/trbo0le/keymap.c` (lines 213-241)

Created `encoder_update_user()` function with:
- Index 0 (left encoder): Volume Up/Down (`KC_VOLU`/`KC_VOLD`)
- Index 1 (right encoder): Page Up/Down (`KC_PGDN`/`KC_PGUP`)
- Debug counters to track encoder events
- Returns `false` to prevent keyboard-level handler from running

### 2. Added Debug Variables (lines 34-36)

Added tracking counters at the top of the file:
```c
static uint8_t encoder_0_count = 0;
static uint8_t encoder_1_count = 0;
```

These counters increment each time an encoder event is detected, allowing us to see which encoder index is being triggered.

### 3. Added OLED Debug Display Function (lines 392-409)

Created `my_render_encoder_debug()` function that displays:
- **Side identification**: Shows "L" for master/left side, "R" for right side
- **Encoder 0 count**: Shows last digit of left encoder rotation count
- **Encoder 1 count**: Shows last digit of right encoder rotation count

### 4. Integrated Debug Display into OLED ([keymap.c:447](d:\projects\turbo-keyboard\mech_keyboards\lily58\keymaps\trbo0le\keymap.c#L447))

Added call to `my_render_encoder_debug()` in the OLED task to display encoder debug info on the master (left) side.

### 5. **Enabled Encoder Sync** ([keyboard.json:118](d:\projects\turbo-keyboard\mech_keyboards\lily58\rev1\keyboard.json#L118))

Updated the split transport configuration to include encoder synchronization:
```json
"transport": {
    "sync": {
        "matrix_state": true,
        "encoders": true  // Added this line
    }
}
```

### 6. **Created Debug Documentation**
Created [ENCODER_DEBUG_GUIDE.md](d:\projects\turbo-keyboard\mech_keyboards\lily58\ENCODER_DEBUG_GUIDE.md) with:
- Summary of all changes
- Testing procedures
- Diagnostic scenarios based on OLED output
- Hardware pin reference
- Troubleshooting steps

## Root Cause Analysis

### The Problem
Both rotary encoder halves were performing the same function (likely both acting as volume controls or both as page navigation).

### Why It Happened
1. **Missing User-Level Handler**: The custom keymap `trbo0le` didn't override the keyboard-level encoder handler
2. **No Encoder Sync**: The split keyboard configuration didn't have encoder sync enabled
3. **Possible Index Collision**: Both encoders may have been registering as the same index (likely index 0)

### Hardware Configuration (Was Already Correct)
The hardware pins were correctly configured in `rev1/keyboard.json`:
- **Left Encoder (Index 0)**: Pins C6 and D4
- **Right Encoder (Index 1)**: Pins F7 and F6

## Changes Made

### File 1: `lily58/keymaps/trbo0le/keymap.c`

#### Added Debug Variables (Lines 34-36)
```c
// Encoder debugging
static uint8_t encoder_0_count = 0;
static uint8_t encoder_1_count = 0;
```

#### Added Encoder Handler Function (Lines 213-241)
```c
#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    // Debug: Track encoder events
    if (index == 0) {
        encoder_0_count++;
    } else if (index == 1) {
        encoder_1_count++;
    }

    // Left encoder (index 0) - Volume control
    if (index == 0) {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    // Right encoder (index 1) - Page up/down
    else if (index == 1) {
        if (clockwise) {
            tap_code(KC_PGDN);
        } else {
            tap_code(KC_PGUP);
        }
    }

    return false;  // Don't call encoder_update_kb
}
#endif
```

This function intercepts encoder events at the user level and provides proper handling for each encoder based on its index.

### 3. **Added OLED Debug Display**

Added a new function `my_render_encoder_debug()` at [keymap.c:392-409](d:\projects\turbo-keyboard\mech_keyboards\lily58\keymaps\trbo0le\keymap.c#L392-L409) that displays:

```c
void my_render_encoder_debug(void) {
    // Show which side this is
    oled_write_P(PSTR("Side:"), false);
    if (is_keyboard_master()) {
        oled_write_P(PSTR("L"), false);
    } else {
        oled_write_P(PSTR("R"), false);
    }
    oled_write_P(PSTR("\n"), false);

    // Show encoder counts for debugging
    oled_write_P(PSTR("E0:"), false);
    oled_write_char('0' + (encoder_0_count % 10), false);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("E1:"), false);
    oled_write_char('0' + (encoder_1_count % 10), false);
    oled_write_P(PSTR("\n"), false);
}
```

This displays:
- Which keyboard half is the master (L/R)
- Count of encoder 0 events (last digit)
- Count of encoder 1 events (last digit)

### 3. **Updated OLED Display** ([keymaps/trbo0le/keymap.c:447](d:\projects\turbo-keyboard\mech_keyboards\lily58\keymaps\trbo0le\keymap.c#L447))
   - Added `my_render_encoder_debug()` call to display encoder debug info on left OLED

### 4. **Enabled Encoder Sync in Split Configuration** ([rev1/keyboard.json:118](d:\projects\turbo-keyboard\mech_keyboards\lily58\rev1\keyboard.json#L118))
   - Added `"encoders": true` to the transport sync configuration
   - Ensures encoder events are properly synchronized between split halves

## Files Modified

1. **`lily58/keymaps/trbo0le/keymap.c`**
   - Added encoder handler function
   - Added debug counters
   - Added OLED debug display function

2. **`lily58/rev1/keyboard.json`**
   - Enabled encoder synchronization in split transport config

3. **`lily58/ENCODER_DEBUG_GUIDE.md`** (NEW)
   - Comprehensive debugging and troubleshooting guide

## Problem Description

The user reported that both halves of their split keyboard's rotary encoders appeared to have the same index/GPIO and were performing the same function instead of their individual assigned functions.

## Root Cause Analysis

1. **Hardware configuration was correct** - Encoders properly defined with different pins:
   - Left encoder (index 0): pins C6 and D4
   - Right encoder (index 1): pins F7 and F6

2. **Software handler was missing** - The custom keymap `trbo0le` did not override the encoder behavior, falling back to the keyboard-level handler in `lily58.c`

3. **Encoder sync was not enabled** - The split transport configuration was not syncing encoder events between halves

## Solutions Implemented

### 1. Encoder Handler Function
Added `encoder_update_user()` to properly handle both encoders:
```c
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        // Left encoder - Volume control
        if (clockwise) tap_code(KC_VOLU);
        else tap_code(KC_VOLD);
    } else if (index == 1) {
        // Right encoder - Page up/down
        if (clockwise) tap_code(KC_PGDN);
        else tap_code(KC_PGUP);
    }
    return false;
}
```

### 2. Debug Counters and OLED Display
Added visual feedback to diagnose the issue:
- Encoder event counters (E0 and E1)
- Side identification (L or R)
- Display on left OLED for real-time monitoring

### 3. Enabled Encoder Sync
Updated `keyboard.json` to sync encoder events:
```json
"transport": {
    "sync": {
        "matrix_state": true,
        "encoders": true
    }
}
```

## Testing Instructions

1. Flash the firmware:
   ```bash
   qmk flash -kb splitkb/aurora/lily58/rev1 -km trbo0le
   ```

2. Observe the left OLED display:
   ```
   Side:L
   E0:X E1:Y
   ```

3. Test each encoder:
   - **Left encoder**: Should control volume (Up/Down)
   - **Right encoder**: Should control page navigation (PgUp/PgDn)
   - **OLED counters**: E0 should increment for left, E1 for right

## Expected Outcomes

- Left encoder rotates → E0 increments, volume changes
- Right encoder rotates → E1 increments, page changes
- Each encoder performs its unique function independently

## Troubleshooting

If both encoders still behave the same way, check the OLED display:
- **Both increment E0 only**: Handedness detection issue (F5 pin)
- **Both increment E1 only**: Handedness reversed
- **Neither increments**: Hardware connection issue
- **Correct increments but same function**: Check encoder pin mapping

## Files Modified

1. `lily58/keymaps/trbo0le/keymap.c` - Added encoder handler and debug code
2. `lily58/rev1/keyboard.json` - Enabled encoder sync
3. `lily58/ENCODER_DEBUG_GUIDE.md` - Created troubleshooting guide

## Date

2026-01-13
