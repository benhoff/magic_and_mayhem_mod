# CPU copy clipping and history continuity

The windowed Quick Battle samples previously contained successful original calls
that the bridge refused before forwarding: attached-clipper CPU copies, rectangles
outside destination bounds, and a clipped fill. The retained quick04 failure then
invalidated a destination with GPU history after a refused clipped CPU copy and a
failed post-call checkpoint. This establishes the failing path, not universal
attribution for every historical GAP.

The native facade now exposes a read-only CPU rectangle plan while provider and
copy admission remain held. Preparation snapshots the same ordered clip pieces
used by its CPU transform; it does not mark GPU or CPU history current. Before a
clipped CPU transform preserves destination pixels, the bridge explicitly materializes a
GPU destination outside the tracker, then captures the operation generations.
The bridge freezes owned source/destination inputs and emits every admitted COPY
piece, or argument-derived fill UPDATE. Empty pieces and partial source-error
prefixes retain their separate results. No CHECK output becomes drawing input.

Current supplied controls in `cpu-copy-results-20261026/cpu-controls04.json`
compare15 cases against actual facade pixels and replay through OpenGL: single,
multiple and empty clips; negative/integer-extreme rectangles; first/late source
failures;256 pieces; four clipped fills; and exact-key clipping. All pass with
zero remaining graphics resources. The23 existing GPU cases and legacy refusal/
restore controls pass in `gpu-controls02.json`, including frozen self/alias/keyed
sources. The current shell/input controls pass in `input-controls03.json`.
These are native-policy controls, not original-driver comparisons.

The first control prefix failed to load kernel32; the second attempted prefix
was automatically returned to the environment pool by its short-lived setup
process and a fresh-prefix retry followed. Those setup failures are retained.
The first shell control's source-stability check failed after a diagnostic counter
and test assertion were corrected during its build; it is not current-code proof.
The correction uses the existing explicit-reader counter category, not an unknown
fifth category. Current controls are declared separately and hashes are preserved.

Live evidence follows below; natural GPU self/keyed reachability remains open.

The first new live run (`quick01.json`) completes normal Quit, END and zero owner
retirement with1274 prepared calls and no GAP. It also exposes a regression:
unconditional destination materialization eliminates every stale GPU source
before presentation. Its zero-GPU sample cannot close whole-output or the GPU
caller objective. The correction restricts preparation-time materialization to
clipped CPU destinations, matching the facade's own destination-read boundary.
Unclipped partial COPY preserves GPU authority and does not require a CPU return.
This failed acquisition and its exact source preimages remain retained.

The corrected current live acquisition (`quick02.json`) completes two World visits,
Results retry/return and original Main Quit with exit0, no fallback/recovery, END,
and all graphics owner counts zero. Its closed census has1370 original application
calls, zero unprepared/failed calls,124 prepared clipped CPU copies and one clipped
fill. All1345 emitted COPY commands have independently checked complete native
destinations;686 CPU-keyed calls and141 GPU-source calls bind their outputs.88 GPU
calls from0058c03d geometrically reduce their emitted rectangle under ordinary
Win32 window movement. No GAP or blit-commit refusal appears in the retained log.

The strict output validator accepts complete contiguous clipped CPU intervals and
still rejects missing, duplicate, wrong-resource/key/translation/extent pieces.
Version2 offsets remain unchanged.47 Python refusal/geometry/routing controls pass.
This closes the observed CPU preparation gap and bounded all-COPY proof. It does
not prove every intermittent history failure eliminated. Fill UPDATE output and
all-session pixels retain their separate boundaries.

Natural self-copy and keyed GPU-source rows remain zero in this bounded sample.
Their23 supplied GPU controls pass; CPU-keyed calls do not count as GPU-keyed
reachability. Clipper reduction within destination bounds, other contexts, arbitrary
readers and original-driver equivalence remain outstanding. No comparison,
integration or replacement milestone is promoted by this change.

The disposable own-batch launch cache contained4848files matching the retained
input template. Before retirement its complete cold archive was hash-verified,
actually restored and checked against every file, with matching local/cold SQLite
plan receipts (`working/coverage/runtime-cache-retention.sqlite3`). Reclaimed
allocated bytes:340836352. Source templates, raw evidence and other worktrees were
preserved. This storage action adds no engine validation.

Final reader review corrects a metadata counter: a keyed application call emitting
multiple clipped COPY pieces counts once in `cpu_keyed_calls`. A dedicated control
checks two pieces from one caller. The47current Python controls pass again, and
`replay01.json` rechecks all immutable quick02 caller/output bindings successfully.
The live686keyed-call count is unchanged. Only this Python counter and its test
changed after the PE32/live execution;all native engine/test sources match exactly
(`current-validation01.json`). Historical fingerprints/preimages remain preserved;
the new replay is not a new original launch or natural-branch observation.

The final strict reader additionally refuses empty CPU intervals: CPU rows lack an
independent expected-piece count, so a legitimate empty clip cannot be separated
from a missing command by the census alone. Rendering empty clips remains covered
by supplied controls;whole-output proof earns no credit for them. The current live
sample has zero empty CPU intervals.48current Python controls and `replay02.json`
pass after this refusal control;`current-validation02.json` records native-source
continuity and the distinct fresh reader validation.
