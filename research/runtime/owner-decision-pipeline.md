# Captured ordinary original decision pipeline (NS88)

This experiment captures the pinned NoCD `415c40` ordinary single-source
count-mode decision pipeline before executing any native decision functions.
A separate native64 process and an ASan/UBSan/LSan process consume its recorded
stimuli and compare every captured slice. The original reference main only runs
original engine bodies; included native fixture functions are not invoked.

The composed chain is `415c40` -> `423590` -> `431be0` / `413330` -> `431ce0`
(regional search, 72 produced features and generated lifetime) -> `4766b0`
(authored nonuniform inference model) -> `4284e0` / `4261b0` -> `4289e0` ->
`428b40` / `432900` / `407c10`. Allocators, fresh evaluator clone, source queue
registration/maintenance, perception, snapshot swapping, training, accounted
credit and command delivery are explicit services inherited from the NS83 lab.
Actual command calls are captured as target/move/stop semantic records.

The scope is one ordinary enabled source, brain0 and two authored hostile memory
records in eight regions. There are 32 model/seed fixtures and 220 scheduler
slices per fixture. Original installed task dates determine the damage/expiry
clock jumps; replay reads those dates as stimuli, without recalculating them from
native task state. The original capture is not a running original game or a
capture of original perception production. Fixture generation remains authored,
source-bound test input, separate from recovered candidate generation.

Each slice records exact clock/root phase/pending execution, execution call count
and remaining body budget, selection reset/score, all 250 strategic RNG words
and cursors, task presence/serial, distinct result14/invalidation15 flags,
start/expiry/value/baseline/attempt/success, chosen target center/region/risk,
all 72 installed saved features and ordered commands. Installed saved features
cover the winning candidate; losing candidate intermediate feature buffers and
planning-budget endpoint values are not independently captured.

## Transcript and reproduction

`MNMDP001` is a bounded fixture-deck transcript, not a general game-save format.
All following values are unsigned little-endian 32-bit words; signed integers
and binary32 values retain their exact bits. The header declares 32 episodes and
220 frames, followed by each fixture index. Each frame holds ordinal, captured
tick, total count allocation, pause, stale-observation, region0 danger and
accounted-credit stimulus; then a length-prefixed state vector and command
vector. The state order is the one listed above. An active task adds its fields
and saved features. The maximum file size is 32 MiB. Exact fixture/ordinal/state
and command extents, input domains and end-of-file are checked. The runner also
requires refusal of bad magic, truncation, trailing data and a changed state.
Source fingerprints pin the fixture/model/world construction alongside the
native code. Files cannot be replayed against an arbitrary changed fixture deck
as if they were portable original observations.

Run under `tools/working-retention.py run --reserve-gib 0.5 --`:

```sh
python3 tools/draft-coverage-claims.py \
  --behavior AI.owner-decision-pipeline-trace --output NEW_CLAIMS.json
python3 tools/test-owner-decision-pipeline.py \
  --claims NEW_CLAIMS.json --output NEW_RUN_DIRECTORY
```

The dedicated runner preserves existing comparison callers. It checks the whole
original hash before mapping or redirecting its private image, records expected
endpoint bytes/target hashes, protects text RX, and verifies all private text
unchanged after execution. It brackets original consumption with manifest
verification, pins compiler dependencies, rejects mixed-source builds and retains
capture/transcript hash, exact binaries, outputs and refusal diagnostics.

## Remaining boundaries

This closes a bounded original-first candidate-to-task comparison milestone;
it does not close Original AI composition. Multi-source decision topology with
real candidate production, wizard scoring/survival, other task families,
second-brain work and full World admission/timing/difficulty/perception scheduling
remain pending. The existing native skirmish task-body replay is a different
milestone. No gameplay balance or live replacement changes are made.

Execution results are registered only after prospective source/scope declaration
and a fresh successful run. Earlier failed development runs remain retained.

## Accepted execution

[NS88 original capture and replay](owner-decision-pipeline-original-ns88-20261018.json)
passes 32 fixtures, 7,040 slices and 2,446,668 exact checks in original32,
native64 and ASan/UBSan/LSan processes, with identical digests. It records 192
installations, 6,073 execution calls, 1,750 pending slices, 101 completion-state
observations, 5,497 targets, 480 moves, 96 stops, 32 expired-task skips and 192
disabled calls. Counts are slice observations, not unique completion events.
All four malformed/changed-transcript controls refuse. The 9,955,640-byte
original transcript, binaries, compiler outputs and refusal diagnostics are
pinned under `working/tests/owner-decision-pipeline-ns88-20261018/.retain`.
All declared/compiler inputs, original private text and immutable original
artifacts remained unchanged. The development failure was a witness counter
observing expiry after retirement; no engine behavior change was required.
