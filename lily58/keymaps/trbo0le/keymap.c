#include QMK_KEYBOARD_H
#include "keymap_norwegian.h"

// Add support for 12 layers (3 sets of 4)
enum layers{
    _DEFAULT = 0,
    _LOWER = 1,
    _RAISE = 2,
    _ADJUST = 3,
    _NORWEGIAN = 4,
    _NORWEGIAN_LOWER = 5,
    _NORWEGIAN_RAISE = 6,
    _NORWEGIAN_ADJUST = 7,
    _GAMES_NORDIC = 8,
    _GAMES_NORDIC_LOWER = 9,
    _GAMES_NORDIC_RAISE = 10,
    _GAMES_NORDIC_ADJUST = 11
};
	
// Custom keycodes for tap-hold functionality
enum custom_keycodes {
    LOWER_TAP = SAFE_RANGE,  // Tap=cycle layer sets, Hold=lower layer
    RAISE_TAP,               // Tap=cycle layer sets, Hold=raise layer
    TOGGLE_LAYOUT            // Simple toggle for testing  
};

// Track which custom keys are held for dual-key combinations
static bool lower_held = false;
static bool raise_held = false;
static uint8_t current_layer_set = 0;  // 0=ANSI, 1=Norwegian, 2=Games Nordic
static uint16_t lower_timer = 0;
static uint16_t raise_timer = 0;
static uint16_t lower_tap_timer = 0;
static uint16_t raise_tap_timer = 0;
static bool lower_tap_pending = false;
static bool raise_tap_pending = false;

#define MAX_LAYER_SETS 3  // ANSI(0), Norwegian(1), Games Nordic(2)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // ANSI LAYERS 0-3 (unchanged)
    [0] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,
        KC_LSFT, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LCTL, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,     KC_LBRC,KC_RBRC, KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                                   KC_LALT, KC_LGUI, LOWER_TAP,KC_SPC, KC_ENT,  KC_BSPC, RAISE_TAP,KC_RALT
    ),
    [1] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, 
	KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12, 
	KC_GRV,  KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,                   KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_TILD,
	_______, _______, _______, _______, _______, _______, _______, _______, XXXXXXX, KC_UNDS, KC_PLUS, KC_LCBR,KC_RCBR, KC_PIPE, 
                                   _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [2] = LAYOUT(
        _______, _______, _______, _______, _______, _______,                   _______, _______, _______, _______, _______, _______, 
	KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    _______, 
	KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     XXXXXXX, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX, 
	KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, KC_PLUS, KC_MINS, KC_EQL,  KC_LBRC, KC_RBRC, KC_BSLS, 
	                           _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [3] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, RM_TOGG, RM_HUEU, RM_SATU, RM_VALU,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_NEXT, RM_HUED, RM_SATD, RM_VALD,
                                   _______, _______, _______, _______, _______, _______, _______, _______
    ),

    // PURE NORWEGIAN LAYERS 4-7
    [4] = LAYOUT( // Norwegian base layer
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    NO_PLUS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    NO_ARNG,
        KC_LSFT, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L,    NO_OSTR, NO_AE,
        KC_LCTL, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    NO_QUOT, NO_BSLS,KC_N,    KC_M,    KC_COMM, KC_DOT,  NO_MINS, KC_RSFT,
                                   KC_LALT, KC_LGUI, LOWER_TAP,KC_SPC, KC_ENT, KC_BSPC, RAISE_TAP,KC_RALT
    ),
    [5] = LAYOUT( // Norwegian symbols layer
        NO_PIPE, NO_EXLM, NO_DQUO, NO_HASH, NO_CURR, KC_PERC,                   NO_AMPR, NO_SLSH, NO_LPRN, NO_RPRN, NO_EQL,  NO_QUES,
        KC_F1,   NO_AT,   KC_F2,   NO_DLR,  NO_EURO, KC_F3,                     NO_LCBR, NO_RCBR, NO_LBRC, NO_RBRC, NO_ACUT, NO_GRV,
        KC_GRV,  NO_LABK, NO_RABK, NO_CIRC, NO_TILD, KC_F4,                     NO_QUOT, NO_ASTR, KC_COLN, KC_SCLN, KC_F5,   KC_F6,
        _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  _______, _______, KC_F12,  KC_UNDS, NO_PLUS, KC_LCBR, KC_RCBR, _______,
                                   _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [6] = LAYOUT( // Norwegian F-keys + Vim layer
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                    KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,
        KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,                   KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,
        _______, _______, _______, _______, _______, _______,                   KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_HOME, KC_END,
        _______, _______, _______, _______, _______, _______, _______, _______, KC_PGDN, KC_PGUP, KC_DEL,  KC_INS,  _______, _______,
                                   _______, _______, _______,  _______, _______, _______, _______, _______
    ),
    [7] = LAYOUT( // Norwegian adjust layer
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, RM_TOGG, RM_HUEU, RM_SATU, RM_VALU,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_NEXT, RM_HUED, RM_SATD, RM_VALD,
                                   _______, _______, _______, _______, _______, _______, _______, _______
    ),

    // GAMES NORDIC LAYERS 8-11 (your original Nordic layers moved here)
    [8] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                     KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    NO_PLUS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                     KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    NO_ARNG,
        KC_LSFT, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                     KC_H,    KC_J,    KC_K,    KC_L,    NO_OSTR, NO_AE,
        KC_LCTL, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    NO_ARNG, NO_QUOT,KC_N,    KC_M,    KC_COMM, KC_DOT,  NO_MINS, KC_RSFT,
                                   KC_LALT, KC_LGUI, LOWER_TAP,KC_SPC, KC_ENT, KC_BSPC, RAISE_TAP,KC_RALT
    ),
    [9] = LAYOUT(
        NO_BSLS, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,                      KC_F6,   NO_LABK, NO_RABK, _______, KC_PIPE, NO_QUOT,
        KC_DOT,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,
        NO_GRV,  KC_EXLM, NO_AT,   NO_HASH, NO_CURR, KC_PERC,                   NO_CIRC, NO_AMPR, NO_ASTR, NO_LPRN, NO_RPRN, NO_TILD,
        KC_I,    KC_M,    KC_P,    KC_PGUP, KC_PGDN, KC_ENT,  KC_Y,    _______, NO_QUOT, NO_UNDS, NO_PLUS, NO_EQL,  NO_RCBR, NO_BSLS,
                                   KC_K,    KC_N,    _______, _______, _______, _______, _______, _______
    ),
    [10] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   _______, _______, _______, _______, _______, _______,
        NO_TILD, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    _______,
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     NO_DQUO, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XXXXXXX,
        KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, NO_AT,   NO_LABK, NO_RABK, NO_LBRC, NO_RBRC, NO_BSLS,
                                   _______, _______, _______, _______, _______, _______, _______, _______
    ),
    [11] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,                   XXXXXXX, XXXXXXX, RM_TOGG, RM_HUEU, RM_SATU, RM_VALU,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RM_NEXT, RM_HUED, RM_SATD, RM_VALD,  			
                                   _______, _______, _______, _______, _______, _______, _______, _______
    )
};

// Custom key processing for tap-hold functionality
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LOWER_TAP:
            if (record->event.pressed) {
                // Key pressed - start timer
                lower_timer = timer_read();
                lower_held = true;
            } else {
                // Key released - check if it was a tap or hold
                if (timer_elapsed(lower_timer) < 90) {
                    // Short press = tap, hold time to activate hold layer 
                    if (lower_tap_pending && timer_elapsed(lower_tap_timer) < 300) {
                        // Double tap detected - decrease layer set time within to do two taps 
                        lower_tap_pending = false;
                        if (current_layer_set > 0) {
                            current_layer_set--;
                        } else {
                            current_layer_set = MAX_LAYER_SETS - 1;  // Wrap to highest set
                        }
                        layer_move(current_layer_set * 4);  // Move to base of new set
                    } else {
                        // First tap - start waiting for second tap
                        lower_tap_pending = true;
                        lower_tap_timer = timer_read();
                    }
                }
                // Release hold layers
                lower_held = false;
                layer_off(1);   layer_off(5);   layer_off(9);   // Lower layers
                if (!raise_held) {
                    layer_off(3);   layer_off(7);   layer_off(11);  // Adjust layers
                }
            }
            return false;
            
        case RAISE_TAP:
            if (record->event.pressed) {
                // Key pressed - start timer
                raise_timer = timer_read();
                raise_held = true;
            } else {
                // Key released - check if it was a tap or hold
                if (timer_elapsed(raise_timer) < 90) {
                    // Short press = tap
                    if (raise_tap_pending && timer_elapsed(raise_tap_timer) < 300) {
                        // Double tap detected - increase layer set
                        raise_tap_pending = false;
                        if (current_layer_set < MAX_LAYER_SETS - 1) {
                            current_layer_set++;
                        } else {
                            current_layer_set = 0;  // Wrap to lowest set
                        }
                        layer_move(current_layer_set * 4);  // Move to base of new set
                    } else {
                        // First tap - start waiting for second tap
                        raise_tap_pending = true;
                        raise_tap_timer = timer_read();
                    }
                }
                // Release hold layers
                raise_held = false;
                layer_off(2);   layer_off(6);   layer_off(10);  // Raise layers
                if (!lower_held) {
                    layer_off(3);   layer_off(7);   layer_off(11);  // Adjust layers
                }
            }
            return false;
            
        case TOGGLE_LAYOUT:
            if (record->event.pressed) {
                // Cycle through layer sets
                current_layer_set = (current_layer_set + 1) % MAX_LAYER_SETS;
                layer_move(current_layer_set * 4);
            }
            return false;
    }
    return true;
}

// Matrix scan function to handle hold detection and tap timeouts
void matrix_scan_user(void) {
    // Check if lower key has been held long enough to activate lower layer
    if (lower_held && timer_elapsed(lower_timer) > 90) {
        uint8_t lower_layer = (current_layer_set * 4) + 1;  // Base + 1 = lower layer
        layer_on(lower_layer);
        if (raise_held && timer_elapsed(raise_timer) > 90) {
            uint8_t adjust_layer = (current_layer_set * 4) + 3;  // Base + 3 = adjust layer
            layer_on(adjust_layer);
        }
    }
    
    // Check if raise key has been held long enough to activate raise layer
    if (raise_held && timer_elapsed(raise_timer) > 90) {
        uint8_t raise_layer = (current_layer_set * 4) + 2;  // Base + 2 = raise layer
        layer_on(raise_layer);
        if (lower_held && timer_elapsed(lower_timer) > 90) {
            uint8_t adjust_layer = (current_layer_set * 4) + 3;  // Base + 3 = adjust layer
            layer_on(adjust_layer);
        }
    }
    
    // Clear pending taps if timeout exceeded
    if (lower_tap_pending && timer_elapsed(lower_tap_timer) > 300) {
        lower_tap_pending = false;
    }
    if (raise_tap_pending && timer_elapsed(raise_tap_timer) > 300) {
        raise_tap_pending = false;
    }
}

#ifdef OLED_ENABLE

// OLED functionality 
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

void my_render_space(void) {
    oled_write_P(PSTR("     "), false);
}

void my_render_mod_status_gui_alt(uint8_t modifiers) {
    static const char PROGMEM gui_off_1[] = {0x85, 0x86, 0};
    static const char PROGMEM gui_off_2[] = {0xa5, 0xa6, 0};
    static const char PROGMEM gui_on_1[] = {0x8d, 0x8e, 0};
    static const char PROGMEM gui_on_2[] = {0xad, 0xae, 0};	

    static const char PROGMEM alt_off_1[] = {0x87, 0x88, 0};
    static const char PROGMEM alt_off_2[] = {0xa7, 0xa8, 0};
    static const char PROGMEM alt_on_1[] = {0x8f, 0x90, 0};
    static const char PROGMEM alt_on_2[] = {0xaf, 0xb0, 0};

    static const char PROGMEM off_off_1[] = {0xc5, 0};
    static const char PROGMEM off_off_2[] = {0xc6, 0};
    static const char PROGMEM on_off_1[] = {0xc7, 0};
    static const char PROGMEM on_off_2[] = {0xc8, 0};
    static const char PROGMEM off_on_1[] = {0xc9, 0};
    static const char PROGMEM off_on_2[] = {0xca, 0};
    static const char PROGMEM on_on_1[] = {0xcb, 0};
    static const char PROGMEM on_on_2[] = {0xcc, 0};

    if(modifiers & MOD_MASK_GUI) {
        oled_write_P(gui_on_1, false);
    } else {
        oled_write_P(gui_off_1, false);
    }

    if ((modifiers & MOD_MASK_GUI) && (modifiers & MOD_MASK_ALT)) {
        oled_write_P(on_on_1, false);
    } else if(modifiers & MOD_MASK_GUI) {
        oled_write_P(on_off_1, false);
    } else if(modifiers & MOD_MASK_ALT) {
        oled_write_P(off_on_1, false);
    } else {
        oled_write_P(off_off_1, false);
    }

    if(modifiers & MOD_MASK_ALT) {
        oled_write_P(alt_on_1, false);
    } else {
        oled_write_P(alt_off_1, false);
    }

    if(modifiers & MOD_MASK_GUI) {
        oled_write_P(gui_on_2, false);
    } else {
        oled_write_P(gui_off_2, false);
    }

    if ((modifiers & MOD_MASK_GUI) && (modifiers & MOD_MASK_ALT)) {
        oled_write_P(on_on_2, false);
    } else if(modifiers & MOD_MASK_GUI) {
        oled_write_P(on_off_2, false);
    } else if(modifiers & MOD_MASK_ALT) {
        oled_write_P(off_on_2, false);
    } else {
        oled_write_P(off_off_2, false);
    }

    if(modifiers & MOD_MASK_ALT) {
        oled_write_P(alt_on_2, false);
    } else {
        oled_write_P(alt_off_2, false);
    }
}

void my_render_mod_status_ctrl_shift(uint8_t modifiers) {
    static const char PROGMEM ctrl_off_1[] = {0x89, 0x8a, 0};
    static const char PROGMEM ctrl_off_2[] = {0xa9, 0xaa, 0};
    static const char PROGMEM ctrl_on_1[] = {0x91, 0x92, 0};
    static const char PROGMEM ctrl_on_2[] = {0xb1, 0xb2, 0};

    static const char PROGMEM shift_off_1[] = {0x8b, 0x8c, 0};
    static const char PROGMEM shift_off_2[] = {0xab, 0xac, 0};
    static const char PROGMEM shift_on_1[] = {0xcd, 0xce, 0};
    static const char PROGMEM shift_on_2[] = {0xcf, 0xd0, 0};

    static const char PROGMEM off_off_1[] = {0xc5, 0};
    static const char PROGMEM off_off_2[] = {0xc6, 0};
    static const char PROGMEM on_off_1[] = {0xc7, 0};
    static const char PROGMEM on_off_2[] = {0xc8, 0};
    static const char PROGMEM off_on_1[] = {0xc9, 0};
    static const char PROGMEM off_on_2[] = {0xca, 0};
    static const char PROGMEM on_on_1[] = {0xcb, 0};
    static const char PROGMEM on_on_2[] = {0xcc, 0};

    if(modifiers & MOD_MASK_CTRL) {
        oled_write_P(ctrl_on_1, false);
    } else {
        oled_write_P(ctrl_off_1, false);
    }

    if ((modifiers & MOD_MASK_CTRL) && (modifiers & MOD_MASK_SHIFT)) {
        oled_write_P(on_on_1, false);
    } else if(modifiers & MOD_MASK_CTRL) {
        oled_write_P(on_off_1, false);
    } else if(modifiers & MOD_MASK_SHIFT) {
        oled_write_P(off_on_1, false);
    } else {
        oled_write_P(off_off_1, false);
    }

    if(modifiers & MOD_MASK_SHIFT) {
        oled_write_P(shift_on_1, false);
    } else {
        oled_write_P(shift_off_1, false);
    }

    if(modifiers & MOD_MASK_CTRL) {
        oled_write_P(ctrl_on_2, false);
    } else {
        oled_write_P(ctrl_off_2, false);
    }

    if ((modifiers & MOD_MASK_CTRL) && (modifiers & MOD_MASK_SHIFT)) {
        oled_write_P(on_on_2, false);
    } else if(modifiers & MOD_MASK_CTRL) {
        oled_write_P(on_off_2, false);
    } else if(modifiers & MOD_MASK_SHIFT) {
        oled_write_P(off_on_2, false);
    } else {
        oled_write_P(off_off_2, false);
    }

    if(modifiers & MOD_MASK_SHIFT) {
        oled_write_P(shift_on_2, false);
    } else {
        oled_write_P(shift_off_2, false);
    }
}

void my_render_logo(void) {
    static const char PROGMEM aurora_logo[] = {
        0x00, 0x00, 0x00, 0xe0, 0x00, 0xf8, 0xc0, 0xf8, 0xe0, 0xc0, 0xfc, 0x00, 0x7e, 0x18, 0x00, 0x80,
        0x00, 0x02, 0x80, 0xf0, 0x00, 0xc0, 0x80, 0xf8, 0xc0, 0xe0, 0x70, 0x60, 0x3c, 0x38, 0x3c, 0x1c,
        0x00, 0x3f, 0x0c, 0x0f, 0x1f, 0x03, 0x07, 0x01, 0xc3, 0x00, 0xe0, 0x80, 0x00, 0xe0, 0x80, 0xf8,
        0x80, 0xc0, 0xf7, 0xc7, 0x6f, 0x7b, 0x39, 0x30, 0x00, 0x80, 0x00, 0xc0, 0x00, 0xc0, 0xc2, 0xe0,
        0x00, 0x40, 0x38, 0x30, 0x38, 0x1e, 0x18, 0x1e, 0x0f, 0x0c, 0x07, 0x07, 0x07, 0x03, 0x03, 0x21,
        0x21, 0x31, 0x30, 0x18, 0x18, 0x1c, 0x08, 0x0c, 0x0e, 0x07, 0x06, 0x07, 0x03, 0xc3, 0x03, 0x01,
        0x4c, 0xcc, 0xc2, 0xc2, 0x41, 0x49, 0x09, 0x2b, 0x2a, 0x6a, 0x6e, 0x24, 0x24, 0x04, 0x92, 0x92,
        0xb1, 0xf1, 0xf1, 0xf2, 0xe6, 0xa4, 0xa4, 0x04, 0x04, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28
    };
    oled_write_raw_P(aurora_logo, sizeof(aurora_logo));
    oled_set_cursor(0, 4);
}

void my_render_logo_text(void) {
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    
    // Show layer number and layout type
    oled_write_char('L', false);
    oled_write_char('0' + layer, false);  // Show actual layer number for debugging
    oled_write_char(' ', false);
    
    // Show current layer set
    switch (current_layer_set) {
        case 0:
            oled_write_P(PSTR("EN"), false);  // ANSI/English
            break;
        case 1:
            oled_write_P(PSTR("NO"), false);  // Pure Norwegian
            break;
        case 2:
            oled_write_P(PSTR("GM"), false);  // Games Nordic
            break;
        default:
            oled_write_char('0' + current_layer_set, false);  // Future: show set number
            oled_write_char('?', false);
            break;
    }
}

void my_render_kb_LED_state(void) {
    led_t led_usb_state = host_keyboard_led_state();
    oled_write_P(led_usb_state.num_lock ? PSTR("N ") : PSTR("  "), false);
    oled_write_P(led_usb_state.caps_lock ? PSTR("C ") : PSTR("  "), false);
    oled_write_P(led_usb_state.scroll_lock ? PSTR("S ") : PSTR("  "), false);
}

void my_render_layer_state(void) {
    static const char PROGMEM default_layer[] = {
        0x20, 0x94, 0x95, 0x96, 0x20,
        0x20, 0xb4, 0xb5, 0xb6, 0x20,
        0x20, 0xd4, 0xd5, 0xd6, 0x20, 0};
    static const char PROGMEM raise_layer[] = {
        0x20, 0x97, 0x98, 0x99, 0x20,
        0x20, 0xb7, 0xb8, 0xb9, 0x20,
        0x20, 0xd7, 0xd8, 0xd9, 0x20, 0};
    static const char PROGMEM lower_layer[] = {
        0x20, 0x9a, 0x9b, 0x9c, 0x20,
        0x20, 0xba, 0xbb, 0xbc, 0x20,
        0x20, 0xda, 0xdb, 0xdc, 0x20, 0};
    static const char PROGMEM adjust_layer[] = {
        0x20, 0x9d, 0x9e, 0x9f, 0x20,
        0x20, 0xbd, 0xbe, 0xbf, 0x20,
        0x20, 0xdd, 0xde, 0xdf, 0x20, 0};

    switch (get_highest_layer(layer_state | default_layer_state)) {
        case 1: // Lower
        case 5: // Norwegian Lower  
        case 9: // Games Nordic Lower
            oled_write_P(lower_layer, false);
            break;
        case 2: // Raise  
        case 6: // Norwegian Raise
        case 10: // Games Nordic Raise
            oled_write_P(raise_layer, false);
            break;
        case 3: // Adjust
        case 7: // Norwegian Adjust
        case 11: // Games Nordic Adjust
            oled_write_P(adjust_layer, false);
            break;
        default: // All base layers
            oled_write_P(default_layer, false);
    }
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        // Left side - Aurora display with modified text
        my_render_logo();
        my_render_logo_text();
        my_render_space();
        my_render_layer_state();
        my_render_space();
        my_render_mod_status_gui_alt(get_mods()|get_oneshot_mods());
        my_render_mod_status_ctrl_shift(get_mods()|get_oneshot_mods());
        my_render_kb_LED_state();
    } else {
        // Right side - original Aurora art
        static const char PROGMEM aurora_art[] = {
            0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x08, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x40,
            0xe0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x80,
            0xc0, 0x80, 0x00, 0x0, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x80, 0x00, 0xf0, 0x00, 0x00, 0xc0,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x81, 0x00, 0xc0, 0x00, 0xfe, 0x00, 0xfc, 0x00, 0xff, 0x20, 0xff, 0xf0, 0x0f, 0xf0, 0x00, 0xff,
            0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0xf8, 0x00, 0x00, 0xf8,
            0xff, 0x10, 0xff, 0x84, 0xff, 0x60, 0xff, 0x36, 0xff, 0x0f, 0xff, 0x3f, 0x00, 0x5f, 0x00, 0x05,
            0x80, 0x00, 0x80, 0x00, 0xc0, 0x38, 0x00, 0xec, 0xf0, 0x00, 0xfb, 0x80, 0xff, 0xf0, 0xff, 0xef,
            0xff, 0xe8, 0xff, 0x03, 0xff, 0x0c, 0xff, 0x00, 0xff, 0x00, 0x03, 0x00, 0x00, 0xf8, 0x00, 0x80,
            0xff, 0x20, 0xff, 0xd0, 0xff, 0xe0, 0xfe, 0xf8, 0xff, 0xfc, 0xff, 0xff, 0x0f, 0xff, 0x01, 0x3f,
            0xff, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0xfe, 0x80, 0xfe, 0x00, 0xc0, 0xff, 0xc4, 0xfb,
            0xff, 0xfe, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0x07, 0xff, 0x03, 0x3f, 0x00, 0x0f, 0xc0, 0x00,
            0x00, 0x00, 0xb8, 0x00, 0xff, 0x40, 0xbe, 0xf0, 0xff, 0xf1, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
            0x1f, 0xff, 0x67, 0x00, 0xef, 0x00, 0x1f, 0x00, 0x00, 0x07, 0x00, 0x00, 0xe0, 0x00, 0xff, 0xf0,
            0xff, 0x88, 0xff, 0xc4, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8f, 0x7f, 0x0f, 0xff,
            0x00, 0x07, 0xfe, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xc0, 0x3f, 0xf8, 0xe7, 0xff,
            0xff, 0xff, 0xff, 0x1f, 0xff, 0xff, 0x1f, 0x3f, 0x01, 0xff, 0x0b, 0x00, 0xff, 0x00, 0x00, 0x05,
            0x00, 0x00, 0x00, 0xe0, 0x00, 0xf8, 0x60, 0x80, 0xfe, 0xe3, 0xfc, 0xff, 0x1e, 0xff, 0xff, 0x23,
            0xff, 0x09, 0xff, 0x20, 0x00, 0x3f, 0x02, 0x00, 0x00, 0x0f, 0x00, 0x40, 0x00, 0xc0, 0x00, 0xfc,
            0xe0, 0xfc, 0xf0, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0x0f, 0xff, 0x07, 0x1f, 0x00, 0x01, 0x0f, 0x00,
            0x0f, 0x00, 0x81, 0x70, 0x0c, 0xf0, 0x80, 0x00, 0x00, 0xe4, 0xf8, 0xe6, 0x70, 0x3f, 0xcf, 0xff,
            0x1f, 0xff, 0x48, 0xff, 0x0f, 0x00, 0x07, 0x00, 0x00, 0x43, 0x60, 0xf8, 0xf0, 0xfe, 0x38, 0xfe,
            0x00, 0xfc, 0x03, 0x00, 0xc8, 0x72, 0xcf, 0xfc, 0x00, 0x03, 0x0f, 0x01, 0xe0, 0x1c, 0xe0, 0x03,
            0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x06, 0xf9, 0x00, 0x03, 0x00, 0x07,
            0xff, 0x00, 0x10, 0x12, 0xc9, 0xf0, 0xcf, 0xb4, 0x7f, 0x80, 0xe0, 0x1e, 0x01, 0x40, 0x65, 0x5e,
            0xe0, 0x00, 0x00, 0xf0, 0x0c, 0xf0, 0x00, 0x80, 0x7e, 0x01, 0x80, 0x93, 0xfc, 0xc0, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x89, 0x18, 0x2c, 0x46, 0x00, 0x07, 0x21, 0x10, 0x10, 0x80, 0x09, 0x13,
            0x31, 0xbf, 0xff, 0x00, 0x08, 0x1a, 0xf7, 0x0f, 0x00, 0x00, 0x44, 0x45, 0x34, 0xbf, 0xb8, 0x00,
            0x10, 0xf0, 0x08, 0xf4, 0x18, 0x11, 0xfc, 0x18, 0xfb, 0x0e, 0x10, 0xf8, 0x04, 0xf8, 0x10, 0x20,
            0x18, 0x09, 0xff, 0x0c, 0xea, 0x1f, 0x28, 0x60, 0x30, 0xf8, 0x20, 0xc0, 0x42, 0x33, 0x21, 0x00
        };
        oled_write_raw_P(aurora_art, sizeof(aurora_art));
    }
    return false;  // Prevent keyboard-level OLED code from running
}

#endif
