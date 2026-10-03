#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
ASSUME_YES=0
SETUP_ONLY=0
MODE=launch
declare -a LAUNCH_ARGUMENTS=()

usage() {
    cat <<'EOF'
Usage: ./run-asahi.sh [launch|check|smoke] [options]

One entry point for Fedora Asahi Remix. It:

  1. Checks the host and required tools.
  2. Offers to install missing extraction tools from Fedora.
  3. Offers to enable the experimental lacamar/wine-arm64ec COPR and install
     Wine when no compatible runtime is present.
  4. Prepares/verifies the working game installation.
  5. Runs the requested launch, check, or smoke-test operation.

Options handled here:
  --setup-only   Prepare dependencies and game files without launching.
  --yes          Accept package/repository prompts non-interactively.
  -h, --help     Show this help.

Other options are forwarded to tools/run-game.sh, including --clean,
-w/--window, -s/--scale, -f/--fullscreen, --window-size, --seconds,
--size, --game-size, --gamescope-arg, --no-gamescope, --prefix, --wine,
and --runner. Gamescope is enabled by default.
EOF
}

die() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

confirm() {
    local prompt=$1 answer
    (( ASSUME_YES )) && return 0
    if [[ ! -t 0 ]]; then
        die "$prompt Rerun interactively or pass --yes."
    fi
    read -r -p "$prompt [y/N] " answer
    [[ $answer == [yY] || $answer == [yY][eE][sS] ]]
}

require_host() {
    [[ $(uname -m) == aarch64 ]] || die 'This entry point is specifically for aarch64 Fedora Asahi Remix'
    [[ -r /etc/os-release ]] || die 'Cannot identify the operating system'
    # shellcheck disable=SC1091
    . /etc/os-release
    case ${ID:-} in
        fedora|fedora-asahi-remix) ;;
        *) die "Expected Fedora Asahi Remix, found ${ID:-unknown}" ;;
    esac
    [[ $(getconf PAGESIZE) -eq 16384 ]] ||
        printf 'Warning: expected a 16 KiB Asahi kernel page size.\n' >&2
    command -v sudo >/dev/null 2>&1 || die 'sudo is required to install missing system packages'
    command -v dnf >/dev/null 2>&1 || die 'dnf is required to install missing system packages'
}

install_fedora_tools() {
    local -a packages=()
    command -v gamescope >/dev/null 2>&1 || packages+=(gamescope)
    command -v unshield >/dev/null 2>&1 || packages+=(unshield)
    command -v FEXBash >/dev/null 2>&1 || packages+=(fex-emu)
    command -v muvm >/dev/null 2>&1 || packages+=(muvm)
    (( ${#packages[@]} == 0 )) && return

    printf 'Missing Fedora packages: %s\n' "${packages[*]}"
    confirm 'Install these packages from the Fedora repositories?' || die 'Dependency installation cancelled'
    sudo dnf -y install "${packages[@]}"
}

install_arm64ec_wine() {
    command -v wine >/dev/null 2>&1 && return

    cat <<'EOF'

No Wine runtime is installed.

This host needs an ARM64EC/FEX-aware Wine build for 32-bit Windows software.
The available solution uses the third-party lacamar/wine-arm64ec COPR. It is
experimental and may not run every application. Enabling it changes the system's
package sources and installing it adds Wine system-wide.
EOF
    confirm 'Enable lacamar/wine-arm64ec and install fex-emu-wine plus wine?' ||
        die 'Wine installation cancelled'
    sudo dnf -y copr enable lacamar/wine-arm64ec
    sudo dnf -y install fex-emu-wine wine
    command -v wine >/dev/null 2>&1 || die 'Wine installation completed without providing a wine command'
}

if [[ $# -gt 0 ]]; then
    case $1 in
        launch|check|smoke) MODE=$1; shift ;;
        -h|--help|help) usage; exit 0 ;;
    esac
fi

while [[ $# -gt 0 ]]; do
    case $1 in
        --setup-only) SETUP_ONLY=1; shift ;;
        --yes) ASSUME_YES=1; LAUNCH_ARGUMENTS+=(--yes); shift ;;
        -h|--help|help) usage; exit 0 ;;
        *) LAUNCH_ARGUMENTS+=("$1"); shift ;;
    esac
done

require_host
install_fedora_tools
install_arm64ec_wine
if [[ ! -d $SCRIPT_DIR/working/game-clean || ! -d $SCRIPT_DIR/working/game-nocd ]]; then
    "$SCRIPT_DIR/tools/prepare-working.sh"
else
    "$SCRIPT_DIR/tools/original-manifest.sh"
fi

if (( SETUP_ONLY )); then
    printf 'Asahi runtime and working installation are ready.\n'
    exit 0
fi

exec "$SCRIPT_DIR/tools/run-game.sh" "$MODE" "${LAUNCH_ARGUMENTS[@]}"
