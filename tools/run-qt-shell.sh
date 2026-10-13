#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
BUILD_DIR="$REPO_DIR/working/build/qt-shell"
BUILD_TESTS=${MNM_QT_BUILD_TESTS:-OFF}
BUILD_TYPE=${MNM_QT_BUILD_TYPE:-RelWithDebInfo}
case "$BUILD_TESTS" in
    ON|OFF) ;;
    *) echo "MNM_QT_BUILD_TESTS must be ON or OFF" >&2; exit 2 ;;
esac
cache_value() {
    local name=$1
    [[ -f "$BUILD_DIR/CMakeCache.txt" ]] || return 0
    sed -n "s/^${name}:[^=]*=//p" "$BUILD_DIR/CMakeCache.txt"
}
if [[ ${MNM_QT_RECONFIGURE:-0} == 1 ||
      $(cache_value CMAKE_HOME_DIRECTORY) != "$REPO_DIR/apps/qt-shell" ||
      $(cache_value CMAKE_BUILD_TYPE) != "$BUILD_TYPE" ||
      $(cache_value BUILD_TESTING) != "$BUILD_TESTS" ||
      ! -f "$BUILD_DIR/Makefile" && ! -f "$BUILD_DIR/build.ninja" ]]; then
    cmake -S "$REPO_DIR/apps/qt-shell" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTING="$BUILD_TESTS"
fi
# CMake's generated build still checks dependencies and regenerates when needed.
if [[ "$BUILD_TESTS" == ON ]]; then
    cmake --build "$BUILD_DIR" --parallel 4
else
    cmake --build "$BUILD_DIR" --target mnm-qt-shell --parallel 4
fi
# Wine and the shell must share the X11 display for foreign-window hosting.
export QT_QPA_PLATFORM=${QT_QPA_PLATFORM:-xcb}
# Ordinary launches skip the full original-media scan; diagnostics opt in.
export MNM_VERIFY_ORIGINALS=${MNM_VERIFY_ORIGINALS:-0}
export MNM_VERIFY_ASSETS=${MNM_VERIFY_ASSETS:-0}
export MNM_CACHE_ADAPTERS=${MNM_CACHE_ADAPTERS:-1}
# Public native sessions use the guarded glyph adapter unless explicitly disabled.
# Standalone observation/comparison launchers keep their own mode selection.
export MNM_GLYPH_BACKEND=${MNM_GLYPH_BACKEND-takeover}
exec "$BUILD_DIR/mnm-qt-shell" --repo "$REPO_DIR" "$@"
