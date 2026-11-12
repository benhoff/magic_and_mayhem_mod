#!/usr/bin/env bash
set -Eeuo pipefail
REPO_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
# Help must work without Wine, a prepared installation or a storage reservation.
if [[ ${1:-} == --help || ${1:-} == -h ]]; then
    exec python3 -B "$REPO_DIR/tools/run-native-game.py" "$@"
fi
# Reserve initialization/build/session output and monitor the full launch.
exec python3 -B "$REPO_DIR/tools/working-retention.py" \
    --policy "${MNM_WORKING_STORAGE_POLICY:-$REPO_DIR/tools/working-retention.json}" \
    run --reserve-gib 4 -- \
    python3 -B "$REPO_DIR/tools/run-native-game.py" "$@"
