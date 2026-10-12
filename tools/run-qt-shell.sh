#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
BUILD_DIR="$REPO_DIR/working/build/qt-shell"
BUILD_TESTS=${MNM_QT_BUILD_TESTS:-OFF}
case "$BUILD_TESTS" in
    ON|OFF) ;;
    *) echo "MNM_QT_BUILD_TESTS must be ON or OFF" >&2; exit 2 ;;
esac
cmake -S "$REPO_DIR/apps/qt-shell" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="${MNM_QT_BUILD_TYPE:-RelWithDebInfo}" -DBUILD_TESTING="$BUILD_TESTS"
if [[ "$BUILD_TESTS" == ON ]]; then
    cmake --build "$BUILD_DIR" --parallel 4
else
    cmake --build "$BUILD_DIR" --target mnm-qt-shell --parallel 4
fi
# Wine and the shell must share the X11 display for foreign-window hosting.
export QT_QPA_PLATFORM=${QT_QPA_PLATFORM:-xcb}
# Public native sessions use the guarded glyph adapter unless explicitly disabled.
# Standalone observation/comparison launchers keep their own mode selection.
export MNM_GLYPH_BACKEND=${MNM_GLYPH_BACKEND-takeover}
exec "$BUILD_DIR/mnm-qt-shell" --repo "$REPO_DIR" "$@"
