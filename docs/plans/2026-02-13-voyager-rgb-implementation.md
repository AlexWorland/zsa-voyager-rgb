# Voyager RGB Layer Generator — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Set up a complete workflow where Claude Code can generate per-key, per-layer RGB color schemes for a ZSA Voyager keyboard, compile the firmware, and flash it — fully automated from an Oryx layout URL.

**Architecture:** The Oryx GraphQL API provides structured keymap JSON. The Oryx source endpoint provides QMK source files. Claude Code categorizes keys from JSON, generates `rgb_matrix_indicators_advanced_user()` C code, injects it into the downloaded source, and builds/flashes via QMK CLI.

**Tech Stack:** QMK firmware (C), ZSA QMK fork (`firmware24` branch), Oryx GraphQL API, `dfu-util` for flashing, shell scripts for automation.

---

### Task 1: Install QMK Build Environment

**Files:**
- None (system-level installation)

**Step 1: Install QMK CLI via Homebrew**

Run:
```bash
brew install qmk/qmk/qmk
```

Expected: QMK CLI installed. Verify with `qmk --version`.

**Step 2: Install dfu-util for Voyager flashing**

Run:
```bash
brew install dfu-util
```

Expected: `dfu-util --version` shows version info.

**Step 3: Clone ZSA QMK firmware fork**

Run:
```bash
qmk setup zsa/qmk_firmware -b firmware24 -H ~/projects/zsa-voyager-rgb/qmk_firmware
```

This clones the ZSA fork into `~/projects/zsa-voyager-rgb/qmk_firmware/` on the `firmware24` branch.

Expected: Full QMK firmware source cloned with ZSA keyboard definitions under `keyboards/zsa/voyager/`.

**Step 4: Verify build environment works**

Run:
```bash
cd ~/projects/zsa-voyager-rgb/qmk_firmware && make zsa/voyager:default
```

Expected: Firmware compiles successfully, produces a `.bin` file.

---

### Task 2: Scaffold Project Structure

**Files:**
- Create: `~/projects/zsa-voyager-rgb/.gitignore`
- Create: `~/projects/zsa-voyager-rgb/README.md`
- Create: directories `oryx-export/`, `rgb-schemes/`, `scripts/`

**Step 1: Create directory structure**

```bash
mkdir -p ~/projects/zsa-voyager-rgb/{oryx-export,rgb-schemes,scripts}
```

**Step 2: Create .gitignore**

Create `~/projects/zsa-voyager-rgb/.gitignore`:
```
qmk_firmware/
*.bin
*.hex
*.uf2
*.zip
.DS_Store
```

**Step 3: Create README.md**

Create `~/projects/zsa-voyager-rgb/README.md` with quick start instructions, referencing the Oryx URL workflow.

**Step 4: Initialize git repo and commit**

```bash
cd ~/projects/zsa-voyager-rgb && git init && git add .gitignore README.md docs/ && git commit -m "Initial project scaffold"
```

---

### Task 3: Create Fetch Layout Script

**Files:**
- Create: `~/projects/zsa-voyager-rgb/scripts/fetch-layout.sh`

**Step 1: Write fetch-layout.sh**

This script takes an Oryx layout URL or hash and:
1. Extracts the geometry and hashId from the URL
2. Calls the Oryx GraphQL API to get the full keymap JSON
3. Saves the JSON to `rgb-schemes/layout.json`
4. Extracts the revision hash
5. Downloads the QMK source ZIP from the source endpoint
6. Unzips the source files into `oryx-export/`

```bash
#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

# Accept URL or hashId
INPUT="${1:?Usage: fetch-layout.sh <oryx-url-or-hash> [geometry]}"
GEOMETRY="${2:-voyager}"

# Extract hashId from URL if full URL provided
if [[ "$INPUT" == *"configure.zsa.io"* ]]; then
    HASH_ID=$(echo "$INPUT" | grep -oP 'layouts/\K[^/]+')
else
    HASH_ID="$INPUT"
fi

echo "Fetching layout: $HASH_ID (geometry: $GEOMETRY)"

# Fetch layout JSON via GraphQL
RESPONSE=$(curl -s -X POST https://oryx.zsa.io/graphql \
  -H 'Content-Type: application/json' \
  -d "{\"operationName\":\"getLayout\",\"variables\":{\"hashId\":\"$HASH_ID\",\"geometry\":\"$GEOMETRY\",\"revisionId\":\"latest\"},\"query\":\"query getLayout(\$hashId: String!, \$revisionId: String!, \$geometry: String) { layout(hashId: \$hashId, geometry: \$geometry, revisionId: \$revisionId) { hashId title geometry revision { hashId qmkVersion layers { position title keys color } } } }\"}")

# Save full response
echo "$RESPONSE" | python3 -m json.tool > "$PROJECT_DIR/rgb-schemes/layout.json"

# Extract revision hash for source download
REVISION_HASH=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['revision']['hashId'])")
LAYOUT_HASH=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['hashId'])")
TITLE=$(echo "$RESPONSE" | python3 -c "import json,sys; print(json.load(sys.stdin)['data']['layout']['title'])")
LAYERS=$(echo "$RESPONSE" | python3 -c "import json,sys; layers=json.load(sys.stdin)['data']['layout']['revision']['layers']; [print(f'  Layer {l[\"position\"]}: {l[\"title\"]} ({len(l[\"keys\"])} keys)') for l in layers]")

echo "Layout: $TITLE"
echo "Hash: $LAYOUT_HASH, Revision: $REVISION_HASH"
echo "Layers:"
echo "$LAYERS"

# Download source ZIP
echo "Downloading QMK source..."
curl -sL "https://oryx.zsa.io/$LAYOUT_HASH/$REVISION_HASH/source" -o "$PROJECT_DIR/oryx-export/source.zip"

# Unzip, flattening any subdirectory
cd "$PROJECT_DIR/oryx-export"
unzip -o source.zip
# Find and move source files if nested
SUBDIR=$(find . -maxdepth 1 -type d -name "*source*" | head -1)
if [ -n "$SUBDIR" ]; then
    mv "$SUBDIR"/* . 2>/dev/null || true
    rmdir "$SUBDIR" 2>/dev/null || true
fi

echo "Done. Source files in oryx-export/, layout JSON in rgb-schemes/layout.json"
```

**Step 2: Make executable**

```bash
chmod +x ~/projects/zsa-voyager-rgb/scripts/fetch-layout.sh
```

**Step 3: Commit**

```bash
cd ~/projects/zsa-voyager-rgb && git add scripts/ && git commit -m "Add Oryx layout fetch script"
```

---

### Task 4: Create Build/Flash Helper Script

**Files:**
- Create: `~/projects/zsa-voyager-rgb/build.sh`

**Step 1: Write build.sh**

```bash
#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
QMK_DIR="$PROJECT_DIR/qmk_firmware"
KEYMAP_NAME="custom"
KEYBOARD="zsa/voyager"
KEYMAP_DIR="$QMK_DIR/keyboards/$KEYBOARD/keymaps/$KEYMAP_NAME"

usage() {
    echo "Usage: $0 {sync|build|flash}"
    echo ""
    echo "  sync   — Copy oryx-export/ source files into QMK keymap directory"
    echo "  build  — Compile firmware (runs sync first)"
    echo "  flash  — Compile and flash firmware (runs sync first)"
    exit 1
}

sync_keymap() {
    echo "Syncing oryx-export/ → QMK keymap directory..."
    mkdir -p "$KEYMAP_DIR"
    cp "$PROJECT_DIR/oryx-export/keymap.c" "$KEYMAP_DIR/keymap.c"
    cp "$PROJECT_DIR/oryx-export/config.h" "$KEYMAP_DIR/config.h"
    cp "$PROJECT_DIR/oryx-export/rules.mk" "$KEYMAP_DIR/rules.mk"
    echo "Synced to $KEYMAP_DIR"
}

build_firmware() {
    sync_keymap
    echo "Building firmware..."
    cd "$QMK_DIR"
    make "$KEYBOARD:$KEYMAP_NAME"
    echo "Build complete."
}

flash_firmware() {
    sync_keymap
    echo "Building and flashing firmware..."
    echo ""
    echo ">>> Put your Voyager in bootloader mode (press the reset button) <<<"
    echo ""
    cd "$QMK_DIR"
    make "$KEYBOARD:$KEYMAP_NAME:flash"
}

case "${1:-}" in
    sync)  sync_keymap ;;
    build) build_firmware ;;
    flash) flash_firmware ;;
    *)     usage ;;
esac
```

**Step 2: Make executable and commit**

```bash
chmod +x ~/projects/zsa-voyager-rgb/build.sh
cd ~/projects/zsa-voyager-rgb && git add build.sh && git commit -m "Add build/flash helper script"
```

---

### Task 5: Fetch User's Layout and Verify

**Files:**
- Populate: `~/projects/zsa-voyager-rgb/rgb-schemes/layout.json`
- Populate: `~/projects/zsa-voyager-rgb/oryx-export/*`

**Step 1: Ask user for their Oryx layout URL**

Use AskUserQuestion to get the user's Oryx layout URL (e.g., `https://configure.zsa.io/voyager/layouts/AbCdE/latest/0`).

**Step 2: Run fetch script**

```bash
~/projects/zsa-voyager-rgb/scripts/fetch-layout.sh "<user's URL>"
```

Expected: Layout JSON saved, QMK source downloaded and extracted.

**Step 3: Verify source compiles**

```bash
cd ~/projects/zsa-voyager-rgb && ./build.sh build
```

Expected: Successful compilation with unmodified source.

**Step 4: Commit baseline**

```bash
cd ~/projects/zsa-voyager-rgb && git add oryx-export/ rgb-schemes/layout.json && git commit -m "Import layout from Oryx API (baseline)"
```

---

### Task 6: Analyze Keymap and Categorize Keys

**Files:**
- Read: `~/projects/zsa-voyager-rgb/rgb-schemes/layout.json`
- Create: `~/projects/zsa-voyager-rgb/rgb-schemes/keymap-analysis.json`

**Step 1: Parse layout.json**

Claude Code reads the layout JSON and for each layer, classifies every key (by array index 0-51) into categories based on:
- `tap.code` — primary keycode
- `hold.code` — hold action (modifier or layer toggle)
- `hold.layer` — target layer number (for MO, TG, etc.)

Categories: alpha, modifier, nav, symbol, numpad, function, media, layer_toggle, editing, rgb_control, transparent, unused.

**Step 2: Write keymap-analysis.json**

```json
{
  "keyboard": "voyager",
  "total_keys": 52,
  "layers": [
    {
      "index": 0,
      "name": "Main",
      "keys": [
        {"position": 0, "tap": "KC_EQUAL", "hold": "KC_ESCAPE", "category": "symbol", "holdCategory": "editing"},
        {"position": 1, "tap": "KC_1", "hold": null, "category": "alpha"},
        ...
      ],
      "summary": {"alpha": 30, "modifier": 8, "layer_toggle": 2, "editing": 4, "symbol": 8}
    }
  ]
}
```

**Step 3: Present summary to user**

Print per-layer category counts.

**Step 4: Commit**

```bash
cd ~/projects/zsa-voyager-rgb && git add rgb-schemes/keymap-analysis.json && git commit -m "Add keymap analysis"
```

---

### Task 7: Generate Color Schemes and Get User Selection

**Files:**
- Create: `~/projects/zsa-voyager-rgb/rgb-schemes/current.json`

**Step 1: Generate 3 color palettes**

Based on keymap analysis, create Functional, Minimal, and Vibrant schemes.

**Step 2: Render text-based Voyager previews**

For each scheme and layer, render the 52-key layout with color labels showing the actual key labels and assigned colors.

**Step 3: Present to user and get selection**

Use AskUserQuestion. User picks one or requests modifications.

**Step 4: Save and commit**

```bash
cd ~/projects/zsa-voyager-rgb && git add rgb-schemes/current.json && git commit -m "Add selected RGB color scheme"
```

---

### Task 8: Generate RGB C Code

**Files:**
- Modify: `~/projects/zsa-voyager-rgb/oryx-export/keymap.c`
- Modify: `~/projects/zsa-voyager-rgb/oryx-export/rules.mk`
- Modify: `~/projects/zsa-voyager-rgb/oryx-export/config.h`

**Step 1: Enable RGB Matrix in rules.mk**

Add `RGB_MATRIX_ENABLE = yes` if not present.

**Step 2: Add RGB defaults to config.h**

Add `#define RGB_MATRIX_STARTUP_VAL 128` and `#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200` if not present.

**Step 3: Generate rgb_matrix_indicators_advanced_user()**

Using keymap-analysis.json and current.json, generate the complete C function that maps each LED index to a color based on the current layer and key category.

**Step 4: Insert into keymap.c**

Add the function at the end of keymap.c (before any closing guards). Replace if it already exists.

**Step 5: Verify compilation**

```bash
cd ~/projects/zsa-voyager-rgb && ./build.sh build
```

Expected: Successful compilation.

**Step 6: Commit**

```bash
cd ~/projects/zsa-voyager-rgb && git add oryx-export/ && git commit -m "Add per-key per-layer RGB indicators"
```

---

### Task 9: Flash Firmware to Voyager

**Step 1: Instruct user to enter bootloader mode**

Tell user to press the Voyager's reset button (pinhole on bottom of left half).

**Step 2: Flash**

```bash
cd ~/projects/zsa-voyager-rgb && ./build.sh flash
```

**Step 3: Verify RGB works**

Ask user to confirm colors on each layer.

**Step 4: Tag working build**

```bash
cd ~/projects/zsa-voyager-rgb && git tag v1.0-initial-rgb
```

---

### Task 10: Document Iterative Workflow

**Files:**
- Modify: `~/projects/zsa-voyager-rgb/README.md`

**Step 1: Add workflow sections**

Document how to:
- Change RGB colors (tell Claude Code, regenerate, rebuild, flash)
- Update after Oryx layout changes (re-run fetch script, re-analyze, regenerate)
- Use the fetch script directly

**Step 2: Commit**

```bash
cd ~/projects/zsa-voyager-rgb && git add README.md && git commit -m "Document iterative RGB workflow"
```
