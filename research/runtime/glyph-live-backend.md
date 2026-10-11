# Opt-in live glyph backend

The fixed No-CD glyph entry at `0x581ec0` is installed through existing producer
tag10. Other drawing and higher text layout remain original. Admission and
recovered effects are separately registered as `NR.glyph-live-admission` and
`RS.glyph-live-route`; bounded live validation is recorded below.

Each call copies its complete indexed frame, current RGB565 canvas prefix,
coverage table and FXSAVE image into private storage. VirtualQuery walks every
region; writable destinations reject read-only, guard and no-access pages.
The kernel validates every row, including hidden rows, and its supported masked
x87 domain. Source bytes and current addresses/clip/format/table/background are
rechecked before commit. Addresses are discovered afresh per call. There is one
observed engine thread and a busy guard; this does not synchronize unrelated
writers. Calls outside admission continue through the original trampoline.

Shadow mode compares the original completed canvas, coverage, volatile/callee
registers, rewritten arguments, defined flags and selected x87/XMM/MXCSR state.
Its return wrapper preserves the argument area while checking and restores the
original caller with RET16. Takeover commits the private trial and bypasses only
the glyph body. LastError, incoming ABI and original fallback are preserved.
FP instruction/data pointer history and empty FP registers remain excluded.

Both modes retain eight distinct complete inputs for independent unchanged
original replay and expose versioned counters. A 65,536-call admission budget
then forwards original calls. Queue-capture completion closes admission;
ordinary sessions flush counters periodically and at DLL detach. This is a
bounded correctness experiment with per-call full-canvas copies, not a measured
performance improvement or full rendering replacement.

Enable explicitly for the normal or live-menu Qt launch:

```sh
MNM_GLYPH_BACKEND=takeover ./tools/run-qt-shell.sh --live-menus --native-commands
```

Use `shadow` for original-active comparison. Launch game, then choose a game in
the native menu. Each launch creates a fresh `glyph-backend` evidence directory
beside its staged installation. Omit the environment variable for the default
original glyph path. Conflicting producer/World/startup/word experiments refuse
activation. Source executable hashes and entry/helper signatures are checked.
Public interactive gameplay equivalence and performance remain pending.

Reproduce bounded validation with prospective claims:

```sh
python3 tools/test-glyph-live.py --claims working/tests/glyph-live-claims.json
python3 tools/test-glyph-backend.py --producer-entry \
  --claims working/tests/glyph-live-claims.json \
  --corpus working/tests/glyph-backend-state/run-3qrghf1m/cases.bin \
  --index working/tests/glyph-backend-state/run-3qrghf1m/cases.json
```

The scripts verify immutable original manifests before/after, pin sources before
execution, retain command logs and immutable outcome reports, and compare
captured primitive inputs independently of original completed output.

## Capture wire and observation boundary

`glyph-stats.bin` is little endian `MNMGLST1`, version1, length144:36 DWORDs.
Words4..23 encode mode, installed, seen, admitted, handled, shadow compared,
forwarded, errors, retained samples, call budget, busy/other-thread refusals,
memory/format refusals, alias refusals, backend refusals, recheck refusals,
allocation refusals, budget/closed refusals and three reserved fields. Words24..35
count admitted controls in PC24/53/64 order, each with RCnearest/down/up/zero.
Concurrent/nested forwards have their own counter and are not included in `seen`.
Ordinary sessions flush every64 seen calls, so counters are periodic snapshots.
The bounded scene experiment flushes when queue capture ends.

Each `glyph-N.bin` starts with `MNMGLIV1`, version1 and a256-byte header.
Words4..18 encode mode, frame size, clip-right width, clip-bottom height, stride,
canvas byte extent, original frame identity, original entry ESP, four clip bounds,
cold flag, sentinel and seen-call sequence. Words20..33 and34..47 contain entry
and returned PUSHAD/flags/caller/four-argument frames respectively. Shadow's
returned ESP/caller are normalized to the entry-frame representation; volatile
and preserved registers, flags and rewritten arguments are actual observations.
Takeover records the frame committed to the production RET16 entry.

Payload order: complete frame bytes; entire pre-call and completed canvas prefix;
256-byte incoming and completed coverage tables;512-byte incoming and completed
FXSAVE images. Canvas extent is `2*stride*clip_bottom`; rows below the clip bottom
remain outside this primitive's contract. Payloads include canvas row padding.
Frame addresses are identities for replay, never pointers in the replay process.
The decoder rejects inconsistent lengths and excessive dimensions.

Retention reserves six black-tint, one nonzero-tint and one cropped/skipped
geometry slot. A fixture need not fill every category: report actual retained
inputs and observed controls rather than inferring live branch coverage from the
synthetic corpus. Independent replay seeds only pre-call input/background; the
completed original/native output never feeds the trial.

## Recorded validation — 2026-10-10

The [final source-bound live report](glyph-live-comparison-20261010.json) records
four World returns after automatic Quick Battle startup in each mode:1,817
original-active shadow completions and1,817 native body-bypassing completions,
with39 original forwards and zero adapter errors per mode. Six distinct complete
inputs in each mode match independent unchanged-original and native replay,
1,684,800 RGB565 WORDs per cohort. Retained inputs are cold, black-tint interior
glyphs under PC53/round-nearest and an empty x87 stack. Nonzero tints, crop/skip
branches and other controls remain isolated evidence, not additional live claims.

The [fresh shared-entry comparison](glyph-live-entry-comparison-20261010.json)
passes28,142 cases/22,656,678 canvas WORDs through both isolated and production
entry:27,966 native handles,176 actual original calls including safe guard
forwards,24 atomic ASan/UBSan refusals, and a freestanding PE32 combined object.
Each process compares stack addresses and ADD ESP flags against its own original;
raw process-specific identities are not compared across runs.

Production VirtualQuery tests pass for read/write committed ranges, a read-only
second region, guarded and no-access second regions, zero/wrapping extents and
alias/adjacent range predicates. Both public launch paths stage the same pinned
glyph DLL and authorize fresh outputs; four invalid authorization requests each
refuse. These staging checks do not assert a full interactive Qt game session.

[Experiment history](glyph-live-experiment-history-20261010.json) retains failed
harness attempts and successful intermediate source versions without refreshing
old hashes. The first shadow capture passed before the wrapper mistook the
printed report path for another directory; the first shared-entry aggregate
check failed because raw stack identities were compared across processes. Final
runs correct those harness checks and rerun the changed admission sources.

Default activation, longer sessions/font resets, public interactive gameplay,
other drawing families, general device recovery/concurrency and a measured
performance improvement remain open. This adapter copies/rechecks full canvas
prefixes per glyph; optimize that only with new independent execution evidence.

Quota exhaustion, nested/concurrent calls, allocation or recheck failures and
long-running font/DC/device transitions are implemented refusal boundaries but
were not induced by the final live fixture. A shadow mismatch ends the diagnostic
process with exit96; it cannot establish equivalence by continuing silently.
