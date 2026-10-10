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
    # One Python process avoids two subprocesses per original file. The path
    # order and TSV bytes match find -type f | LC_ALL=C sort -z exactly.
    python3 - "$REPO_DIR" "$ORIGINAL_DIR" "$1" <<'PYTHON'
import hashlib
import os
from pathlib import Path
import sys
repo, original, destination = map(Path, sys.argv[1:])
files = []
for directory, _, names in os.walk(original, followlinks=False):
    for name in names:
        path = Path(directory) / name
        if path.is_file() and not path.is_symlink():
            files.append(path)
files.sort(key=lambda path: os.fsencode(str(path)))
with destination.open('wb') as output:
    output.write(b'sha256\tsize\tpath\n')
    for path in files:
        size = path.stat().st_size
        with path.open('rb') as source:
            digest = hashlib.file_digest(source, 'sha256').hexdigest()
        relative = os.fsencode(str(path.relative_to(repo)))
        output.write(digest.encode() + b'\t' + str(size).encode() + b'\t' + relative + b'\n')
PYTHON
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
