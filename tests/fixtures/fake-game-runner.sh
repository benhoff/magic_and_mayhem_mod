#!/usr/bin/env bash

set -Eeuo pipefail

game_executable=${1:-}
[[ -f $game_executable ]] || {
    printf 'Expected a game executable as the first argument.\n' >&2
    exit 2
}

sleep "${MNM_FAKE_RUN_SECONDS:-60}"

