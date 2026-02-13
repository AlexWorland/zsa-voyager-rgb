
/* ─── Per-Key Per-Layer RGB Indicators ─── */

enum rgb_cat {
    CAT_ALPHA = 0,
    CAT_MOD = 1,
    CAT_NAV = 2,
    CAT_SYM = 3,
    CAT_FUNC = 4,
    CAT_MEDIA = 5,
    CAT_EDIT = 6,
    CAT_RGB = 7,
    CAT_LAYER = 8,
    CAT_MOUSE = 9,
    CAT_TRANS = 10,
    CAT_OFF = 11,
    CAT_SYS = 12,
    CAT_OTHER = 13,
    CAT_COUNT
};

static const uint8_t PROGMEM rgb_colors[][3] = {
    [CAT_ALPHA] = {160, 155, 140},  // warm white
    [CAT_MOD] = {  0, 200, 200},  // cyan
    [CAT_NAV] = { 40,  80, 220},  // blue
    [CAT_SYM] = {220, 130,   0},  // orange
    [CAT_FUNC] = {140,  40, 220},  // purple
    [CAT_MEDIA] = {  0, 200,  50},  // green
    [CAT_EDIT] = {220,  30,  30},  // red
    [CAT_RGB] = {220, 200,   0},  // yellow
    [CAT_LAYER] = {220,   0, 220},  // magenta
    [CAT_MOUSE] = {  0, 200, 130},  // teal
    [CAT_TRANS] = { 25,  25,  25},  // dim glow
    [CAT_OFF] = {  0,   0,   0},  // off
    [CAT_SYS] = {255,   0,   0},  // bright red
    [CAT_OTHER] = {100, 100, 100},  // gray
};

static const uint8_t PROGMEM layer_cat_map[][52] = {
    [0] = { // Main
        CAT_SYM, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_EDIT, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_EDIT,
        CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_MOD, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_LAYER, CAT_EDIT,
        CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_ALPHA,
        CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_MOD, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_MOD, CAT_EDIT, CAT_LAYER
    },
    [1] = { // Sym+Num
        CAT_EDIT, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_TRANS,
        CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_TRANS, CAT_TRANS, CAT_SYM, CAT_SYM, CAT_SYM, CAT_SYM, CAT_TRANS, CAT_TRANS,
        CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_FUNC, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_SYM, CAT_FUNC, CAT_ALPHA,
        CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_SYM, CAT_EDIT, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_ALPHA, CAT_SYM, CAT_EDIT, CAT_TRANS, CAT_ALPHA
    },
    [2] = { // Brd+Sys
        CAT_RGB, CAT_RGB, CAT_RGB, CAT_RGB, CAT_RGB, CAT_RGB, CAT_TRANS, CAT_TRANS, CAT_MEDIA, CAT_MEDIA, CAT_MEDIA, CAT_TRANS, CAT_TRANS,
        CAT_MEDIA, CAT_MEDIA, CAT_MEDIA, CAT_MEDIA, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_OTHER, CAT_OTHER, CAT_OTHER, CAT_TRANS, CAT_TRANS,
        CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_SYS, CAT_NAV, CAT_NAV, CAT_NAV, CAT_NAV, CAT_TRANS, CAT_TRANS, CAT_NAV,
        CAT_NAV, CAT_NAV, CAT_NAV, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_EDIT, CAT_EDIT, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_TRANS, CAT_TRANS
    },
    [3] = { // Mouse
        CAT_MOUSE, CAT_MOUSE, CAT_OFF, CAT_OFF, CAT_OFF, CAT_LAYER, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_MOUSE, CAT_MOUSE, CAT_OFF,
        CAT_OFF, CAT_OFF, CAT_MOUSE, CAT_MOUSE, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF,
        CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF,
        CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF, CAT_OFF
    },
};

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    if (layer > 3) return false;

    for (uint8_t i = led_min; i <= led_max && i < 52; i++) {
        uint8_t cat = pgm_read_byte(&layer_cat_map[layer][i]);
        rgb_matrix_set_color(i,
            pgm_read_byte(&rgb_colors[cat][0]),
            pgm_read_byte(&rgb_colors[cat][1]),
            pgm_read_byte(&rgb_colors[cat][2]));
    }
    return false;
}