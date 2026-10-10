# Opt-in direct-word clipping shadow

`MNM_WORD_SPRITES=clip-shadow` and scene capture's `--word-sprites clip-shadow`
select a new shadow-only route for the two pinned No-CD backend entries. Existing
shadow/takeover selections remain distinct. This mode does not bypass drawing.

For an admitted positive direct-word frame, the hook snapshots the full borrowed
canvas and workspace, preflights the CPU core and recovered state model, draws
into an alignment-preserving guarded copy, then executes original drawing once.
It compares every canvas byte, stride padding and all 16 workspace words; the
engine retains original pixels and original workspace. Only original-equivalent
argument-slot mutation/zero return are supplied by the entry wrapper. Mismatch
or capture failure stops further comparisons and later calls forward. Empty,
unsupported, unreadable/unwritable, allocation-failed or stopped requests forward
without CPU writes to the engine. No clipped takeover selection is added.

The no-CRT PE32 build includes the recovered workspace model. Its previous libc
copy dependency is replaced by explicit word/member copies, with identical scoped
semantics. A fresh original comparison is required for the changed model; old
reports keep their hashes and do not assert current-code/live equivalence.

Synthetic validation links the actual production route C and entry assembly with
host Win32 API substitutes, and unchanged original code. Original entry wrappers
count actual body executions. Four masked floating/flag seeds, caller registers,
arguments, stack/guards, selected floating state, LastError, complete canvas and
source bytes are checked. Five forced original-safe refusals per valid input
exercise allocation failure, unreadable frame, unwritable canvas, already-busy
and stopped states. This substitutes OS permissions and file writes; it is not a
real nested-reentry, installation, filesystem or Win32 exception stress test.

Bounded live capture checks real Wine installation/admission, separates clipped
from interior comparisons, retains eight clipped before/after samples and records
zero body bypasses. Independent i386 replay checks those samples against both
unchanged original drawing and native CPU/state composition; native drawing never
reads the expected after pixels. Full-session, shutdown, physical Windows,
unmasked/full-stack floating exceptions, other backends and installed-asset
generality remain open.

Reproduce with prospective declarations for `RS.word-clip-shadow` and the
appropriate isolated/live scenario. Use a fresh output filename:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.word-clip-shadow --scenario word-clip-shadow-isolated-20261010 --output working/tests/clip-shadow-claims-new.json
python3 tools/test-word-clip-shadow.py --claims working/tests/clip-shadow-claims-new.json
python3 tools/draft-coverage-claims.py --behavior RS.word-clip-shadow --scenario word-clip-shadow-world-entry-20261010 --output working/tests/clip-shadow-live-claims-new.json
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a python3 tools/capture-scene-game.py --word-sprites clip-shadow --samples 4 --claims working/tests/clip-shadow-live-claims-new.json
python3 tools/replay-word-clip-shadow.py --directory working/experiments/scene-observer/run-NEW/word-sprites --claims working/tests/clip-shadow-live-claims-new.json
```

Each runner preserves manifests, immutable result/source hashes and per-command
logs. Replay preserves scalar destination alignment and normalizes only workspace
words 7/10, overwritten frame identities for these positive samples; untouched
words retain captured values. Pointer identities are never assumed stable across
launches. The [diagnostic format](../formats/word-clip-shadow-v1.md) describes the
new mode and separate versioned clipping counters.

## Recorded result — 2026-10-10

The [final isolated run](word-clip-shadow-isolated-20261010.json) passes all
4,448 cases: 4,400 positive shadow comparisons and 48 empty forwards, with
original-once and LastError checks passing for every case. All 22,240 forced
refusal checks pass. The sanitized CPU boundary suite and freestanding PE32
DLL build pass. The Linux host shim requires `-mstackrealign` because the
Windows entry has four-byte cdecl alignment; this is a host build correction.

The [live result](word-clip-shadow-live-20261010.json) combines immutable
[capture](word-clip-shadow-capture-20261010.json) and
[independent replay](word-clip-shadow-replay-20261010.json) records. Software
Wine rendering, a repeated original map-dialog round trip and four World
observations complete. Eight clipped menu/banner requests compare successfully:
one 208×111 frame, forward backend `0x596cb8`, 800×300 canvas, clip-left 588.
Replay matches all 16 workspace words and 1,920,000 complete canvas WORDs for
both unchanged original execution and native CPU/state composition. Mismatches,
capture errors, stopped state and original body bypasses are zero.

The final counter snapshot has 21,607 forwarded calls. Refusal reasons are not
classified by this format; auxiliary/indexed frames are excluded by admission,
but this run does not establish the cause of every refusal. There are no observed
live scalar comparisons or admitted World sprites. These remain specific gaps,
alongside broader installed assets, full floating/Win32 behavior and sustained
sessions. The next replacement milestone is a separately guarded clipped
takeover; this increment supplies opt-in shadow evidence only.

[Experiment history](word-clip-shadow-experiment-history-20261010.json)
preserves the initial host alignment crash, its passing intermediate rerun,
the pre-comparison GL startup timeout, and a seven-sample capture rejected by
the declared eight-sample requirement. The repeated map transition was declared
before the final reruns. Supporting legacy regression passes 256 in-bounds
comparisons, 647 refusals and host CTest; its unbound result does not refresh
historical live claims. Shared model/launcher/capture changes leave older
evidence fingerprints stale, with their recorded statuses and hashes preserved.
