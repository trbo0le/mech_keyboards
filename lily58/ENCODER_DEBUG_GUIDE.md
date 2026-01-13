# Rotary Encoder Debugging Guide

## Changes Made

### 1. Added Encoder Handler Function
**File:** `keymaps/trbo0le/keymap.c`

Added `encoder_update_user()` function to handle encoder events:
- **Index 0** (Left encoder): Volume Up/Down
- **Index 1** (Right encoder): Page Up/Down

### 2. Added Debug Counters
**File:** `keymaps/trbo0le/keymap.c`

Added tracking variables:
```c
static uint8_t encoder_0_count = 0;
static uint8_t encoder_1_count = 0;
```

### 3. Added OLED Debug Display
**File:** `keymaps/trbo0le/keymap.c`

Added `my_render_encoder_debug()` function that displays:
- Which side is master (L or R)
- Encoder 0 event count (E0)
- Encoder 1 event count (E1)

### 4. Enabled Encoder Sync
**File:** `rev1/keyboard.json`

Updated split transport configuration:
```json
"transport": {
    "sync": {
        "matrix_state": true,
        "encoders": true  // Added this line
    }
}
```

## How to Test

### Step 1: Flash the Firmware
```bash
qmk flash -kb splitkb/aurora/lily58/rev1 -km trbo0le
```

Flash both halves of the keyboard.

### Step 2: Check OLED Display (Left Side)

The left OLED should now show at the bottom:
```
Side:L
E0:X E1:Y
```

Where:
- `Side:L` = This is the master (left) side
- `E0:X` = Number of times encoder 0 (left) has been rotated (shows last digit)
- `E1:Y` = Number of times encoder 1 (right) has been rotated (shows last digit)

### Step 3: Test Each Encoder

**Test Left Encoder:**
1. Rotate the left encoder clockwise → Volume should increase
2. Rotate the left encoder counter-clockwise → Volume should decrease
3. Watch OLED: `E0:` value should increment

**Test Right Encoder:**
1. Rotate the right encoder clockwise → Page Down
2. Rotate the right encoder counter-clockwise → Page Up
3. Watch OLED: `E1:` value should increment

## Diagnosis Based on OLED Output

### Scenario 1: Both encoders increment E0 only
**Problem:** Both encoders are being detected as index 0
**Possible causes:**
- Handedness detection not working (F5 pin issue)
- Right half not properly identified
- Encoder sync not working

**Fix:** Check hardware connections for F5 pin

### Scenario 2: Both encoders increment E1 only
**Problem:** Both encoders are being detected as index 1
**Possible causes:**
- Handedness detection reversed
- Wrong half flashed as master

**Fix:** Re-flash with correct handedness

### Scenario 3: No counters increment when rotating
**Problem:** Encoders not detected at all
**Possible causes:**
- Encoder pins not connected (C6/D4 for left, F7/F6 for right)
- ENCODER_ENABLE not set in firmware
- Hardware issue with encoders

**Fix:** Check physical encoder connections

### Scenario 4: Left increments E0, Right increments E1 (CORRECT!)
**Success!** Encoders are working as expected:
- Left encoder → Volume control
- Right encoder → Page navigation

### Scenario 5: Both encoders perform the SAME function
**Problem:** Only one encoder index is being triggered
**Check OLED:**
- If only E0 increments: Right encoder is somehow mapped to index 0
- If only E1 increments: Left encoder is somehow mapped to index 1

**Fix:** Check encoder pin configuration in keyboard.json

## Hardware Pin Reference

From `rev1/keyboard.json`:

**Left Encoder (Index 0):**
- Pin A: C6
- Pin B: D4

**Right Encoder (Index 1):**
- Pin A: F7
- Pin B: F6

**Handedness Detection:** F5 pin
**Serial Communication:** D2 pin (TRRS cable)

## Additional Debugging

### Enable QMK Console Output
Add to `keymaps/trbo0le/config.h`:
```c
#define CONSOLE_ENABLE
```

Then add debug prints in the encoder function:
```c
dprintf("Encoder %d: %s\n", index, clockwise ? "CW" : "CCW");
```

View output with:
```bash
qmk console
```

## Next Steps if Still Not Working

1. **Verify handedness detection:**
   - Measure voltage on F5 pin
   - Left should read HIGH, right should read LOW (or vice versa)

2. **Check TRRS cable:**
   - Ensure all 4 conductors are connected
   - Try a different cable

3. **Test with default keymap:**
   ```bash
   qmk flash -kb splitkb/aurora/lily58/rev1 -km default
   ```
   If default works but trbo0le doesn't, issue is in custom keymap

4. **Verify encoder hardware:**
   - Test continuity of encoder pins to microcontroller
   - Swap encoders between halves to see if issue follows hardware

## Expected Behavior Summary

| Action | Expected Result | OLED Change |
|--------|----------------|-------------|
| Rotate left encoder CW | Volume Up | E0 increments |
| Rotate left encoder CCW | Volume Down | E0 increments |
| Rotate right encoder CW | Page Down | E1 increments |
| Rotate right encoder CCW | Page Up | E1 increments |
