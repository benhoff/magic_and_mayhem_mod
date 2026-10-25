# Storage upload causal investigation — 2026-10-25

The fresh software-presentation experiment identifies the wrong upload source
window after the first 800×600 → 640×480 transition. The original source texture
and nominal default framebuffer agree throughout. At the failing presentation,
XCB submits row 0 of a still-832×640 SHM image; those exact bytes explain all
301,992 wrong physical pixels. Rows 120–599 of that same sampled image match the
correct 640×480 default framebuffer, with zero mismatches. The next upload uses
a fresh 640×512 image and agrees without correction.

This confirms the public software-upload boundary in this observed run. It does
not identify a private Wine/Mesa allocator function or prove a general rule for
every resize, driver, writer or format. No native rendering arithmetic defect is
demonstrated, and no native renderer code or web UI code is changed by this batch.

## Evidence and independent controls

All original executions use immutable NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The manifest verifies all 2,927 immutable inputs before and after each capture
and finite analysis. Staged original PE section bytes are compared separately.
The diagnostic is a separate explicitly selected observer dependency; existing
EGL observation code and its opt-in correction remain unchanged.

| Acquisition | Closed presentation interval | Result |
| --- | ---: | --- |
| Unchanged observer, no SHM sampler | 232 frames | Source/default agree throughout; physical mismatch only at shrink ordinal 172 |
| Separate SHM sampler, correction disabled | 238 frames | Source/default agree throughout; wrong output at shrink ordinal 166 is exactly explained by the submitted source window |
| Separate SHM sampler, correction explicitly enabled | 244 frames | Source/default/physical agree throughout; one witnessed crop, six independently bound complete SHM sample points |

These are independent runs, with run-local ordinals. The fresh 238-frame run is
not the historical 238-frame fixture. Results are immutable SQLite reports:
[plain](storage-upload-cause-results-20261025/plain-finite-03.json),
[uncorrected](storage-upload-cause-results-20261025/baseline-finite-03.json), and
[corrected](storage-upload-cause-results-20261025/corrected-finite-03.json).
The correction changes the submitted source window using the independently
observed logical height delta; it does not rewrite the retained reference.

The [owned controls](storage-upload-cause-results-20261025/owned-08.json)
pass eight decoder/causality guards, exact checked/unchecked forwarding controls
(including cookies and incoming/returned errno), six SHM pattern cases and seven
EGL resize cases. The uncorrected owned stale-image control has 307,196 wrong
pixels; fresh and corrected controls match. Unknown-segment samples refuse
without changing forwarded results. A missing process terminal marker remains
a refusal for complete sample-cohort acceptance.

The finite diagnostic verifies every one of the historical fixture's 238
physical planes against the original acquisition hash, as well as its catalog,
retention report and original report identity. Reference packaging is unchanged.
The historical exact SHM source was never captured. Its sparse ordinal-170
frame alone cannot uniquely identify a 120-row origin: that precise attribution
is confirmed for the new rich-content run, and remains an inference for the old
frame. The historical 126-GPU + 126-physical-pixel discrepancy stays retained.

## Failed session acceptance stays failed

All three original captures complete the storage/rebuild stimulus and close
their independently fenced EGL physical intervals, but their separate producer
source sessions fail `Session source interval did not close quiescently`.
The collector consequently terminates its owned process. No complete source
session, normal Quit, source-only native replay, complete process sample cohort,
original comparison milestone or live replacement is accepted from these runs.
The pending source frame and shutdown cause remain open.

Finite analysis admits only individually complete metadata/pixel pairs, each
bound to one exact enclosing original XCB upload by PID, thread, geometry,
arguments, cookie and monotonic interval. It requires the entire closed
presentation cohort, every shrink point and an explanation for every physical
discrepancy. It never manufactures a missing terminal marker. All broader
acceptance flags remain false in the successful diagnostic reports.

Earlier failures remain retained: the initial private fixture started at
640×480 rather than the storage controller's required 800×600; an early Xvfb
wrapper failed; and a later diagnostic parent hit a relative-report-path error
after the child had already recorded its source-closure failure. Initial native
replay attempts separately retain RAM exhaustion, Xvfb/CTest setup failure and
a corrected-profile acquisition blocked by the concurrent SQLite archive writer;
that last attempt produced no corrected-profile comparison. The latter
path handling is corrected in source. None of those failures is a renderer pass.
An explicit host-network namespace attempt is also retained as a refused
admission control. The accepted controls and captures enter both private mount
and network namespaces and use a private RAM-backed temporary directory.

## Integration and remaining scope

The integration base is `d6618d09b01d803e409ae82a882db7439f0c9925`;
the reviewed assurance parent is
`05213222317d102fbe64b22a96fd206f272462d1`. Combined bookkeeping preserves both
sets of behavior IDs, receipts and independent status dimensions. The live owner
runner keeps the newer functional lifecycle options and the assurance parent's
explicit `--no-native-music` option. Historical source fingerprints are retained;
an inherited successful execution is not automatically a current combined-source
proof. The merged runner passes the twelve native-only contract guards and CLI
admission for both functional lifecycle and music options; this is not a new
combined-source live journey.

The [fresh software replay](storage-upload-cause-results-20261025/native-software-04.json)
passes all 323 Quick Battle and 222 explicitly corrected-storage frames. The
unchanged 238-frame legacy reference still differs at ordinal 170 by 126 GPU
and 126 physical pixels. The [fresh NVIDIA replay](storage-upload-cause-results-20261025/native-hardware-03.json)
independently reproduces all three results. Three scene/session CTests pass in
each accepted acquisition. The [current correct-output assessment](storage-upload-cause-results-20261025/correct-output-acceptance.json)
passes the existing user-selected policy, with the legacy diagnostic retained
and `whole_renderer_accepted` false. This supplies current selected native replay
evidence while retaining the failed legacy profile.

The original ask was confidence in the port: this work removes one uncertainty
about whether the storage discrepancy came from native arithmetic or reference
packaging. It supplies a measured original-presentation cause in a fresh bounded
run. Complete route/writer coverage, source-session shutdown, physical loss and
restore, unmodified-driver parity, whole-game replacement and performance remain
independent outstanding questions.

## Reproduction and retention

Use an owned checkout, locally available verified SQLite payloads, a private
mount/network namespace, an idle exclusively locked prefix and an 800×600 staged
NoCD installation. Generate prospective declarations with
`tools/draft-coverage-claims.py` and the source closure used by the producers.
Run `tools/test-storage-upload-causality.py` for owned controls, then
`tools/capture-storage-upload-causality.py` with distinct `plain`, `baseline` and
`corrected` outputs. Full collector refusals are retained. Use
`tools/analyze-storage-upload-causality.py` with the finite-presentation scenario
to diagnose a separately closed presentation interval without granting complete
session acceptance. See the [wire format](../formats/egl-shm-upload-source.md)
for sampling bounds, strict admission and perturbation limits.

Original captures and initial builds use task-owned RAM storage. Repeated native
replay output filled that temporary filesystem; the failed attempts remain
retained. Execution outputs and builds were copied to task-owned secondary-disk
storage, every file was verified, and private bind mounts preserve their paths.
The original RAM payloads were retained during this move. The cohort producer
now accepts an explicit temporary directory under `working/`, and normalizes
relative report paths. No RAM limit increase was made. Durable reports, exact
source deltas, raw evidence and verified SQLite backups are kept separately on
disk. The [original raw catalog](storage-upload-cause-results-20261025/original-raw-catalog.json)
verifies all 25,706 archive members (9,453,922,837 raw bytes) after reading the
[archive](storage-upload-cause-results-20261025/original-raw.tar.gz) back through
SQLite. It includes failures, actual sampled SHM bytes, physical/source/default
planes, original observation records, saves, declarations and executed observer
libraries. Unchanged staged input files are identified separately rather than
stored as another full installation snapshot. This batch uses the normal 90 GiB reserve; the earlier
88 GiB exception is not extended. Original inputs, existing fixtures and
historical evidence are preserved.

The separately retained [native raw catalog](storage-upload-cause-results-20261025/native-raw-catalog.json)
verifies every member of the [native archive](storage-upload-cause-results-20261025/native-raw.tar.gz),
including failed acquisitions, current replay inputs/outputs, hardware admission
logs and prospective policy declarations. Retention does not grant acceptance
to failed profiles. The full local evidence database is
`/home/hoff/Videos/mnm-rendering-storage-cause-20261025/coverage/evidence-pending.sqlite3`;
set `MM_EVIDENCE_DB` to that path when resolving its large raw archives from
another checkout. Its verified backup catalog is retained beside the task's
SQLite backups. No evidence is published by local completion.

The completion helper's working-tree check omits pending `MERGE_HEAD` and
therefore refuses three assurance-parent milestones as new promotions. The
[refusal](storage-upload-cause-results-20261025/completion-helper-refusal.json)
is retained. Completion uses the documented manual **staged** draft workflow,
imports its exact pending receipt into SQLite, retains verified local backups
and commits with the normal staged pre-commit gate. Only actual merge parents
provide milestone inheritance; no enforcement source, status, historical hash
or validation claim is changed to clear this tooling limitation.
