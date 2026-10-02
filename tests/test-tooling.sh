#!/usr/bin/env bash

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
REPO_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd -P)

bash -n "$REPO_DIR/tools/original-manifest.sh"
bash -n "$REPO_DIR/tools/prepare-working.sh"
bash -n "$REPO_DIR/tools/run-game.sh"
bash -n "$REPO_DIR/run-asahi.sh"
python3 -m py_compile \
    "$REPO_DIR/tools/cfg-precedence-experiment.py" \
    "$REPO_DIR/tools/cfg-writer-experiment.py" \
    "$REPO_DIR/tools/decode-cfg.py" \
    "$REPO_DIR/tools/encode-cfg.py" \
    "$REPO_DIR/tools/decompile-game.py" \
    "$REPO_DIR/tools/inventory-game-files.py"
python3 "$REPO_DIR/tests/test-decompile-game.py"
python3 "$REPO_DIR/tools/verify-decompilation-baseline.py"
python3 "$REPO_DIR/tests/test-decompilation-baseline.py"
python3 "$REPO_DIR/tests/test-route-trace.py"
"$REPO_DIR/tools/trace-route-experiment.py" --help >/dev/null
"$REPO_DIR/tools/validate-route-trace.py" --help >/dev/null
"$REPO_DIR/tests/test-reconstruction.sh"
"$REPO_DIR/run-asahi.sh" --help >/dev/null
"$REPO_DIR/tools/cfg-precedence-experiment.py" --help >/dev/null
"$REPO_DIR/tools/cfg-writer-experiment.py" --help >/dev/null
encode_preview=$("$REPO_DIR/tools/encode-cfg.py")
grep -Fq 'mode=0' <<<"$encode_preview"
grep -Fq 'round_trip=valid' <<<"$encode_preview"
"$REPO_DIR/tools/run-game.sh" --help >/dev/null
cfg_report=$("$REPO_DIR/tools/decode-cfg.py")
[[ $(grep -c $'\tvalid$' <<<"$cfg_report") -eq 11 ]]
grep -Fq $'creature.cfg\t2\t4878\t21599' <<<"$cfg_report"
inventory_report=$("$REPO_DIR/tools/inventory-game-files.py")
grep -Fq 'Files: 4834' <<<"$inventory_report"
decode_test_dir=$(mktemp -d)
trap 'rm -rf -- "$decode_test_dir"' EXIT
"$REPO_DIR/tools/encode-cfg.py" \
    "$REPO_DIR/working/runtime/game-nocd/CFG/tables.cfg" \
    --output "$decode_test_dir/tables.cfg" >/dev/null
encoded_report=$("$REPO_DIR/tools/decode-cfg.py" "$decode_test_dir/tables.cfg")
grep -Fq $'tables.cfg\t0\t881\t881' <<<"$encoded_report"
if "$REPO_DIR/tools/encode-cfg.py" \
    "$REPO_DIR/working/runtime/game-nocd/CFG/tables.cfg" \
    --output "$decode_test_dir/tables.cfg" </dev/null >/dev/null 2>&1; then
    printf 'Encoder unexpectedly replaced an existing output.\n' >&2
    exit 1
fi
"$REPO_DIR/tools/decode-cfg.py" \
    "$REPO_DIR/working/game-clean/CFG/Encrypted/creature.cfg" \
    --output-dir "$decode_test_dir" >/dev/null
printf '%s  %s\n' \
    e6cdb7fb28baaef775bdba3f619801819ffe39fa1d2af799ffc10d8df2d74b36 \
    "$decode_test_dir/creature.cfg" | sha256sum --check --status
if "$REPO_DIR/tools/decode-cfg.py" \
    "$REPO_DIR/working/game-clean/CFG/Encrypted/creature.cfg" \
    --output-dir "$decode_test_dir" </dev/null >/dev/null 2>&1; then
    printf 'Decoder unexpectedly replaced an existing output.\n' >&2
    exit 1
fi
"$REPO_DIR/tools/decode-cfg.py" \
    "$REPO_DIR/working/game-clean/CFG/Encrypted/creature.cfg" \
    --output-dir "$decode_test_dir" --force >/dev/null
"$REPO_DIR/tools/run-game.sh" check \
    --runner "$REPO_DIR/tests/fixtures/fake-game-runner.sh"
window_check=$("$REPO_DIR/tools/run-game.sh" check \
    --wine "$REPO_DIR/tests/fixtures/fake-wine.sh" --window)
grep -Fq '/desktop=MagicMayhem\,1280x960' <<<"$window_check"
scale_check=$(MNM_DISPLAY_SIZE=1512x982 "$REPO_DIR/tools/run-game.sh" check \
    --wine "$REPO_DIR/tests/fixtures/fake-wine.sh" --scale)
grep -Fq '/desktop=MagicMayhem\,1512x982' <<<"$scale_check"
MNM_FAKE_RUN_SECONDS=60 "$REPO_DIR/tools/run-game.sh" smoke \
    --seconds 1 --runner "$REPO_DIR/tests/fixtures/fake-game-runner.sh"
"$REPO_DIR/tools/original-manifest.sh"
"$REPO_DIR/tools/prepare-working.sh" stage-media

printf 'Tooling checks passed.\n'
