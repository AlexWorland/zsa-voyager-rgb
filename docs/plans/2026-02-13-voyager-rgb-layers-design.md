# ZSA Voyager RGB Layer Generator — Design Document

**Date**: 2026-02-13
**Status**: Approved (v2 — updated with Oryx API discovery)

## Purpose

Enable Claude Code to programmatically generate per-key, per-layer RGB color schemes for the ZSA Voyager keyboard, compile the firmware, and flash it — replacing manual Oryx UI clicking.

## Approach

**Oryx API + QMK Source Editing** — Claude Code fetches the user's layout from the Oryx GraphQL API, categorizes keys from the structured JSON, proposes color schemes, downloads the QMK source via the Oryx source endpoint, writes `rgb_matrix_indicators_advanced_user()` into the source, and builds/flashes via QMK CLI.

### Key Discovery: Oryx API

ZSA's Oryx configurator exposes two undocumented but publicly accessible endpoints:

| Endpoint | Method | Returns |
|----------|--------|---------|
| `https://oryx.zsa.io/graphql` | POST (GraphQL) | Full layout JSON: all layers, all 52 keys with tap/hold actions, layer targets, glow colors |
| `https://oryx.zsa.io/{hashId}/{revisionId}/source` | GET (302 redirect) | ZIP containing QMK source (`keymap.c`, `config.h`, `rules.mk`) + pre-compiled `.bin` |

Both work unauthenticated from `curl`. No manual export needed.

**GraphQL query:**
```graphql
query getLayout($hashId: String!, $revisionId: String!, $geometry: String) {
  layout(hashId: $hashId, geometry: $geometry, revisionId: $revisionId) {
    hashId
    title
    revision {
      hashId
      qmkVersion
      layers {
        position
        title
        keys
      }
    }
  }
}
```

**Per-key JSON structure:**
```json
{
  "tap":  { "code": "KC_A", "layer": null, "modifier": null },
  "hold": { "code": "KC_LEFT_SHIFT", "layer": null },
  "glowColor": null,
  "customLabel": null
}
```

### Why This Approach

- **No manual export** — user provides their Oryx URL, everything else is automated
- **Structured data** — JSON is far easier to parse than C source macros
- **Full QMK power** — per-key, per-layer, conditional logic in generated C code
- **Source download included** — ZIP provides the exact QMK source for compilation

### Alternatives Considered

- **Manual Oryx export**: Requires user to download/unzip — eliminated by API discovery
- **USB HID read**: Oryx protocol doesn't expose keymap data (only key events and LED control)
- **DFU binary dump**: Requires reverse-engineering compiled binary — fragile and unnecessary
- **Custom MCP Server**: Overkill for current needs
- **Config-driven generator script**: Adds indirection without benefit

## Workflow

```
1. INPUT
   User provides Oryx layout URL:
   https://configure.zsa.io/voyager/layouts/{hashId}/latest/0
   Claude Code extracts hashId and geometry

2. FETCH
   GraphQL API → full keymap JSON (all layers, all keys)
   Source endpoint → QMK source ZIP

3. ANALYZE
   Parse JSON keys array (52 elements = 52 physical positions)
   Categorize each key on each layer by function

4. GENERATE RGB SCHEMES
   Propose 2-3 color palettes
   Render text-based Voyager layout previews
   User picks one or requests modifications

5. WRITE CODE
   Unzip QMK source into oryx-export/
   Generate rgb_matrix_indicators_advanced_user() in keymap.c
   Enable RGB_MATRIX in config.h and rules.mk

6. BUILD & FLASH
   make zsa/voyager:<keymap>:flash
   User puts Voyager in bootloader mode (physical reset button)
```

## Project Structure

```
~/projects/zsa-voyager-rgb/
├── oryx-export/          # Downloaded QMK source from Oryx API
│   ├── keymap.c          # Layer definitions (modified with RGB code)
│   ├── config.h          # Board config
│   └── rules.mk          # Build rules
├── rgb-schemes/          # Generated color palette definitions
│   ├── keymap-analysis.json  # Parsed key categorizations
│   └── current.json          # Active scheme (category → color mappings)
├── scripts/
│   └── fetch-layout.sh   # Fetch layout from Oryx API
├── docs/
│   └── plans/            # Design documents
├── build.sh              # Build and flash helper
└── README.md
```

## Voyager Physical Layout

52 keys total: 4 rows x 6 columns x 2 sides (48 main) + 2 thumb keys x 2 sides (4 thumb).

```
Left Half                              Right Half
┌────┬────┬────┬────┬────┬────┐       ┌────┬────┬────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │       │26  │27  │28  │29  │30  │31  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────┼────┤
│ 6  │ 7  │ 8  │ 9  │10  │11  │       │32  │33  │34  │35  │36  │37  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────┼────┤
│12  │13  │14  │15  │16  │17  │       │38  │39  │40  │41  │42  │43  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────┼────┤
│18  │19  │20  │21  │22  │23  │       │44  │45  │46  │47  │48  │49  │
└────┴────┴────┴────┼────┼────┤       ├────┼────┼────┴────┴────┴────┘
                    │24  │25  │       │50  │51  │
                    └────┴────┘       └────┘────┘
```

LED/key index mapping: Array index from the Oryx API JSON directly maps to LED index.
- 0-23: left half (row by row, top to bottom, left to right)
- 24-25: left thumb keys
- 26-49: right half (same order)
- 50-51: right thumb keys

## Key Categorization

Claude Code classifies every key on every layer using the structured JSON:

| Category | JSON tap.code patterns | Default Color Role |
|----------|----------------------|-------------------|
| `alpha` | KC_A through KC_Z, KC_1-KC_0, KC_COMMA, KC_DOT, etc. | Base typing color |
| `modifier` | KC_LEFT_SHIFT, KC_LEFT_CTRL, KC_LEFT_ALT, KC_LEFT_GUI (+ RIGHT variants). Also detected via hold.code on dual-function keys | Accent color |
| `nav` | KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT, KC_HOME, KC_END, KC_PGUP, KC_PGDN | Directional color |
| `symbol` | KC_MINUS, KC_EQUAL, KC_LBRC, KC_RBRC, KC_BSLS, KC_GRV, KC_TILD | Symbol color |
| `numpad` | KC_KP_* keycodes | Numeric color |
| `function` | KC_F1 through KC_F24 | Function color |
| `media` | KC_VOLU, KC_VOLD, KC_MUTE, KC_MPLY, KC_MNXT, KC_MPRV, KC_BRIU, KC_BRID | Media color |
| `layer_toggle` | Detected via hold.code = "MO", "TG", "TO", "OSL", "TT" with hold.layer set | Layer indicator color |
| `editing` | KC_BSPC, KC_DEL, KC_ENTER, KC_TAB, KC_ESC, CW_TOGG | Editing color |
| `rgb_control` | RGB_TOG, RGB_MODE_*, RGB_VAI/VAD, RGB_HUI/HUD, RGB_SLD, TOGGLE_LAYER_COLOR | RGB control color |
| `transparent` | KC_TRANSPARENT | Dim/inherit from below |
| `unused` | KC_NO | Off |

## Color Scheme Presets

Claude Code proposes 3 scheme types:

### Functional (high contrast)
Every category gets a distinct, saturated color. Maximizes information density.

### Minimal (subtle)
Only modifiers, layer toggles, and nav keys are lit. Alpha keys stay off. Clean aesthetic.

### Vibrant (full color)
All keys lit with saturated colors. Bright, colorful, every key clearly categorized.

## Code Generation

Single function inserted into `keymap.c`:

```c
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state | default_layer_state);
    for (uint8_t i = led_min; i < led_max; i++) {
        switch (layer) {
            case 0:
                // per-key colors from scheme
                break;
            case 1:
                // ...
                break;
        }
    }
    return false;
}
```

Additionally enables in `rules.mk`:
```
RGB_MATRIX_ENABLE = yes
```

And sets defaults in `config.h`:
```c
#define RGB_MATRIX_STARTUP_VAL 128
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200
```

## Build & Flash

**Build**: `make zsa/voyager:<keymap_name>` from ZSA QMK fork root.

**Flash options**:
1. `make zsa/voyager:<keymap_name>:flash` — builds and flashes (DFU bootloader, STM32)
2. Keymapp GUI — drag compiled `.bin` file

**Bootloader entry**: Physical reset button on the Voyager. Claude Code prompts user to press it before flashing.

## Prerequisites (One-Time Setup)

1. Install QMK CLI: `brew install qmk/qmk/qmk`
2. Install dfu-util: `brew install dfu-util`
3. Clone ZSA fork: `qmk setup zsa/qmk_firmware -b firmware24 -H ~/projects/zsa-voyager-rgb/qmk_firmware`

## API Stability Note

The Oryx GraphQL API and source download endpoint are **undocumented**. They work today (2026-02-13) but could change without notice. The manual Oryx export workflow remains as a fallback — the project structure supports both paths.
