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
    cp "$PROJECT_DIR/oryx-export/keymap.json" "$KEYMAP_DIR/keymap.json"
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
