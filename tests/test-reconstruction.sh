#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
mkdir -p -- "$REPO_DIR/working/tests"
BUILD_DIR=$(mktemp -d "$REPO_DIR/working/tests/reconstruction.XXXXXXXX")
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/tests/route-request-test.cpp" -o "$BUILD_DIR/route-request-test"
# LeakSanitizer requires ptrace unavailable in the Codex sandbox; ASan/UBSan
# remain enabled. This model owns no heap allocations requiring leak checks.
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" "$BUILD_DIR/route-request-test"
printf 'Reconstruction tests passed (address/undefined-behavior sanitizers).\n'
