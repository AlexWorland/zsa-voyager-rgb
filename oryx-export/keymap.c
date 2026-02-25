#include QMK_KEYBOARD_H
#include "version.h"
#include "oryx.h"
#include <string.h>
#define MOON_LED_LEVEL LED_LEVEL
#ifndef ZSA_SAFE_RANGE
#define ZSA_SAFE_RANGE SAFE_RANGE
#endif

enum custom_keycodes {
  RGB_SLD = ZSA_SAFE_RANGE,
  HSV_0_255_255,
  HSV_74_255_255,
  HSV_169_255_255,
  DRAG_SCROLL,
  TOGGLE_SCROLL,
  NAVIGATOR_INC_CPI,
  NAVIGATOR_DEC_CPI,
  NAVIGATOR_TURBO,
  NAVIGATOR_AIM,
  CYCLE_PALETTE
};



#define DUAL_FUNC_0 LT(6, KC_F16)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_voyager(
    DUAL_FUNC_0,    KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_MINUS,       
    CW_TOGG,        KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_BSLS,        
    MT(MOD_LSFT, KC_BSPC),KC_A,           KC_S,           KC_D,           KC_F,           KC_G,                                           KC_H,           KC_J,           KC_K,           KC_L,           KC_SCLN,        MT(MOD_RSFT, KC_QUOTE),
    KC_LEFT_GUI,    MT(MOD_LALT, KC_Z),KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         MT(MOD_RALT, KC_SLASH),KC_RIGHT_CTRL,  
                                                    LT(1, KC_ENTER),MT(MOD_LCTL, KC_TAB),                                MT(MOD_LSFT, KC_BSPC),LT(2, KC_SPACE)
  ),
  [1] = LAYOUT_voyager(
    KC_ESCAPE,      KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,         
    KC_GRAVE,       KC_EXLM,        KC_AT,          KC_HASH,        KC_DLR,         KC_PERC,                                        KC_7,           KC_8,           KC_9,           KC_MINUS,       KC_SLASH,       KC_F12,         
    KC_TRANSPARENT, KC_CIRC,        KC_AMPR,        KC_ASTR,        KC_LPRN,        KC_RPRN,                                        KC_4,           KC_5,           KC_6,           KC_PLUS,        KC_ASTR,        KC_BSPC,        
    KC_TRANSPARENT, KC_TRANSPARENT, KC_LBRC,        KC_RBRC,        KC_LCBR,        KC_RCBR,                                        KC_1,           KC_2,           KC_3,           KC_DOT,         KC_EQUAL,       KC_ENTER,       
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_0
  ),
  [2] = LAYOUT_voyager(
    RGB_TOG,        CYCLE_PALETTE,  RGB_MODE_FORWARD,RGB_SLD,        RGB_VAD,        RGB_VAI,                                        KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, QK_BOOT,        
    KC_TRANSPARENT, KC_TRANSPARENT, KC_AUDIO_VOL_DOWN,KC_AUDIO_VOL_UP,KC_AUDIO_MUTE,  KC_TRANSPARENT,                                 KC_PAGE_UP,     KC_HOME,        KC_UP,          KC_END,         KC_TRANSPARENT, KC_TRANSPARENT, 
    KC_TRANSPARENT, KC_MEDIA_PREV_TRACK,KC_MEDIA_NEXT_TRACK,KC_MEDIA_STOP,  KC_MEDIA_PLAY_PAUSE,KC_TRANSPARENT,                                 KC_PGDN,        KC_LEFT,        KC_DOWN,        KC_RIGHT,       KC_TRANSPARENT, KC_TRANSPARENT, 
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, HSV_0_255_255,  HSV_74_255_255, HSV_169_255_255,                                KC_TRANSPARENT, LCTL(LSFT(KC_TAB)),LCTL(KC_TAB),   KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, 
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
  [3] = LAYOUT_voyager(
    NAVIGATOR_DEC_CPI,NAVIGATOR_INC_CPI,KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, QK_LLCK,                                        KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, 
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_MS_BTN3,     TOGGLE_SCROLL,                                  KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, 
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_MS_BTN2,     KC_MS_BTN1,     DRAG_SCROLL,                                    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, 
    KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, KC_TRANSPARENT, 
                                                    KC_TRANSPARENT, KC_TRANSPARENT,                                 KC_TRANSPARENT, KC_TRANSPARENT
  ),
};






extern bool set_scrolling;
extern bool navigator_turbo;
extern bool navigator_aim;
void pointing_device_init_user(void) {
  set_auto_mouse_enable(true);
}

bool is_mouse_record_user(uint16_t keycode, keyrecord_t* record) {
  // Treat all keys as mouse keys when in the automouse layer so that any key set resets the timeout without leaving the layer.
  if (!layer_state_is(AUTO_MOUSE_TARGET_LAYER)){
    // When depressing a mouse key with a LT key at the same time, the mouse key tracker is not decremented.
    // This is a workaround to fix that
    if (IS_MOUSE_KEYCODE(keycode) && !record->event.pressed) {
      return true;
    }
    return false;
  }
  else {
    return true;
  }
}

/* ─── Toggle State Tracking ─── */
#define NUM_PALETTES 4
#define PAL_VIBRANT 0
#define PAL_DARK    1
#define PAL_LIGHT   2
#define PAL_PASTEL  3

uint8_t current_palette = PAL_VIBRANT;
bool mute_active = false;
bool caps_word_active_state = false;

#define CUSTOM_EVT_STATE 0xFD

static void send_toggle_states(void) {
    if (!rawhid_state.paired) return;
    uint8_t data[RAW_EPSIZE];
    memset(data, 0, RAW_EPSIZE);
    data[0] = CUSTOM_EVT_STATE;
    data[1] = set_scrolling ? 1 : 0;
    data[2] = mute_active ? 1 : 0;
    data[3] = caps_word_active_state ? 1 : 0;
    data[4] = current_palette;
    raw_hid_send(data, RAW_EPSIZE);
}

void caps_word_set_user(bool active) {
    caps_word_active_state = active;
    send_toggle_states();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    send_toggle_states();
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
  case QK_MODS ... QK_MODS_MAX:
    // Mouse and consumer keys (volume, media) with modifiers work inconsistently across operating systems,
    // this makes sure that modifiers are always applied to the key that was pressed.
    if (IS_MOUSE_KEYCODE(QK_MODS_GET_BASIC_KEYCODE(keycode)) || IS_CONSUMER_KEYCODE(QK_MODS_GET_BASIC_KEYCODE(keycode))) {
      if (record->event.pressed) {
        add_mods(QK_MODS_GET_MODS(keycode));
        send_keyboard_report();
        wait_ms(2);
        register_code(QK_MODS_GET_BASIC_KEYCODE(keycode));
        return false;
      } else {
        wait_ms(2);
        del_mods(QK_MODS_GET_MODS(keycode));
      }
    }
    break;

    case DUAL_FUNC_0:
      if (record->tap.count > 0) {
        if (record->event.pressed) {
          register_code16(KC_EQUAL);
        } else {
          unregister_code16(KC_EQUAL);
        }
      } else {
        if (record->event.pressed) {
          register_code16(KC_ESCAPE);
        } else {
          unregister_code16(KC_ESCAPE);
        }  
      }  
      return false;
    case DRAG_SCROLL:
      if (record->event.pressed) {
        set_scrolling = true;
      } else {
        set_scrolling = false;
      }
      send_toggle_states();
      return false;
    case TOGGLE_SCROLL:
      if (record->event.pressed) {
        set_scrolling = !set_scrolling;
        send_toggle_states();
      }
      return false;
    case KC_AUDIO_MUTE:
      if (record->event.pressed) {
        mute_active = !mute_active;
        send_toggle_states();
      }
      return true;
    break;
  case NAVIGATOR_TURBO:
    if (record->event.pressed) {
      navigator_turbo = true;
    } else {
      navigator_turbo = false;
    }
    return false;
  case NAVIGATOR_AIM:
    if (record->event.pressed) {
      navigator_aim = true;
    } else {
      navigator_aim = false;
    }
    return false;
  case NAVIGATOR_INC_CPI:
    if (record->event.pressed) {
        pointing_device_set_cpi(1);
    }
    return false;
  case NAVIGATOR_DEC_CPI:
    if (record->event.pressed) {
        pointing_device_set_cpi(0);
    }
    return false;
    case RGB_SLD:
      if (record->event.pressed) {
        rgblight_mode(1);
      }
      return false;
    case HSV_0_255_255:
      if (record->event.pressed) {
        rgblight_mode(1);
        rgblight_sethsv(0,255,255);
      }
      return false;
    case HSV_74_255_255:
      if (record->event.pressed) {
        rgblight_mode(1);
        rgblight_sethsv(74,255,255);
      }
      return false;
    case HSV_169_255_255:
      if (record->event.pressed) {
        rgblight_mode(1);
        rgblight_sethsv(169,255,255);
      }
      return false;
    case CYCLE_PALETTE:
      if (record->event.pressed) {
        current_palette = (current_palette + 1) % NUM_PALETTES;
        send_toggle_states();
      }
      return false;
  }
  return true;
}

/* ─── Per-Key Per-Layer RGB Indicators ─── */

enum rgb_cat {
    CAT_ALPHA = 0, CAT_MOD, CAT_NAV, CAT_SYM, CAT_FUNC,
    CAT_MEDIA, CAT_EDIT, CAT_RGB, CAT_LAYER, CAT_MOUSE,
    CAT_TRANS, CAT_OFF, CAT_SYS, CAT_OTHER,
    CAT_NUMPAD, CAT_ARROW,
    CAT_SHIFT, CAT_CTRL, CAT_ALT, CAT_GUI,
    CAT_NUM,
    CAT_VOLUME, CAT_BRIGHT,
    CAT_PAREN, CAT_BRACKET, CAT_BRACE,
    CAT_COUNT
};

static const uint8_t PROGMEM rgb_palettes[NUM_PALETTES][CAT_COUNT][3] = {
    [PAL_VIBRANT] = {
        [CAT_ALPHA]  = {160, 155, 140},
        [CAT_MOD]    = {  0, 200, 200},
        [CAT_NAV]    = { 40,  80, 220},
        [CAT_SYM]    = {220, 130,   0},
        [CAT_FUNC]   = {140,  40, 220},
        [CAT_MEDIA]  = {  0, 200,  50},
        [CAT_EDIT]   = {220,  30,  30},
        [CAT_RGB]    = {220, 200,   0},
        [CAT_LAYER]  = {220,   0, 220},
        [CAT_MOUSE]  = {  0, 200, 130},
        [CAT_TRANS]  = { 25,  25,  25},
        [CAT_OFF]    = {  0,   0,   0},
        [CAT_SYS]    = {255,   0,   0},
        [CAT_OTHER]  = {100, 100, 100},
        [CAT_NUMPAD] = {  0, 230, 180},
        [CAT_ARROW]  = {240, 240, 240},
        [CAT_SHIFT]  = {255,  64,   0},
        [CAT_CTRL]   = { 64, 255,   0},
        [CAT_ALT]    = {  0, 191, 255},
        [CAT_GUI]    = {191,   0, 255},
        [CAT_NUM]    = {255, 200,  60},
        [CAT_VOLUME] = {255,  50, 130},
        [CAT_BRIGHT] = {180, 255,  50},
        [CAT_PAREN]  = {255, 120, 200},
        [CAT_BRACKET]= { 80, 200, 255},
        [CAT_BRACE]  = {160, 255,  80},
    },
    [PAL_DARK] = {
        [CAT_ALPHA]  = { 56,  54,  49},
        [CAT_MOD]    = {  0,  70,  70},
        [CAT_NAV]    = { 14,  28,  77},
        [CAT_SYM]    = { 77,  46,   0},
        [CAT_FUNC]   = { 49,  14,  77},
        [CAT_MEDIA]  = {  0,  70,  18},
        [CAT_EDIT]   = { 77,  11,  11},
        [CAT_RGB]    = { 77,  70,   0},
        [CAT_LAYER]  = { 77,   0,  77},
        [CAT_MOUSE]  = {  0,  70,  46},
        [CAT_TRANS]  = {  8,   8,   8},
        [CAT_OFF]    = {  0,   0,   0},
        [CAT_SYS]    = { 89,   0,   0},
        [CAT_OTHER]  = { 35,  35,  35},
        [CAT_NUMPAD] = {  0,  81,  63},
        [CAT_ARROW]  = { 84,  84,  84},
        [CAT_SHIFT]  = { 89,  22,   0},
        [CAT_CTRL]   = { 22,  89,   0},
        [CAT_ALT]    = {  0,  67,  89},
        [CAT_GUI]    = { 67,   0,  89},
        [CAT_NUM]    = { 89,  70,  21},
        [CAT_VOLUME] = { 89,  18,  46},
        [CAT_BRIGHT] = { 63,  89,  18},
        [CAT_PAREN]  = { 89,  42,  70},
        [CAT_BRACKET]= { 28,  70,  89},
        [CAT_BRACE]  = { 56,  89,  28},
    },
    [PAL_LIGHT] = {
        [CAT_ALPHA]  = {227, 225, 221},
        [CAT_MOD]    = {179, 239, 239},
        [CAT_NAV]    = {191, 203, 245},
        [CAT_SYM]    = {245, 218, 179},
        [CAT_FUNC]   = {221, 191, 245},
        [CAT_MEDIA]  = {179, 239, 194},
        [CAT_EDIT]   = {245, 188, 188},
        [CAT_RGB]    = {245, 239, 179},
        [CAT_LAYER]  = {245, 179, 245},
        [CAT_MOUSE]  = {179, 239, 218},
        [CAT_TRANS]  = {140, 140, 140},
        [CAT_OFF]    = {  0,   0,   0},
        [CAT_SYS]    = {255, 179, 179},
        [CAT_OTHER]  = {209, 209, 209},
        [CAT_NUMPAD] = {179, 248, 233},
        [CAT_ARROW]  = {251, 251, 251},
        [CAT_SHIFT]  = {255, 198, 179},
        [CAT_CTRL]   = {198, 255, 179},
        [CAT_ALT]    = {179, 236, 255},
        [CAT_GUI]    = {236, 179, 255},
        [CAT_NUM]    = {255, 239, 197},
        [CAT_VOLUME] = {255, 194, 218},
        [CAT_BRIGHT] = {233, 255, 194},
        [CAT_PAREN]  = {255, 215, 239},
        [CAT_BRACKET]= {203, 239, 255},
        [CAT_BRACE]  = {227, 255, 203},
    },
    [PAL_PASTEL] = {
        [CAT_ALPHA]  = {180, 175, 165},
        [CAT_MOD]    = {120, 200, 200},
        [CAT_NAV]    = {120, 150, 210},
        [CAT_SYM]    = {210, 170, 100},
        [CAT_FUNC]   = {180, 130, 210},
        [CAT_MEDIA]  = {120, 200, 140},
        [CAT_EDIT]   = {210, 130, 130},
        [CAT_RGB]    = {210, 200, 120},
        [CAT_LAYER]  = {210, 120, 210},
        [CAT_MOUSE]  = {120, 200, 175},
        [CAT_TRANS]  = { 20,  20,  20},
        [CAT_OFF]    = {  0,   0,   0},
        [CAT_SYS]    = {230, 100, 100},
        [CAT_OTHER]  = {140, 140, 140},
        [CAT_NUMPAD] = {120, 210, 190},
        [CAT_ARROW]  = {210, 210, 210},
        [CAT_SHIFT]  = {220, 150, 120},
        [CAT_CTRL]   = {150, 220, 120},
        [CAT_ALT]    = {120, 185, 220},
        [CAT_GUI]    = {185, 120, 220},
        [CAT_NUM]    = {220, 195, 130},
        [CAT_VOLUME] = {220, 130, 170},
        [CAT_BRIGHT] = {185, 220, 130},
        [CAT_PAREN]  = {220, 155, 195},
        [CAT_BRACKET]= {140, 195, 220},
        [CAT_BRACE]  = {175, 220, 140},
    },
};

static const uint8_t PROGMEM layer_cat_map[][52] = {
    [0] = { // Main
        CAT_SYM,   CAT_NUM,   CAT_NUM,   CAT_NUM,   CAT_NUM,   CAT_NUM,
        CAT_EDIT,  CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA,
        CAT_SHIFT, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA,
        CAT_GUI,   CAT_ALT,   CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA,
        CAT_LAYER, CAT_CTRL,
        CAT_NUM,   CAT_NUM,   CAT_NUM,   CAT_NUM,   CAT_NUM,   CAT_SYM,
        CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM,
        CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM,   CAT_SHIFT,
        CAT_ALPHA, CAT_ALPHA, CAT_SYM,   CAT_SYM,   CAT_ALT,   CAT_CTRL,
        CAT_SHIFT, CAT_LAYER
    },
    [1] = { // Sym+Num
        CAT_EDIT,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,
        CAT_SYM,   CAT_SYM,   CAT_SYM,   CAT_SYM,   CAT_SYM,   CAT_SYM,
        CAT_TRANS,  CAT_SYM,   CAT_SYM,   CAT_SYM,   CAT_PAREN, CAT_PAREN,
        CAT_TRANS,  CAT_TRANS,  CAT_BRACKET,CAT_BRACKET,CAT_BRACE, CAT_BRACE,
        CAT_TRANS,  CAT_TRANS,
        CAT_FUNC,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,  CAT_FUNC,
        CAT_NUMPAD,CAT_NUMPAD,CAT_NUMPAD,CAT_SYM,   CAT_SYM,   CAT_FUNC,
        CAT_NUMPAD,CAT_NUMPAD,CAT_NUMPAD,CAT_SYM,   CAT_SYM,   CAT_EDIT,
        CAT_NUMPAD,CAT_NUMPAD,CAT_NUMPAD,CAT_NUMPAD,CAT_SYM,   CAT_EDIT,
        CAT_TRANS,  CAT_NUMPAD
    },
    [2] = { // Brd+Sys
        CAT_RGB,   CAT_LAYER, CAT_RGB,   CAT_RGB,   CAT_BRIGHT,CAT_BRIGHT,
        CAT_TRANS,  CAT_TRANS,  CAT_VOLUME,CAT_VOLUME,CAT_VOLUME,CAT_TRANS,
        CAT_TRANS,  CAT_MEDIA, CAT_MEDIA, CAT_MEDIA, CAT_MEDIA, CAT_TRANS,
        CAT_TRANS,  CAT_TRANS,  CAT_TRANS,  CAT_RGB,   CAT_RGB,   CAT_RGB,
        CAT_TRANS,  CAT_TRANS,
        CAT_TRANS,  CAT_TRANS,  CAT_TRANS,  CAT_TRANS,  CAT_TRANS,  CAT_SYS,
        CAT_NAV,   CAT_NAV,   CAT_ARROW, CAT_NAV,   CAT_TRANS,  CAT_TRANS,
        CAT_NAV,   CAT_ARROW, CAT_ARROW, CAT_ARROW, CAT_TRANS,  CAT_TRANS,
        CAT_TRANS,  CAT_EDIT,  CAT_EDIT,  CAT_TRANS,  CAT_TRANS,  CAT_TRANS,
        CAT_TRANS,  CAT_TRANS
    },
    [3] = { // Mouse
        CAT_MOUSE, CAT_MOUSE, CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_LAYER,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_MOUSE, CAT_MOUSE,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_MOUSE, CAT_MOUSE, CAT_OFF,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,   CAT_OFF,
        CAT_OFF,   CAT_OFF
    },
};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer > 3) return false;

    for (uint8_t i = led_min; i <= led_max && i < 52; i++) {
        uint8_t cat = pgm_read_byte(&layer_cat_map[layer][i]);
        rgb_matrix_set_color(i,
            pgm_read_byte(&rgb_palettes[current_palette][cat][0]),
            pgm_read_byte(&rgb_palettes[current_palette][cat][1]),
            pgm_read_byte(&rgb_palettes[current_palette][cat][2]));
    }

    // Dynamic scroll toggle indicator (position 11 on mouse layer)
    if (layer == 3 && led_min <= 11 && 11 <= led_max) {
        if (set_scrolling) {
            rgb_matrix_set_color(11, 0, 255, 50);   // bright green = scroll active
        } else {
            rgb_matrix_set_color(11, 0, 200, 130);   // teal = scroll inactive
        }
    }

    // Dynamic mute indicator (position 10 on Brd+Sys layer)
    if (layer == 2 && led_min <= 10 && 10 <= led_max) {
        if (mute_active) {
            rgb_matrix_set_color(10, 255, 0, 0);     // bright red = muted
        }
    }

    // Dynamic CapsWord indicator (position 6 on Main layer)
    if (layer == 0 && led_min <= 6 && 6 <= led_max) {
        if (caps_word_active_state) {
            rgb_matrix_set_color(6, 255, 255, 0);    // bright yellow = caps active
        }
    }

    return false;
}
