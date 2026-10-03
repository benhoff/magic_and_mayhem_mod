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
# remain enabled. The search model uses standard containers for host scratch.
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" "$BUILD_DIR/route-request-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/tests/route-search-test.cpp" -o "$BUILD_DIR/route-search-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" "$BUILD_DIR/route-search-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/tests/route-neighbors-test.cpp" -o "$BUILD_DIR/route-neighbors-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" "$BUILD_DIR/route-neighbors-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/tests/route-creature-acceptance-test.cpp" -o "$BUILD_DIR/route-creature-acceptance-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-creature-acceptance-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/tests/route-movement-test.cpp" -o "$BUILD_DIR/route-movement-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-movement-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/tests/route-cell-test.cpp" -o "$BUILD_DIR/route-cell-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-cell-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/tests/route-occupancy-test.cpp" -o "$BUILD_DIR/route-occupancy-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-occupancy-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/tests/route-validity-test.cpp" -o "$BUILD_DIR/route-validity-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-validity-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_validity.cpp" \
    "$REPO_DIR/tests/route-cell-validity-test.cpp" -o "$BUILD_DIR/route-cell-validity-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-cell-validity-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_record_query.cpp" \
    "$REPO_DIR/tests/route-record-query-test.cpp" -o "$BUILD_DIR/route-record-query-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-record-query-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_record_query.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_support.cpp" \
    "$REPO_DIR/tests/route-cell-support-test.cpp" -o "$BUILD_DIR/route-cell-support-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-cell-support-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_record_query.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_support.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_boundary.cpp" \
    "$REPO_DIR/tests/route-boundary-test.cpp" -o "$BUILD_DIR/route-boundary-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-boundary-test"
"${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I "$REPO_DIR/reconstruction/pathfinding" \
    "$REPO_DIR/reconstruction/pathfinding/route_request.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_search.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_neighbors.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_creature_acceptance.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_movement.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_occupancy.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_validity.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_record_query.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_cell_support.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_boundary.cpp" \
    "$REPO_DIR/reconstruction/pathfinding/route_clearance.cpp" \
    "$REPO_DIR/tests/route-clearance-test.cpp" -o "$BUILD_DIR/route-clearance-test"
ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
    "$BUILD_DIR/route-clearance-test"
for COMPONENT in scalar world; do
    "${CXX:-g++}" -std=c++17 -Wall -Wextra -Werror -pedantic -g \
        -fsanitize=address,undefined -fno-omit-frame-pointer \
        -I "$REPO_DIR/reconstruction/pathfinding" \
        "$REPO_DIR"/reconstruction/pathfinding/route_{request,search,neighbors,creature_acceptance,movement,cell,occupancy,validity,cell_validity,record_query,cell_support,boundary,clearance,scalar,world}.cpp \
        "$REPO_DIR/tests/route-$COMPONENT-test.cpp" -o "$BUILD_DIR/route-$COMPONENT-test"
    ASAN_OPTIONS="${ASAN_OPTIONS:+$ASAN_OPTIONS:}detect_leaks=0" \
        UBSAN_OPTIONS="${UBSAN_OPTIONS:+$UBSAN_OPTIONS:}halt_on_error=1" \
        "$BUILD_DIR/route-$COMPONENT-test"
done
printf 'Reconstruction tests passed (address/undefined-behavior sanitizers).\n'
