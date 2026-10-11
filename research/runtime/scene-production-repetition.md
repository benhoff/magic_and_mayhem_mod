# Repeated original scene production experiment

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
protected difference or refusal. No state rollback is performed. The file flushes
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
Actual TITAN mode3 execution is pending. This experiment does not change the50ms simulation or activate a
default rendering schedule; full nested/map/visibility/timer/pixel safety remains
outside its scope.
