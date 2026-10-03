#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
BUILD_DIR="$REPO_DIR/working/build/qt-shell"
cmake -S "$REPO_DIR/apps/qt-shell" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --parallel 4
# Wine and the shell must share the X11 display for foreign-window hosting.
export QT_QPA_PLATFORM=${QT_QPA_PLATFORM:-xcb}
exec "$BUILD_DIR/mnm-qt-shell" --repo "$REPO_DIR" "$@"
