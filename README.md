# ZSA Voyager RGB Layer Generator

Programmatic per-key, per-layer RGB color schemes for the ZSA Voyager keyboard via the Oryx API and QMK firmware.

## What It Does

This project enables automated generation of custom RGB lighting schemes for the ZSA Voyager keyboard. Instead of manually clicking through the Oryx configurator's UI, Claude Code:

1. Fetches your keyboard layout from the Oryx GraphQL API
2. Categorizes every key by function (alpha, modifier, nav, symbol, etc.)
3. Generates color schemes based on key categories
4. Injects RGB code into QMK firmware source
5. Compiles and flashes the firmware to your keyboard

The result: per-key, per-layer RGB lighting that adapts to your actual keymap structure, fully customizable through natural language requests.

## Prerequisites

Install the following tools once before using this project:

### 1. QMK CLI
```bash
brew install qmk/qmk/qmk
```

Verify installation:
```bash
qmk --version
```

### 2. dfu-util
```bash
brew install dfu-util
```

Verify installation:
```bash
dfu-util --version
```

### 3. ZSA QMK Firmware Fork
```bash
qmk setup zsa/qmk_firmware -b firmware24 -H ~/projects/zsa-voyager-rgb/qmk_firmware
```

This clones the ZSA fork into `qmk_firmware/` on the `firmware24` branch, which includes Voyager-specific support.

Verify the build environment works:
```bash
cd ~/projects/zsa-voyager-rgb/qmk_firmware && make zsa/voyager:default
```

## Quick Start

### 1. Fetch Your Layout

Provide your Oryx layout URL to the fetch script:

```bash
./scripts/fetch-layout.sh https://configure.zsa.io/voyager/layouts/AbCdE/latest/0
```

This fetches your keymap JSON and downloads the QMK source files into `oryx-export/`.

### 2. Generate RGB Scheme

Work with Claude Code to analyze your keymap and generate color schemes:

```
Claude, analyze my Voyager layout and propose RGB color schemes for my layers.
```

Claude Code will:
- Parse the layout JSON
- Categorize each key by function
- Propose multiple color schemes (Functional, Minimal, Vibrant)
- Show text-based previews with your actual key labels
- Generate the RGB C code and inject it into `keymap.c`

### 3. Build and Flash

Compile the firmware:
```bash
./build.sh build
```

Flash to your Voyager:
```bash
./build.sh flash
```

Follow the prompt to put your Voyager in bootloader mode (press the reset button on the underside of the left half).

## Script Usage

### fetch-layout.sh

Downloads your layout from Oryx and extracts QMK source files.

```bash
./scripts/fetch-layout.sh <oryx-url-or-hash> [geometry]
```

Examples:
```bash
# From full Oryx URL
./scripts/fetch-layout.sh https://configure.zsa.io/voyager/layouts/AbCdE/latest/0

# From layout hash only
./scripts/fetch-layout.sh AbCdE

# Specify geometry (default: voyager)
./scripts/fetch-layout.sh AbCdE voyager
```

Output:
- `rgb-schemes/layout.json` — Full keymap structure
- `oryx-export/*.{c,h,mk}` — QMK source files

### build.sh

Manages QMK keymap syncing, compilation, and flashing.

```bash
./build.sh {sync|build|flash}
```

Commands:
- `sync` — Copy `oryx-export/` files into QMK keymap directory
- `build` — Compile firmware (runs sync first)
- `flash` — Compile and flash firmware (runs sync first)

## Physical Layout

The Voyager has 52 keys: 4 rows × 6 columns × 2 sides (48 keys) + 2 thumb keys × 2 sides (4 keys).

```
Left Half                              Right Half
┌────┬────┬────┬────┬────┬────┐       ┌────┬────┬────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │       │26  │27  │28  │29  │30  │31  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────┼────┤
│ 6  │ 7  │ 8  │ 9  │10  │11  │       │32  │33  │34  │35  │36  │37  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────┼────┤
│12  │13  │14  │15  │16  │17  │       │38  │39  │40  │41  │42  │43  │
├────┼────┼────┼────┼────┼────┤       ├────┼────┼────┼────┼────�┼────┤
│18  │19  │20  │21  │22  │23  │       │44  │45  │46  │47  │48  │49  │
└────┴────┴────┴────┼────┼────┤       ├────┼────┼────┴────┴────┴────┘
                    │24  │25  │       │50  │51  │
                    └────┴────┘       └────┘────┘
```

Array indices from the Oryx API map directly to LED indices in QMK.

## Key Categories

Claude Code classifies keys into functional categories based on their keycodes:

| Category | Example Keys | Purpose |
|----------|-------------|---------|
| `alpha` | A-Z, 0-9, comma, period | Base typing keys |
| `modifier` | Shift, Ctrl, Alt, GUI | Modifier keys (tap or hold) |
| `nav` | Arrow keys, Home, End, PgUp, PgDn | Navigation |
| `symbol` | `-`, `=`, `[`, `]`, `\`, `` ` ``, `~` | Symbol keys |
| `numpad` | KP_0 through KP_9, KP_PLUS, etc. | Numpad keys |
| `function` | F1-F24 | Function keys |
| `media` | Volume, play/pause, brightness | Media controls |
| `layer_toggle` | MO(1), TG(2), TO(3), etc. | Layer switching |
| `editing` | Backspace, Delete, Enter, Tab, Esc | Editing actions |
| `rgb_control` | RGB_TOG, RGB_MODE_*, RGB_VAI/VAD | RGB lighting controls |
| `transparent` | KC_TRANSPARENT | Passes through to lower layer |
| `unused` | KC_NO | Disabled key |

Each key is assigned a color based on its category and the active layer.

## Iterative Workflow

### Changing Colors

To modify your RGB scheme:

1. Tell Claude Code your desired changes:
   ```
   Claude, make all modifiers cyan instead of red.
   ```

2. Claude Code regenerates the RGB code and rebuilds:
   ```bash
   ./build.sh build
   ```

3. Flash the updated firmware:
   ```bash
   ./build.sh flash
   ```

### Updating After Oryx Changes

If you modify your layout in the Oryx configurator:

1. Re-fetch your layout:
   ```bash
   ./scripts/fetch-layout.sh <your-oryx-url>
   ```

2. Ask Claude Code to re-analyze and regenerate RGB:
   ```
   Claude, I updated my layout. Re-analyze the keys and regenerate the RGB scheme.
   ```

3. Build and flash:
   ```bash
   ./build.sh flash
   ```

### Manual RGB Code Editing

The RGB logic lives in `oryx-export/keymap.c` in the `rgb_matrix_indicators_advanced_user()` function. You can edit this directly if you prefer, then rebuild with `./build.sh build`.

## Oryx API Note

This project uses two undocumented but publicly accessible Oryx endpoints:

| Endpoint | Purpose |
|----------|---------|
| `https://oryx.zsa.io/graphql` | Fetch full keymap JSON (all layers, keys, actions) |
| `https://oryx.zsa.io/{hash}/{revision}/source` | Download QMK source ZIP |

Both work unauthenticated. They are not officially documented by ZSA and could change without notice. The manual Oryx export workflow remains available as a fallback.

## Project Structure

```
~/projects/zsa-voyager-rgb/
├── oryx-export/              # QMK source from Oryx API
│   ├── keymap.c              # Layer definitions + RGB code
│   ├── config.h              # Board config
│   ├── rules.mk              # Build rules
│   └── source.zip            # Original download
├── rgb-schemes/              # Generated color schemes
│   ├── layout.json           # Full keymap from Oryx GraphQL
│   ├── keymap-analysis.json  # Key categorizations
│   └── current.json          # Active color scheme
├── scripts/
│   └── fetch-layout.sh       # Fetch layout from Oryx API
├── qmk_firmware/             # ZSA QMK fork (git ignored)
├── build.sh                  # Build and flash helper
└── docs/
    └── plans/                # Design documents
```

## License

MIT
