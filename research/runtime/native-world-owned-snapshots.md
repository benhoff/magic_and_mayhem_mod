# Owned native World snapshots

Native policy work separates original20Hz production, owned scene validation,
native World execution and GUI import. V4 continues to acknowledge submitted
native GPU work; new opt-inV5 acknowledges source ownership and a validated
native handoff. It cannot assert GPU completion or original equivalence.

The capture owner verifies actual original closing records and immutable prefix
before slot release. At mosttwo snapshots may be outstanding until GUI import;
no frame is dropped or reordered. Backpressure remains finite and visible.
All native producer history is applied once per source update. Independent draws
must restore native destination-dependent baselines, reuse owned prepared draws
and recompose native HUD operations; they cannot advance original simulation.

Controls, producer/consumer failures and full TITAN gameplay are separate pending
milestones. Existing V3 pixel and V4 submission modes remain separately admitted.


Owned snapshot handoff milestone (2026-10-12): explicitV5/mode2/MNMSACK1
accepts owned validated scene source before native GPU execution. One capture
owner verifies unchanged prefix and exact actual return before slot release;
two outstanding snapshots include taken/native/GUI work until ordered import.
Mapped held-render/backpressure/overwrite/12failure/cancel/join controls,512legacy
V4leases and independent PE32 exact80-byte ACK/all80mutation controls pass with
source/original/private cleanup. Build setup/type failures remain under
working/tests/world-owned-snapshots. Native policy implementation is partial and
integration headless; live original NOACCESS/bind/pacing/full gameplay and the
independent native render/UI recipe are pending. No60FPS or original comparison.
See `world-owned-snapshot-controls-20261012.json`.


Prepared independent rendering milestone (2026-10-12): opt-in
`--resident-world --independent-world` uses the owned V5 capture worker and a
GUI-owned native rendering clock. Actual source return/import credits remain
ordered and bounded at two; the original source producer is never called for
extra native frames. The default V4 route remains separately admitted.

Native World drawing restores its exact GPU baseline and reuses resolved native
scene draws/resources. Nine storage controls,40destination-dependent repeats,
two source updates and four saved-source repeat comparisons pass in
`native-prepared-world-controls-20261012.json`. Source history advances only on
engine updates. Native HUD replay retains versioned GPU inputs at their actual
read boundaries: static uploads/copies are folded once, while World-dependent
destinations restore and execute their ordered native operations per frame.
`native-frame-recipe-controls-20261012.json` records30World/HUD repeats,
source mutation/deletion, partial-frame refusal,100folded uploads, versioned
masked writes,10swaps and indexed palette ownership. Ordinary readbacks and
teardown surfaces are zero. Diagnostic output reads are confined to the tests.
Resource budgets and conservative terminal refusal remain in force.

Live evidence remains negative. The first attempt exposed a null capture owner
at cold start, now fixed and regression-tested. The second attempt executed
7107native World frames (3661extra), reaching38.36569renderFPS with
18.64224sourceHz. It missed3327deadlines and later terminated on native command
history GAP reason2, with pending imports and no successful normal Quit. Its
bounded trace records a cross-thread copy-order timeout: waiting thread0x1dc,
owner0x194, owner/waiter site0x101, phase4 and native stage0; the wait/owner ages were
157/381ms despite the50ms policy. Phase4 follows original BltFast return and
precedes tracker commit. This locates an admission/commit stall; it does not
establish which scheduler/tracker/driver caused it or justify extending bounds.
The source-stable negatives remain immutable in
`independent-native-startup-negative-20261012.json` and
`independent-native-first-render-negative-20261012.json`.

Static HUD folding and separate extra-World/HUD submission timing were added
following this failure; full TITAN gameplay/pacing/Quit/performance must be
rerun. No60FPS, original equivalence, live replacement or interpolation claim
is made. The engine still produces scene state; native simulation/interpolation
remain separate work. Committed history e3b8400a..b3757828 was reviewed against
18exact parent/current file receipts with zero unresolved accounting gaps;
this is history accounting, not retrospective validation.
