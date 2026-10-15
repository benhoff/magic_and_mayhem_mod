#!/usr/bin/env bash
set -Eeuo pipefail
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
export MNM_RTS_CONTROLS=1
export MNM_VERIFY_ORIGINALS=1
export MNM_RTS_CONTROLS_CONFIG=${MNM_RTS_CONTROLS_CONFIG:-"$SCRIPT_DIR/../patches/rts-controls.json"}
exec python3 "$SCRIPT_DIR/working-retention.py" run --reserve-gib 2 -- python3 "$SCRIPT_DIR/run-rts-controls.py" "$@"
