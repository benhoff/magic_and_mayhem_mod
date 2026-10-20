# High tranche 2 results — 2026-10-19

The [implementation review](high-tranche2-implementation-20261019.md) closes the
selected missing read model and reconciles the surface umbrella. All seventeen
tasks have bounded implementations; their original understanding, comparison,
integration and replacement boundaries remain independent.

`HIGH02.distortion-word-read.comparison-20261019` retains the new
[original/model result](high-tranche2-distortion-read-comparison-20261019.json).
All eighteen original leaf cases match the safe reconstruction: two dimensions,
three final source positions and zero/one/two readable tail bytes. Four fault
cases match instruction `5965f7`, the effective source operand and the first
inaccessible byte. Fourteen cases return zero and store the expected low WORD.
Thirteen native controls include invalid input, endian/unused-tail checks and
protected-page refusal. Input and declared source hashes are stable; original
manifests pass before and after. Implementation is registered partial and the
selected original comparison recorded; integration/replacement stay none.
The exact crashed live request, other leaves and allocation ownership remain
unknown. Native padding and finite-edge policy are separate implementations.

The [surface crosswalk](coverage/high-tranche2-surface-crosswalk-20261019.json)
is static evidence. It identifies delivered scoped suppliers for all twelve
required operation families and retains four conditional discovery domains.
It promotes no surface engine stage. Five portable corpus cases compare, but
ten required operation cells are missing from that corpus; its
`milestone_complete` remains false.

Retained outputs are under
`working/tests/high-tranche2-implementation-20261019/`. Sixteen selected CTests
pass: animation forward/placement/attachments, sprite queue/visibility, the new
read model, sprite upload, sprite/layer scenes, shared scene rendering, native
blits, persistent surfaces, command replay, terrain shading/light-cycle and the
bounded World scene. Five corpus-validator tests pass. Seven reconciliation
schema/source tests pass. The thirty-one planning tests have twenty-nine passes
and two failures from concurrent work: three unrated retirement behaviors and
an additional historically changed scene-runner source. The seven High planning
checks, including the new read model and surface crosswalk, pass.

The native regression runner's broad source-stability flag is false because
the two dashboard test files changed while it ran. No compiled dependency was
among those changed files. This report claims the observed test results, not
complete scope-bound current validation for the other sixteen contracts.
Some compiler inputs were outside that runner's broad declaration; no aggregate
source-equivalence claim is made. The new original/model result has its own
complete, stable declaration and is unaffected.

Earlier attempts remain retained: `read-001` refused a modified working
executable hash, `read-002` was stopped by sandbox 32-bit syscall restrictions,
and `native-001` could not start Xvfb inside the sandbox. `read-003` and the
isolated-Xvfb `native-002` supply the successful measurements. No game session
ran, no original file or binary was patched, and no evidence was deleted.
Storage guards retained the approved 90 GiB reserve.

Repository-wide audit/gate completion is blocked by concurrent incomplete
DirectDraw-retirement scenario records (page hints, shutdown controls, live
shutdown and startup-probe retirement). Their missing execution evidence is
not invented or removed. Additive dashboard source links are marked pending;
old fingerprints, classifications and execution results are preserved. The
draft gate command and its refusal are retained; no whole-tree gate pass or
commit is claimed. Exact content receipts for this change remain separate from
those unresolved concurrent scenarios and broader gameplay/live acceptance.
