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
TITAN casting/combat/menu/Quit execution and safe repeat scheduling remain pending.
