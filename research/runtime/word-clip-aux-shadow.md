# Clipped main-word drawing with opaque auxiliary planes

The previous clipping core rejected every nonzero auxiliary offset. The selected
word backends draw the main plane; auxiliary payload interpretation belongs to
original caller passes. The expanded core accepts bounded auxiliary offsets,
uses the earliest as the colour-stream limit, and never reads or changes payloads.
The recovered workspace model uses an independent corresponding extent check.
Native drawing still writes only to a guarded shadow copy; no clipped bypass.

Positive synthetic cases now exercise no plane, first-only, second-only, both
ordered and both reversed offsets, with padding and opaque payload sentinels.
The original comparison checks complete source bytes, pixels, all workspace
words and selected caller state through the actual route/entry. Atomic native
refusals cover auxiliary offsets before tables, inside colour data, out of the
frame and late/hidden invalid rows. These refusals are native policy rather than
unsafe original malformed execution.

Live capture retains distinct auxiliary-bearing inputs: two clipped and six interior instead of filling
all slots with repeated small menu banners. A diagnostic FNV32 frame fingerprint
plus backend suppresses repeats; SHA-256 replay independently checks actual input
diversity. Hash collisions conservatively omit samples. Eight records, exactly two clipped, and at
least four different frame byte sequences are required, plus observed auxiliary
and clipped comparisons. Backend and refusal counts are retained separately in
[admission diagnostics](../formats/word-admission-stats-v1.md).

Use prospective claims for `RS.word-clip-aux-shadow` and
`NR.word-clipping-plane-bounds` before invoking `tools/test-word-clip-shadow.py`.
Live execution uses `LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a` with the existing
`tools/capture-scene-game.py --word-sprites clip-shadow --samples 4` pipeline;
`tools/replay-word-clip-shadow.py` independently executes both original and native
bodies for the retained samples. Original manifests are verified before/after.

Payload effects, indexed input, source-row padding/reordering, complete floating
exceptions/history, real nested reentry, physical Windows and sustained sessions
remain separate. Backend bounds describe the current drawing target; they do
not identify its semantic caller. World startup observations establish continued
operation separately. No scalar live coverage is claimed unless its
counter is nonzero. Old results retain their hashes and original scopes.

## Recorded result — 2026-10-10

[Final isolated execution](word-clip-aux-isolated-20261010.json) passes all
22,048 original/model/actual-route cases: 22,000 positive shadows (17,600 with
auxiliary metadata) and 48 empty forwards. All 110,240 forced refusals pass
caller-state, original-once, LastError and exact admission-reason checks.
ASan/UBSan verifies 1,440 manual placements, 79 atomic refusals and three empty
no-ops; the no-CRT PE32 DLL build passes. The compact record retains the full
per-run report hash, canonical case-index hash, command/provenance and outcomes.

[Live capture plus independent replay](word-clip-aux-live-20261010.json) passes
13,312 native/original full-canvas/workspace comparisons through four World
startup observations, including 705 clipped and 12,607 interior requests. All
observed requests carry auxiliary metadata and use forward backend `0x596cb8`.
There are zero fallbacks, mismatches, capture errors, stopped state or body
bypasses. Eight distinct encoded frames, from 1×18 to 49×50, include two clipped
and six interior samples; independent original/native replay matches every
workspace word and 1,420,800 complete canvas WORDs. A runtime input hash is not a
universal asset inventory or semantic caller classification. Scalar live
coverage remains absent; indexed and auxiliary-effect drawing remain original.

The initial [dimensional sampler failure](word-clip-aux-dimensional-sampler-failure-20261010.json)
and [distinct-clipped-only failure](word-clip-aux-distinct-sampler-failure-20261010.json)
remain preserved. The first wrongly required 800×600 targets; actual clip bounds
vary. The second correctly found only two distinct clipped startup inputs rather
than eight. The final mixed cohort and source/scenario contracts were declared
before rerunning both experiments. Intermediate passing synthetic reports remain
historical with their exact hashes under `working/`; they do not support the final
scope. Older CPU/model/shadow evidence retains its hashes after these shared
source changes and is not promoted by this result.
