#!/usr/bin/env bash
set -Eeuo pipefail

REPO_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
case $(uname -m) in
    x86_64|i?86) ;;
    *) printf 'This launcher requires an x86 host.\n' >&2; exit 1 ;;
esac

# The original renderer remains an explicit diagnostic/compatibility choice.
if [[ ${1:-} == --original-renderer ]]; then
    shift
    export WINEPREFIX=${WINEPREFIX:-$REPO_DIR/working/wineprefix-$(uname -m)}
    exec "$REPO_DIR/tools/run-game.sh" "$@"
fi
exec "$REPO_DIR/run-native.sh" "$@"
