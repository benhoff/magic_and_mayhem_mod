# Live finite startup history

Native policy connects `SceneHistory` to the owned asynchronous World worker and
resumable GPU upload pipeline. Version2 delivers every first Quick Battle queue
in a bounded journal; the viewer retains its native canvas and complete GPU
lease while preparing the next frame. Source queue identity and native reset
metadata are explicit. Source gaps or omitted raster work refuse the session.

The first scope is1..16 contiguous queues on one fixed original canvas. Original
menus/input/drawing remain active. Original pre-canvas menu/loading pixels and
between-queue HUD writes are excluded from native inputs. The earlier offline
startup comparison establishes a fixture to test, not current live equivalence.
Native zero initialization is intentional policy. No unbounded history, loading
transitions, HUD/input ownership, total GPU/RSS bound, hard driver latency or
original raster bypass is claimed.

The public `./tools/run-native-world.py --startup-history [--verify]` now runs
the automatic bounded producer-history startup instead of this World-only
channel. It captures menu/loading/HUD producers and compares every completed
canvas through16World queues. See [public launcher](native-world-startup-launcher.md).
The World-only history implementation and historical evidence below remain
available through the dedicated capture/test tools. Default continuous launches
still use the interactive newest-frame mode.

Automated installed-game execution uses `tools/capture-scene-game.py
--world-live history-verify --live-frames 16` under Xvfb. Synthetic checks cover
queued ENDED draining, retained previous pixels, explicit canvas resets and
resize, malformed/gapped metadata, refusal, cancellation and PE32 publication.
Frame processing time measures packet admission through native completion;
independent GUI heartbeat and maximum poll duration are measured separately.
Pending work advances on a1ms Qt timer; idle producer polling uses16ms. The
initial installed attempt exposed cumulative16ms waits for every serial
preparation/upload phase and was deliberately interrupted through a working
channel failure. Its sources/results remain historical. Subsequent-frame times
start with catalogue/decoded residency warm; GPU entries may still miss or evict.
Observed processing times are not hard latency guarantees.

[Fresh installed verification](native-world-live-history-verified-20261008.json)
presents all16 first queues on the observed Quick Battle fixture, comparing
7,680,000 pixels with zero mismatches, drops, superseding or capability refusals.
Native processing was2831ms for the first frame and2931..3067ms subsequently.
Maximum GUI heartbeat gap was61ms and maximum native poll51ms under Xvfb/software
OpenGL. These are bounded observations, not real-time throughput or hard limits.

The [initial interrupted16ms experiment](native-world-live-history-polling-interrupted-20261008.json)
completed9 matching frames but accumulated about45seconds per subsequent frame;
adaptive pending polling removes the artificial per-phase wait. The experiments
use different startup requests, so this is not a controlled same-input speedup.

A [separate fast first-canvas attempt](native-world-live-history-first-canvas-refusal-20261008.json)
refused2 pixel differences before presenting any frame. [Independent original
replay from zero](native-world-live-history-zero-gap-20261008.json) matches that
native canvas exactly across480,000 pixels while differing from the actual
original output at those2 pixels. This is consistent with inherited startup or
other external producer content; the precise producer is not established here.
Zero initialization is not a general original-entry equivalence contract.
The successful16-prefix is a fixture result; closing startup/HUD producer gaps
remains necessary for broader takeover. Historical source hashes stay unchanged.


## Final source snapshots and pending workspace validation

The reproducible `tools/run-world-history-validation.py --mode live` freezes
sources before building and running normal/verified16-frame prefixes. `--mode
synthetic` freezes the69-test/12-PE32-mode corpus. Test include files and compressed
fixtures are retained. Original-manifest checks delegate to the real repository;
staging resolves source directory symlinks before copying. The failed symlink
staging attempt was detected by the source hash guard, and the working No-CD
executable/preferences were restored to their exact prior hashes; originals
were verified unchanged. Generated staged DLLs were retained in that failed
experiment's `source-restoration` directory.

Final [normal execution](native-world-live-history-snapshot-normal-20261008.json)
presents16 frames with zero native readbacks, comparisons, drops and superseding.
[Verified execution](native-world-live-history-snapshot-verified-20261008.json)
presents all16 and matches7,680,000 pixels exactly. [Synthetic execution](native-world-live-history-snapshot-synthetic-20261008.json)
passes69 tests and12 PE32 modes. Normal first/subsequent processing was3386ms and
3899..4185ms; max GUI gap59ms and poll53ms. Verified timings were3308ms and
3591..3822ms, max GUI gap76ms/poll68ms. Synthetic/build work ran concurrently;
these are observations rather than a controlled speed comparison.

[Snapshot provenance](native-world-live-history-source-snapshots-20261008.json)
records subsequent independent canvas-producer edits. Their protocol/hook and
canvas replay files changed during execution. Snapshot results stay historical;
the new behavior register retains partial implementation/no current integration
promotion until the required current workspace sources are rerun together.
No prior source fingerprints are refreshed. Native startup/HUD producer coverage,
all seeds/transitions and current whole-workspace validation remain pending.

Warm GPU upload churn is the next performance target: the final normal prefix
prepared22,678 uploads/transferred346,079,720 bytes, despite warmed decoded
resources. A bounded sprite atlas and palette reuse can remove repeated uploads
and per-miss handoffs; a real-time frame-rate claim needs new measurement.


The [focused current preparation regression](native-world-history-preparation-focused-20261008.json)
passes all4 resource/World preparation, GPU upload and Qt session tests using
those policies' complete reviewed dependency closure (including channel v2).
It supports the independent worker/upload policies without asserting validation
of the concurrently changing producer/replay implementation. Full snapshot69-test
results remain separate. Run `tools/test-world-preparation.py --focused` under
Xvfb to repeat this check.


## Completion accounting

The completion census records 1,438 source/build files with no missing, removed,
changed or drifting entries at review time. The full audit still reports eight
scenario errors in the independent canvas-producer workstream: four scenarios
lack evidence/tests and the corresponding execution level. The source-scoped
review excludes only its two new contracts and records exact pending receipts
for the history implementation and shared integration dependencies. This does
not constitute a repository-wide gate pass. Earlier startup status promotions
also require fresh execution after shared hook sources changed; their historical
evidence and source hashes remain intact. No new current integration promotion
is made for live history. Machine-readable completion audit and gate reports
remain under `working/tests/world-live-history-completion-*`.

## Manual launcher: queue-2 mismatch, 2026-10-08

`./tools/run-native-world.py --startup-history --verify` in experiment
`run-l5v_hzru` builds successfully and starts the existing World-only shadow
preview. Queue 1 matches; queue 2 is refused with eight differing pixels at
x425–427/y131–134. This is an output comparison failure, with zero capture
refusals and producer reason0, rather than an unsupported-kind refusal.

The [retained independent diagnostic](native-world-manual-history-gap-20261008.json)
extracts the first two immutable channel packets and runs the private unmodified
original World reference from zero, then its own first completion. Both complete
replay hashes equal the native report hashes. Queue 1 also equals live original;
queue 2 differs from live original at precisely the same eight pixels. Sources
and inputs remain stable; original manifests verify 2,927 files before/after.
The replay uses no live destination bytes as native inputs.

The captured requests and chosen initial-state replay reproduce the native
result. The two packets do not identify the missing original writer, its timing,
or any omitted original context; between-queue producer content and untraced
writes remain possibilities. No rendering arithmetic change is justified by
this diagnostic alone. The standalone producer-history/raster-prefix proof is a
separate path: this manual launcher still builds `mnm-world-live`, rather than
`mnm-canvas-producers-live --world-handoff`. Wiring the validated native producer
history into the public startup launcher remains an integration milestone.
Historical successful prefixes retain their hashes; this failure receives its
own diagnostic ID and does not claim fresh complete-prefix equivalence.
