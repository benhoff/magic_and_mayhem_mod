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
