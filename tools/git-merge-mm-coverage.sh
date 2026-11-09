#!/bin/sh
set -eu
exec python3 -B "$(dirname "$0")/merge_coverage_batch.py" "$@"
