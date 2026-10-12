# Repeated original scene production experiment

Current candidate:004fd315..004fd5c1 includes original queue cleanup and applies
extra-only position-history suppression. Final33 PE32/52 byte-guard controls and17
decoder refusals pass. Corrected TITAN state/scene equality and normal Quit are
pending; earlier failures remain historical evidence, not safe defaults.

Pinned No-CD hash40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168.

The previous engine-cadence experiment found that full gameplay updates couple
counter and ANI advancement to World production. This experiment isolates the
original scene producer004fd280 and candidate body004fd315..004fd5ad, preserving
the original50ms target and simulation. Extra native Worlds require fresh exact
state comparisons; no state rollback, ignored critical differences or60FPS claim.

Static findings: outer producer decrements object+99, prepares creatures through
0051e260, builds/sorts/consumes the World and then performs creature/camera
post-work including0052ab80 and orientation changes. The narrower body includes
original scene generation004fcf50, sort/visibility and actual5002a0 consumption,
with its original caller PCs and graphics ownership; pre/post state work is
excluded. Complete helper/field roles and slice safety remain hypotheses pending
execution. Counters/ANI/RNG/scheduler and whole entity pools will be protected by
exact byte comparisons. All module/renderer differences remain recorded and
unclassified unless separately recovered. Complete gameplay-object/map-context
headers are conservatively protected; nested map/visibility pointers are excluded.

The creature preparation helper can increment target creature+6e4; post-draw
0052ab80 clears+6e4 across active creatures. Do not infer a pure draw contract
from this pairing. RNG state includes both indices and all250 table words.
The primary call is0046bbd3 (return0046bbd8, ECX00689930); alternate0054af50
is excluded. Static export verifies the executable hash and anchor bytes, retaining
six assembly ranges and both direct callers. Findings are build-specific and partial.

Final-source controls pass18 authored PE32 cases: full/slice caller registers,
flags,512-byte FP/error state and exact stack restoration; once-only normal/extra
queue counts;28 single-byte guard refusals; counter, whole-creature, scheduler,
last RNG word, gameplay/map-context mutations; pool shape/unreadable refusals.
These exercise the hook against stand-ins, not original game/driver equivalence.
Three preceding fixture negatives retain their hashes: absolute ESP/call-site and
unseeded FP comparisons, then an overly strict single-refusal expectation. The
final fixture seeds x87/SSE/MXCSR and tests stack restoration per invocation.

Live admission starts after512 primary calls, samples every128, caps at16 repeats,
requires the ordinary caller/object/mode and owning thread, and stops at the first
protected difference or refusal. No state rollback is performed. The initial file flushed
only at detach. Snapshots and independently copied36-byte scene records allow
offline exact comparison; scene equality is distinct from protected-state equality.
Both original candidates fail exact creature invariance on TITAN. Full production
passes its first repeat, then changes five bytes in slot0+760/+768 and
slot3+76c/+770/+774 on repeat2. The narrower body changes slot0+774 on repeat1.
Each repeat still emits one native World with identical ordered760/762 scene
records. Counter/sequence, whole gameplay/map headers, ANI, RNG, scheduler and
other protected pools are unchanged in those retained pairs. This does not excuse
the creature writes: the probes fail and stop, retaining full before/after bytes.
Both independent casting/combat/refusal/movement/portrait/Mini/normal Quit journeys
pass19FPS, source stability, original verification and private cleanup. The full
campaign records4459 balanced Worlds at19.44656 aggregate FPS. Module/renderer
differences remain unclassified; independent60FPS and safe defaults remain pending.

Static confirmed helper0051fa60 performs a24-byte overlapping copy from creature
+754 to+760, then writes two arguments to+754/+758 and the value selected through
creature+b4 to+75c, returning RET8. One direct scene-builder call is004fa4ef,
return004fa4f4. These three12-byte records are interpreted as draw position history;
full pointer/coordinate/copy-helper/consumer and indirect-call semantics remain
partial. `RS.creature-draw-position-history` preserves that recovered boundary.
The observed changed fields agree with its copy destination; static attribution
does not by itself prove every runtime write came from this helper.

Diagnostic mode3 (`frozen-history`) applies an explicit native policy: keep all
normal calls and original return PCs, and skip only the guarded history call during
a validated extra slice on the owning thread, with a creature-pool ownership check.
No state is restored or omitted from comparisons. Wireversion2 records primary
history forwards, extra skips and extra forwards; the decoder retainsversion1
compatibility. A live pass requires at least8 cases with protected equality,
identical scene rows, positive matching primary/skip counts and zero extra forwards.
Final-source30 PE32 mutation/guard/forwarding controls and17 decoder refusals
pass, including negative history-count/scene-equality gates andversion1 compatibility.
The initial TITAN mode3 journeys failed; see the corrected boundary below. This experiment does not change the50ms simulation or activate a
default rendering schedule; full nested/map/visibility/timer/pixel safety remains
outside its scope.


The first frozen-history journey fails player Fireball enemy-damage verification
after summon/movement;1792 balanced Worlds run at19.97688FPS. An unchanged retry
verifies ranged damage/combat input events, then Wine faults reading/executing
address0 and World delivery stops; lethal verification fails. Both source/original
and private cleanup checks pass; neither reaches normal Quit, so the initial
detach-only production trace remains empty. Preserve these negatives. Crash
attribution is unknown; do not infer state invariance from an empty trace.

Confirmed slice defect: its old exit004fd5ad precedes original cleanup. Retained
renderer bytes show count+231 and writepointer+22d remain populated. Original
004fd5ad..004fd5c1 zeroes+231 and sets+22d from base+229; subsequent production
relies on that state. The corrected exit004fd5c1 includes those instructions,
replays its7 normal bytes (MOV AL,[005e140c]; TEST AL,AL) and resumes004fd5c8.
`RS.scene-queue-retirement` records this partial original contract. This fixes the
leaked queue postcondition; a causal link to the page fault is not yet established.

Wireversion3 adds the complete16-byte queue header as protected kind19. Extra
admission requires the primary queue count0 and writepointer==base; before/after
header bytes stay exact, with no restoring of a saved state. The guarded original
cleanup executes normally. Final33 authored PE32 controls pass all three modes,
52 single-byte guard mutations, queue postconditions and preexisting leaked-queue
refusal before an extra.17 malformed decoder cases, negative history/scene count
gates andversion1/2 compatibility pass. Full renderer/driver/allocator and nested
map/visibility/helper semantics remain partial; none of these are original helper
execution comparisons.

The probe now flushes once at the first repeat failure or16case bound, then
disables probing; fewer admitted cases flush at detach. No per-frame write is
introduced, but this checkpoint timing is instrumented. Trace totals cover the
probe window, while the separate campaign retains the full journey. Failed
campaigns still fail even when a checkpoint proves protected equality. The wrapper
records campaign/source/original/cleanup outcomes before attempting to decode a
possibly missing trace. Corrected live state+scene+queue and normal Quit proof
remain pending; simulation pacing and default launch remain unchanged.
