# Covered displacement admission

The full native colour-cache run fails atWorld3708during resumed gameplay with
Scene displacement samples outside canvas. Both mask padding and row-phase
conservatism require investigation; this does not establish cache causality.
The retained V5 slots contain the failed owned source/request; no original
destination pixels are native inputs. The attempted whole-World isolated replay
refuses unknown native producer canvas because earlier native history is absent.
An admission-only source-frame fixture is required instead.

The proposed native policy scans actual covered clipped samples only when the
conservative destination envelope exceeds storage. Renderer mask proof must
match native upload/mutation state; unknown proofs stay conservative. Native
sampling arithmetic/order/AX and original simulation stay unchanged. New
synthetic/captured geometry and full public validation remain pending.

## Confirmed native boundary fix, 2026-10-12

Captured source3708maps3visible displacement draws. One fails the old envelope
predicate even though every covered sample, using absolute destination row phase,
is inside800x600storage. Six atlas/nonatlas draws match an independent scalar
reference at480000words each. This proves a conservative native admission bug,
not new original execution equivalence or colour-cache causality.

Native indexed masks retain one byte of proof per texel for binary CPU uploads.
Unwritten pixels still fail authoritative known-word admission. Nonbinary
uploads and GPU writes clear proof; full binary reload restores it. Native
clone/restore/swap preserve word/proof association and palette updates preserve
index words. Live surface count/pixel limits bound total proof capacities to
16MiB; coverageProofBytes reports retained vector capacities. Unknown proofs
keep the old conservative renderer refusal. Only proven unused snapshot padding
is clipped; actual covered overflow still refuses before drawing.

Eleven mask/proof controls and four renderer regressions pass, withzero ordinary
readbacks, no remaining surfaces and original/input/source checks. The failed
full public journey, unknown-history isolated wholeWorld attempts and historical
FPS/source outcomes stay recorded. Current full gameplay/performance is pending.

## Finite native World edge policy, 2026-10-13

The retained `run-r9nz3st2` V5 generation2354 source, producer1592, admits
backend9/AX0 at anchor763,603 (decoded origin31,31, extent61x47). Its opaque
pixel788,599 samples linear word480001, beyond the logical480000-word canvas.
This is distinct from the previously fixed conservative padding refusal.
The isolated unmodified nocd-40209ca7 entries0x596490/0x5968a4 both execute the
request with explicit authored32-word backing padding and match the independent
linear model over960064words. Each reads one opaque tap outside the logical
canvas. Confidence is high for this input; backing allocation contents in a live
original process remain unknown, and no equivalence is claimed for that tap.

Public native World draws explicitly select `retainOutsideDisplacement`:
covered taps outside owned storage leave the existing destination word intact.
All in-range arithmetic, row phase, mask coverage, original callerAX and valid
linear wrapping remain unchanged. Generic blitter/scene draws remain strict by
default. The shader discards invalid taps before any undefined fetch; no original
padding, readback or destination pixels enter native rendering. This is an
intentional native boundary policy, separate from recovered engine behavior.

[Fresh controls](displacement-finite-edge-controls-20261013.json) pass14 strict,
mask/proof, retained-edge and valid wrapping cases, two captured full800x600
atlas/nonatlas pixel comparisons, and scene/atlas regressions. They retain one
invalid tap, withzero ordinary delivery readbacks and no remaining surfaces.
Original/input/source checks pass. Full public casting/combat, continuity and
FPS validation remain pending; historical failures/evidence are unchanged.
