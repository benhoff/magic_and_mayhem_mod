# Storage source-session closure investigation — 2026-10-25

This batch investigates the independent producer-session closure refusal in the
three prior original storage acquisitions. It does not alter the web UI or grant
whole-renderer acceptance. The existing recorder requires no pending original
producer frames, no open World, no failures, a bounded queue count and a flushed
journal. The collector separately corroborates the actual original MainQuit
callback and process-handle exit.

An opt-in bounded snapshot records observer state after a requested close times
out. It preserves every pending frame and does not create a completion marker.
Thread IDs, stack/object pointers and callers describe one process only. They
supply no cross-launch identity, alias ownership or renderer equivalence proof.
The retained earlier streams establish an admission failure, rather than an
unclassified shutdown timeout. The [read-only raw analysis](storage-session-closure-results-20261025/old-source-diagnosis.json)
finds failure18/tag52 at the original colour-rectangle hook `0x00547d80`:
375 failures in baseline03 and251 each in corrected04/plain02. Their final
record tails are51/87/51 bytes respectively. The strict failure-free closure
gate correctly refused them. Code18 has several guards, including binding,
readability/format and unsupported shape inputs; the original failed subcondition
was not captured. No pending-frame deadlock or repair of those shape branches is
claimed. Historical reports, hashes and negative results remain unchanged.

The fresh [baseline source session](storage-session-closure-results-20261025/closure-07-original.json)
passes actual Save/Load, both display rebuilds, recreated MainLoad and return Main,
with36030 producer records,16 balanced World queues, a verified independent source
marker and the observed original MainQuit callback/process-handle exit0.
[Its native comparison](storage-session-closure-results-20261025/closure-07.json)
matches all218 original GL-source and nominal-destination frames. Physical output
still differs at first-shrink ordinal158 by301992 pixels. This is a successful
negative control, not accepted physical parity.

The separate [explicit corrected-upload session](storage-session-closure-results-20261025/corrected-08-original.json)
closes34376 producer records and15 balanced World queues with the same independent
Quit checks. [All205 native/source/default/physical presentations match](storage-session-closure-results-20261025/corrected-08.json),
with zero mismatching pixels. This is the existing explicit stale-upload crop
policy, not an unmodified-original reference. Original oracle pixels remain
external comparisons, never native replay inputs. Both runs use software OpenGL;
neither grants whole-renderer, hardware-scanout, all-writer or replacement credit.
Physical RGB comparisons retain channel tolerance1 and exclude alpha.

## Storage cadence and fixture changes

Attempt06 already closed its source stream and exited normally, but its overall
collector [refused the global sequence check](storage-session-closure-results-20261025/diagnostic-06-original.json).
The raw cadence trace contains accepted sequence21 followed later by1, immediately
after a complete pacer entry/return both observing sequence0. Storage reporting
now records these as separate observed epochs in `engine_session_observation`.
It requires that exact adjacent zero-pacer/sequence1 pattern, preserves counter,
nesting, thread, QPC and queue checks, and never joins continuity across epochs.
It does not infer Load causality. Continuous/source-binding consumers still use
the strict `engine_update_observation` analyzer and refuse duplicate/reversed
sequences. Both fresh runs observe two epochs.

The [controls](storage-session-closure-results-20261025/controls-03.json) pass the
six C refusal boundaries plus20 Python tests, including unchanged default
sequence refusals, new epoch refusals, malformed snapshots and legacy marker
accounting. An empty snapshot never grants source admission.

The isolated fixture initially encountered three optional Mono installer timeouts,
one missing launcher manifest and one original SOUND ERROR startup dialog.
All failed attempts are retained. Wineboot alone now disables optional Mono/Gecko
installers; the game environment is unchanged by that bootstrap policy. Acquisition
uses the existing private PipeWire/Pulse null sink, with no desktop routing or
hardware monitors. Original startup and actual original shutdown are both checked.
Loopback and normal-user identity setup changes alone did not solve the installer.

Input admission caught a previously modified working executable,11 encrypted
configurations and an adaptive AI Brain. Only the private fixture was repaired.
[Full admission before07](storage-session-closure-results-20261025/input-admission-07.json)
checks all4834 installation-manifest entries; only authored preferences differ.
The earlier Brain input and all failed source variants remain retained. Generated
save bytes and all original callback/input checks remain separate from rendering
equivalence. Every original manifest bookend verifies2927 files. No gameplay rules
or web UI were changed.

## Refusal snapshot and retention

The opt-in `MNM_CANVAS_SESSION_CLOSE_DIAGNOSTICS=1` recorder reads one explicit
`canvas-session-0000.probe` request. `MNMPSC01` is a64-byte header followed by at
most128 rows of32 bytes: slot, hook tag, local thread, primitive flag, caller RVA,
stack pointer, retained operation kind and object pointer. The header retains
sequence/queue/byte counts, failures, World-open and journal failure state. The
strict decoder rejects malformed extents/identities. The snapshot does not clear
frames, alter original functions, write the admission marker or prove stable
pointer ownership. Fresh successful runs did not require a refusal snapshot;
actual refusal-state preservation is covered by the C controls.

The [exact preservation plan](storage-session-closure-results-20261025/raw-plan-01.json)
and [verified SQLite archive catalog](storage-session-closure-results-20261025/raw-catalog-01.json)
retain all unique raw attempts, native outputs, source variants, private prefix,
input configuration and saves. Repeated assets reconstruct from one retained
private installation:25729 retained members and43925 duplicate-asset mappings.
The complete archive round trip and a retained verified SQLite backup passed
before the two idle RAM namespace holders were stopped. Temporary RAM builds/caches and the bounded archive-import
file are execution outputs, not additional validation. The normal90GiB guard
applies; no earlier88GiB exception was reused.

These results are bound to the owned `storage-session-closure-20261025` worktree,
based on568c7304 plus preserved unique source variants. Concurrent main-branch
integration is separate and requires its own exact review/freshness accounting.
