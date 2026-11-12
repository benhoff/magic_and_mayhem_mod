#!/usr/bin/env bash
set -Eeuo pipefail
REPO_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
# Help must work without Wine, a prepared installation or a storage reservation.
if [[ ${1:-} == --help || ${1:-} == -h ]]; then
    exec python3 -B "$REPO_DIR/tools/run-native-game.py" "$@"
fi
# Reserve initialization/build/session output and monitor the full launch.
DEFAULT_STORAGE_POLICY="$REPO_DIR/working/storage-policy-local.json"
if [[ ! -f "$DEFAULT_STORAGE_POLICY" ]]; then
    DEFAULT_STORAGE_POLICY="$REPO_DIR/tools/working-retention.json"
fi
export MNM_WORKING_STORAGE_POLICY="${MNM_WORKING_STORAGE_POLICY:-$DEFAULT_STORAGE_POLICY}"
exec python3 -B "$REPO_DIR/tools/working-retention.py" \
    --policy "$MNM_WORKING_STORAGE_POLICY" \
    run --reserve-gib 4 -- \
    python3 -B "$REPO_DIR/tools/run-native-game.py" "$@"
