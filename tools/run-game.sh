#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
WORKING_DIR="$REPO_DIR/working"
LOG_ROOT="$WORKING_DIR/logs"
RUNTIME_ROOT="$WORKING_DIR/runtime"
MODE=launch
VARIANT=nocd
SMOKE_SECONDS=20
ASSUME_YES=0
WINE_BINARY=${MNM_WINE_BIN:-}
RUNNER_WRAPPER=${MNM_RUNNER:-}
PREFIX_PATH=${WINEPREFIX:-$WORKING_DIR/wineprefix}
DISPLAY_MODE=fullscreen
DESKTOP_SIZE=
USE_GAMESCOPE=1
GAMESCOPE_BINARY=${MNM_GAMESCOPE_BIN:-gamescope}
GAME_SIZE=800x600
OUTPUT_SIZE=1280x960
declare -a GAMESCOPE_ARGUMENTS=()
declare -a GAME_ARGUMENTS=()
declare -a RUN_COMMAND=()

usage() {
    cat <<'EOF'
Usage: ./tools/run-game.sh [launch|check|smoke] [options] [-- GAME_ARGUMENTS...]

Commands:
  launch              Validate and launch the no-CD working build (default).
  check               Validate files, display access, and runtime without launch.
  smoke               Launch for a bounded interval; staying alive is a pass.

Options:
  --clean              Use the retail executable instead of the no-CD build.
  -w, --window         Run in a 1280x960 gamescope window.
  -s, --scale          Size the output to the current display.
  -f, --fullscreen     Fullscreen gamescope output (default).
  --size WxH           Output size (default: 1280x960); keeps the display mode.
  --window-size WxH    Set output size and select windowed mode.
  --game-size WxH      Gamescope virtual display size (default: 800x600).
  --gamescope          Enable gamescope (default; fit + nearest scaling).
  --no-gamescope       Use Wine directly; -w/-s use a Wine virtual desktop.
  --gamescope-arg ARG   Append one gamescope argument; repeat for extra options.
  --wine PATH          Wine executable. On aarch64 it is run through muvm.
  --runner PATH        Custom wrapper that accepts Chaos.exe and its arguments.
  --prefix DIRECTORY   Wine prefix (default: working/wineprefix).
  --seconds NUMBER     Smoke-test duration (default: 20).
  --yes                Confirm use of a prefix outside working/ non-interactively.
  -h, --help           Show this help.

Environment equivalents: MNM_WINE_BIN, MNM_RUNNER, WINEPREFIX,
and MNM_GAMESCOPE_BIN (gamescope executable).

Examples:
  ./run-x86.sh -w --size 1600x1200
  ./run-x86.sh -f --game-size 640x480
  ./run-x86.sh --gamescope-arg -r --gamescope-arg 60
  ./run-x86.sh --no-gamescope -w

With --no-gamescope, --size selects a Wine desktop window; --game-size and
--gamescope-arg require gamescope. Arguments after -- go to the game.
EOF
}

die() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

manifest_digest() {
    local manifest=$1 relative=$2
    awk -F '\t' -v path="$relative" '$3 == path { print $1; exit }' "$manifest"
}

verify_digest() {
    local file=$1 expected=$2 actual
    [[ -n $expected ]] || die "No recorded digest for ${file##*/}"
    actual=$(sha256sum -- "$file")
    actual=${actual%% *}
    [[ $actual == "$expected" ]] || die "Hash mismatch: $file"
}

confirm_external_prefix() {
    local answer
    case $PREFIX_PATH in
        "$WORKING_DIR"|"$WORKING_DIR"/*) return 0 ;;
    esac
    (( ASSUME_YES )) && return 0
    if [[ ! -t 0 ]]; then
        die 'Wine prefix is outside working/; rerun interactively or pass --yes'
    fi
    read -r -p "Use Wine prefix outside the workspace at $PREFIX_PATH? [y/N] " answer
    [[ $answer == [yY] || $answer == [yY][eE][sS] ]] || die 'Launch cancelled'
}

validate_size() {
    [[ $1 =~ ^[1-9][0-9]*x[1-9][0-9]*$ ]] || die "Invalid display size: $1 (expected WIDTHxHEIGHT)"
}

detect_display_size() {
    local detected=
    if [[ -n ${MNM_DISPLAY_SIZE:-} ]]; then
        detected=$MNM_DISPLAY_SIZE
    elif command -v kscreen-doctor >/dev/null 2>&1; then
        detected=$(kscreen-doctor -o 2>/dev/null |
            sed -nE 's/.*Geometry: [^ ]+ ([0-9]+x[0-9]+).*/\1/p' | head -n 1)
    fi
    if [[ -z $detected ]] && command -v xrandr >/dev/null 2>&1; then
        detected=$(xrandr --current 2>/dev/null |
            sed -nE 's/.* connected( primary)? ([0-9]+x[0-9]+)\+.*/\2/p' | head -n 1)
    fi
    [[ -n $detected ]] || die 'Could not detect display size; use --window-size WIDTHxHEIGHT or set MNM_DISPLAY_SIZE'
    validate_size "$detected"
    DESKTOP_SIZE=$detected
}

select_game() {
    if [[ $VARIANT == clean ]]; then
        SOURCE_GAME_DIR="$WORKING_DIR/game-clean"
        GAME_MANIFEST="$WORKING_DIR/manifests/game-clean.tsv"
    else
        SOURCE_GAME_DIR="$WORKING_DIR/game-nocd"
        GAME_MANIFEST="$WORKING_DIR/manifests/game-nocd.tsv"
    fi
    GAME_DIR=$SOURCE_GAME_DIR
    GAME_EXE="$SOURCE_GAME_DIR/Chaos.exe"
}

verify_game() {
    local required expected
    if [[ ! -d $SOURCE_GAME_DIR ]]; then
        printf 'Working installation is missing; preparing it now.\n'
        "$SCRIPT_DIR/prepare-working.sh"
    fi
    [[ -f $GAME_MANIFEST ]] || die "Missing installation manifest: $GAME_MANIFEST"

    for required in Chaos.exe jpeg.dll CDROMDrive.cfg; do
        [[ -f $SOURCE_GAME_DIR/$required ]] || die "Required game file is missing: $SOURCE_GAME_DIR/$required"
        expected=$(manifest_digest "$GAME_MANIFEST" "$required")
        verify_digest "$SOURCE_GAME_DIR/$required" "$expected"
    done
    for required in CFG Creatures Interface Realms; do
        [[ -d $SOURCE_GAME_DIR/$required ]] || die "Required game directory is missing: $SOURCE_GAME_DIR/$required"
    done
    file -- "$SOURCE_GAME_DIR/Chaos.exe" | grep -q 'PE32 executable.*Intel i386' ||
        die 'Chaos.exe is not the expected 32-bit x86 Windows executable'
}

select_runtime_tree() {
    local runtime_dir="$RUNTIME_ROOT/game-$VARIANT"
    if [[ ! -d $runtime_dir ]]; then
        mkdir -p -- "$RUNTIME_ROOT"
        printf 'Creating writable runtime copy at %s\n' "${runtime_dir#"$REPO_DIR/"}"
        cp -a --reflink=auto -- "$SOURCE_GAME_DIR" "$runtime_dir"
    fi
    GAME_DIR=$runtime_dir
    GAME_EXE="$GAME_DIR/Chaos.exe"
}

resolve_runtime() {
    local host_arch wine_candidate wine_format
    local -a wine_arguments=()
    host_arch=$(uname -m)

    if [[ -n $RUNNER_WRAPPER ]]; then
        [[ $USE_GAMESCOPE == 1 || $DISPLAY_MODE == fullscreen ]] ||
            die 'Window and scaling flags require Wine directly; they cannot be combined with --runner'
        command -v "$RUNNER_WRAPPER" >/dev/null 2>&1 ||
            [[ -x $RUNNER_WRAPPER ]] || die "Custom runner is not executable: $RUNNER_WRAPPER"
        if [[ $RUNNER_WRAPPER == */* ]]; then
            RUNNER_WRAPPER=$(realpath -e -- "$RUNNER_WRAPPER")
        else
            RUNNER_WRAPPER=$(command -v "$RUNNER_WRAPPER")
        fi
        RUN_COMMAND=("$RUNNER_WRAPPER" "$GAME_EXE" "${GAME_ARGUMENTS[@]}")
        RUNTIME_DESCRIPTION="custom runner: $RUNNER_WRAPPER"
        return
    fi

    if [[ -n $WINE_BINARY ]]; then
        wine_candidate=$WINE_BINARY
    elif command -v wine >/dev/null 2>&1; then
        wine_candidate=$(command -v wine)
    else
        if [[ $host_arch == aarch64 ]]; then
            die 'No Wine runtime found. On Fedora Asahi, set MNM_WINE_BIN to an x86 Wine binary that muvm/FEX can run, or use --runner with a Proton/Wine wrapper.'
        fi
        die 'Wine is not installed or not on PATH'
    fi

    command -v "$wine_candidate" >/dev/null 2>&1 ||
        [[ -x $wine_candidate ]] || die "Wine executable is not available: $wine_candidate"
    if [[ $wine_candidate == */* ]]; then
        wine_candidate=$(realpath -e -- "$wine_candidate")
    else
        wine_candidate=$(command -v "$wine_candidate")
    fi

    if (( ! USE_GAMESCOPE )) && [[ $DISPLAY_MODE != fullscreen ]]; then
        wine_arguments=(explorer "/desktop=MagicMayhem,$DESKTOP_SIZE" "$GAME_EXE" "${GAME_ARGUMENTS[@]}")
    else
        wine_arguments=("$GAME_EXE" "${GAME_ARGUMENTS[@]}")
    fi

    wine_format=$(file -L -b -- "$wine_candidate")
    if [[ $host_arch == aarch64 && $wine_format =~ (x86-64|80386) ]]; then
        command -v muvm >/dev/null 2>&1 ||
            die 'muvm is required to run x86 Wine on this aarch64 host'
        command -v FEXBash >/dev/null 2>&1 ||
            die 'FEX is required to run x86 Wine on this aarch64 host'
        RUN_COMMAND=(muvm -- "$wine_candidate" "${wine_arguments[@]}")
        RUNTIME_DESCRIPTION="muvm/FEX with Wine: $wine_candidate"
    else
        RUN_COMMAND=("$wine_candidate" "${wine_arguments[@]}")
        RUNTIME_DESCRIPTION="native Wine: $wine_candidate"
    fi
}

wrap_gamescope() {
    (( USE_GAMESCOPE )) || return 0
    command -v "$GAMESCOPE_BINARY" >/dev/null 2>&1 ||
        [[ -x $GAMESCOPE_BINARY ]] ||
        die 'gamescope is not available; install gamescope or use --no-gamescope'
    if [[ $GAMESCOPE_BINARY == */* ]]; then
        GAMESCOPE_BINARY=$(realpath -e -- "$GAMESCOPE_BINARY")
    else
        GAMESCOPE_BINARY=$(command -v "$GAMESCOPE_BINARY")
    fi
    local -a scope_arguments=(
        -w "${GAME_SIZE%x*}" -h "${GAME_SIZE#*x}"
        -W "${OUTPUT_SIZE%x*}" -H "${OUTPUT_SIZE#*x}"
        -S fit -F nearest
    )
    [[ $DISPLAY_MODE != fullscreen ]] || scope_arguments+=(-f)
    RUN_COMMAND=("$GAMESCOPE_BINARY" "${scope_arguments[@]}"
        "${GAMESCOPE_ARGUMENTS[@]}" -- "${RUN_COMMAND[@]}")
    RUNTIME_DESCRIPTION="gamescope ($GAME_SIZE -> $OUTPUT_SIZE), $RUNTIME_DESCRIPTION"
}

preflight() {
    command -v file >/dev/null 2>&1 || die 'file is required for executable validation'
    command -v realpath >/dev/null 2>&1 || die 'realpath is required for runner validation'
    command -v sha256sum >/dev/null 2>&1 || die 'sha256sum is required for hash validation'
    select_game
    verify_game
    [[ $MODE == check ]] || select_runtime_tree
    case $DISPLAY_MODE in
        window) [[ -n $DESKTOP_SIZE ]] || DESKTOP_SIZE=1280x960 ;;
        scale) detect_display_size ;;
    esac
    if (( USE_GAMESCOPE )) && [[ $DISPLAY_MODE == scale ]]; then
        OUTPUT_SIZE=$DESKTOP_SIZE
    fi
    [[ -n ${DISPLAY:-} || -n ${WAYLAND_DISPLAY:-} ]] ||
        die 'No graphical display is available (DISPLAY and WAYLAND_DISPLAY are unset)'
    confirm_external_prefix
    resolve_runtime
    wrap_gamescope
}

print_command() {
    printf 'Command:'
    printf ' %q' "${RUN_COMMAND[@]}"
    printf '\n'
}

write_environment_log() {
    local log_dir=$1 exe_hash
    exe_hash=$(sha256sum -- "$GAME_EXE")
    exe_hash=${exe_hash%% *}
    {
        printf 'timestamp_utc=%s\n' "$(date -u +'%Y-%m-%dT%H:%M:%SZ')"
        printf 'repository=%s\n' "$REPO_DIR"
        printf 'game_variant=%s\n' "$VARIANT"
        printf 'source_game_directory=%s\n' "$SOURCE_GAME_DIR"
        printf 'game_directory=%s\n' "$GAME_DIR"
        printf 'executable_sha256=%s\n' "$exe_hash"
        printf 'runtime=%s\n' "$RUNTIME_DESCRIPTION"
        printf 'display_mode=%s\n' "$DISPLAY_MODE"
        printf 'desktop_size=%s\n' "${DESKTOP_SIZE:-native}"
        printf 'gamescope=%s\n' "$USE_GAMESCOPE"
        printf 'game_size=%s\n' "$GAME_SIZE"
        printf 'output_size=%s\n' "$OUTPUT_SIZE"
        printf 'wineprefix=%s\n' "$PREFIX_PATH"
        printf 'host_arch=%s\n' "$(uname -m)"
        printf 'kernel=%s\n' "$(uname -sr)"
        printf 'display=%s\n' "${DISPLAY:-}"
        printf 'wayland_display=%s\n' "${WAYLAND_DISPLAY:-}"
        printf 'command='
        printf '%q ' "${RUN_COMMAND[@]}"
        printf '\n'
    } >"$log_dir/environment.txt"
}

run_game() {
    local timestamp log_dir status=0
    timestamp=$(date -u +'%Y%m%dT%H%M%SZ')
    mkdir -p -- "$LOG_ROOT" "$PREFIX_PATH"
    log_dir=$(mktemp -d "$LOG_ROOT/run-${timestamp}.XXXXXX")
    write_environment_log "$log_dir"
    printf 'Runtime: %s\nLogs: %s\n' "$RUNTIME_DESCRIPTION" "${log_dir#"$REPO_DIR/"}"
    print_command

    if [[ $MODE == smoke ]]; then
        set +e
        (
            cd -- "$GAME_DIR"
            export WINEPREFIX="$PREFIX_PATH"
            export WINEDEBUG=${WINEDEBUG:-fixme-all}
            timeout --signal=TERM --kill-after=5s "${SMOKE_SECONDS}s" \
                "${RUN_COMMAND[@]}"
        ) >"$log_dir/stdout.log" 2>"$log_dir/stderr.log"
        status=$?
        set -e
        if [[ $status -eq 124 || $status -eq 137 ]]; then
            printf 'Smoke test passed: the game remained alive for %s seconds.\n' "$SMOKE_SECONDS"
            return 0
        fi
        die "Smoke test failed: the game exited early with status $status; inspect $log_dir"
    fi

    (
        cd -- "$GAME_DIR"
        export WINEPREFIX="$PREFIX_PATH"
        export WINEDEBUG=${WINEDEBUG:-fixme-all}
        "${RUN_COMMAND[@]}"
    ) > >(tee "$log_dir/stdout.log") 2> >(tee "$log_dir/stderr.log" >&2)
}

if [[ $# -gt 0 ]]; then
    case $1 in
        launch|check|smoke) MODE=$1; shift ;;
        -h|--help|help) usage; exit 0 ;;
    esac
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean) VARIANT=clean; shift ;;
        -w|--window) DISPLAY_MODE=window; DESKTOP_SIZE=; shift ;;
        -s|--scale) DISPLAY_MODE=scale; DESKTOP_SIZE=; shift ;;
        -f|--fullscreen) DISPLAY_MODE=fullscreen; DESKTOP_SIZE=; shift ;;
        --gamescope) USE_GAMESCOPE=1; shift ;;
        --no-gamescope) USE_GAMESCOPE=0; shift ;;
        --size)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            validate_size "$2"
            OUTPUT_SIZE=$2; DESKTOP_SIZE=$2
            shift 2
            ;;
        --game-size)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            validate_size "$2"
            GAME_SIZE=$2; shift 2
            ;;
        --gamescope-arg)
            [[ $# -ge 2 && -n $2 && $2 != -- ]] || die '--gamescope-arg requires one argument (other than --)'
            GAMESCOPE_ARGUMENTS+=("$2"); shift 2
            ;;
        --window-size)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            validate_size "$2"
            DISPLAY_MODE=window; DESKTOP_SIZE=$2; OUTPUT_SIZE=$2; shift 2
            ;;
        --wine)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            WINE_BINARY=$2; shift 2
            ;;
        --runner)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            RUNNER_WRAPPER=$2; shift 2
            ;;
        --prefix)
            [[ $# -ge 2 ]] || { usage >&2; exit 2; }
            PREFIX_PATH=$2; shift 2
            ;;
        --seconds)
            [[ $# -ge 2 && $2 =~ ^[1-9][0-9]*$ ]] || die '--seconds requires a positive integer'
            SMOKE_SECONDS=$2; shift 2
            ;;
        --yes) ASSUME_YES=1; shift ;;
        --) shift; GAME_ARGUMENTS=("$@"); break ;;
        -h|--help|help) usage; exit 0 ;;
        *) usage >&2; exit 2 ;;
    esac
done

if (( ! USE_GAMESCOPE )); then
    # Direct Wine has no separate output size; use a virtual desktop.
    [[ $DISPLAY_MODE != fullscreen || -z $DESKTOP_SIZE ]] || DISPLAY_MODE=window
    [[ $GAME_SIZE == 800x600 && ${#GAMESCOPE_ARGUMENTS[@]} == 0 ]] ||
        die '--game-size and --gamescope-arg require gamescope'
fi

preflight
printf 'Preflight passed for working/game-%s.\n' "$VARIANT"
printf 'Runtime: %s\n' "$RUNTIME_DESCRIPTION"
print_command

[[ $MODE == check ]] || run_game
