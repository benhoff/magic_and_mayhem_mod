#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)
ORIGINAL_DIR="$REPO_DIR/original"
MANIFEST="$REPO_DIR/research/original-files.tsv"
TEMPORARY_PATH=
ASSUME_YES=0

cleanup() {
    if [[ -n $TEMPORARY_PATH ]]; then
        rm -f -- "$TEMPORARY_PATH"
    fi
}

usage() {
    cat <<'EOF'
Usage: ./tools/original-manifest.sh [verify|refresh] [--yes]

  verify   Verify every original file's path, size, and SHA-256 (default).
  refresh  Regenerate research/original-files.tsv from original/.
  --yes    Confirm replacement without an interactive prompt.

The script never writes to original/.
EOF
}

confirm_replacement() {
    local answer
    (( ASSUME_YES )) && return 0
    if [[ ! -t 0 ]]; then
        printf 'Refusing to replace %s without a terminal; rerun with --yes.\n' \
            "${MANIFEST#"$REPO_DIR/"}" >&2
        return 1
    fi
    read -r -p "Replace ${MANIFEST#"$REPO_DIR/"} with a new baseline? [y/N] " answer
    [[ $answer == [yY] || $answer == [yY][eE][sS] ]]
}

snapshot() {
    local destination=$1 file relative size digest

    printf 'sha256\tsize\tpath\n' >"$destination"
    while IFS= read -r -d '' file; do
        relative=${file#"$REPO_DIR/"}
        size=$(stat -c '%s' -- "$file")
        digest=$(sha256sum -- "$file")
        digest=${digest%% *}
        printf '%s\t%s\t%s\n' "$digest" "$size" "$relative" >>"$destination"
    done < <(find "$ORIGINAL_DIR" -type f -print0 | LC_ALL=C sort -z)
}

refresh() {
    mkdir -p -- "$(dirname -- "$MANIFEST")"
    if [[ -e $MANIFEST ]] && ! confirm_replacement; then
        printf 'Manifest refresh cancelled.\n' >&2
        return 1
    fi
    TEMPORARY_PATH=$(mktemp "${MANIFEST}.partial.XXXXXX")
    trap cleanup EXIT
    snapshot "$TEMPORARY_PATH"
    mv -f -- "$TEMPORARY_PATH" "$MANIFEST"
    TEMPORARY_PATH=
    chmod 0644 -- "$MANIFEST"
    trap - EXIT
    printf 'Recorded %s original files in %s\n' \
        "$(( $(wc -l <"$MANIFEST") - 1 ))" "${MANIFEST#"$REPO_DIR/"}"
}

verify() {
    [[ -f $MANIFEST ]] || {
        printf 'Manifest not found: %s\nRun this once: %s refresh\n' \
            "$MANIFEST" "$0" >&2
        exit 1
    }

    TEMPORARY_PATH=$(mktemp "${MANIFEST}.verify.XXXXXX")
    trap cleanup EXIT
    snapshot "$TEMPORARY_PATH"
    if ! cmp -s -- "$MANIFEST" "$TEMPORARY_PATH"; then
        printf 'Original artifact verification failed. Differences follow:\n' >&2
        diff -u -- "$MANIFEST" "$TEMPORARY_PATH" >&2 || true
        exit 1
    fi
    cleanup
    TEMPORARY_PATH=
    trap - EXIT
    printf 'Verified %s original files against %s\n' \
        "$(( $(wc -l <"$MANIFEST") - 1 ))" "${MANIFEST#"$REPO_DIR/"}"
}

action=verify
for argument in "$@"; do
    case $argument in
        verify|refresh) action=$argument ;;
        --yes) ASSUME_YES=1 ;;
        -h|--help|help) usage; exit 0 ;;
        *) usage >&2; exit 2 ;;
    esac
done

case $action in
    verify) verify ;;
    refresh) refresh ;;
    *) usage >&2; exit 2 ;;
esac
