#!/usr/bin/env bash
set -Eeuo pipefail

REPO_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
case $(uname -m) in
    x86_64|i?86) ;;
    *) printf 'This launcher requires an x86 host.\n' >&2; exit 1 ;;
esac

# Keep the prefix separate from the ARM64 Wine installation brought from Asahi.
# Explicit WINEPREFIX and run-game.sh --prefix overrides remain supported.
export WINEPREFIX=${WINEPREFIX:-$REPO_DIR/working/wineprefix-$(uname -m)}
exec "$REPO_DIR/tools/run-game.sh" "$@"
