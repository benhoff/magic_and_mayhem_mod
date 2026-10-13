# Read-only descriptions and native drawing continuity

A missed `GetSurfaceDesc` observation was treated like a missed drawing operation.
At the copy lease timeout, `surface_desc` advanced metadata/pixel epochs before
and after forwarding the original; at callback-gate and tracker admission it
also used mutation invalidation. The query reads a descriptor into caller output;
its native facade implementation `ns_desc` changes no surface state or pixels.
Dropping a query observation cannot make a previously owned canvas incomplete.
It also cannot establish new metadata: unknown information stays unknown.

The retained pre-fix PE32 `desc-timeout` fixture performs a failed query and a
successful retry while an admitted original copy owns the drawing lease. Exact
arguments, return values and LastError are preserved, but the native session
refuses with transport GAPreason2 / internal epochreason1. The original/scalar
frame remains valid. A bounded epoch diagnostic records pixel epoch0->8,
metadata epoch4, last metadata tag7 at `bridge.c:190`. This is a confirmed false
continuity refusal for that schedule. Historical public `run-y6hr9adu` also
records an epoch mismatch, but has no invalidation site: this finding does not
establish that every public GAP shares this cause.

The repair applies only to surface-description reads (COM slot22). Callback
admission retains active counts and collision refusal but marks this callback
read-only. Copy admission keeps the existing50ms bound and original-once
forwarding without recording a missed mutation. Tracker admission skips the
observation without invalidating epochs. Admitted returned descriptors still
run existing layout/source-key checks; real observed changes can still invalidate
pending copies. Mutating callbacks, interface/lifetime operations, unknown
writes and unsupported raster operations keep conservative refusal semantics.
No returned descriptor from a skipped observation is used as new native proof.

Metadata invalidations retain bounded source-site/thread/epoch diagnostic tags.
The failure log samples those fields only after a real epoch mismatch. Tags are
diagnostic clues, not a coherent ownership snapshot under concurrent writers,
not a wire contract and not permission to recover incomplete history.

Validation separates three forced admission failures (copy, tracker, exclusive
callback gate), ordinary cross-thread observation, changed descriptor rejection
and actual mutation timeout rejection. Original-game and driver equivalence,
full public gameplay/FPS on final sources and the exact provenance of the old
public GAP remain pending until separately recorded.

## Current headless repair proof

[Fresh controls](readonly-description-controls-20261013.json) pass all eight
cases. `desc-timeout`, `desc-tracker`, `desc-gate` and `cross-desc` complete two
independently authored native frames each with unchanged pixel/metadata epochs.
The exclusive-gate case still records administrative collision and forwards the
failed/successful original queries exactly. `meta-change`, `copy-timeout`,
`lock-timeout` and `add-timeout` each retain GAPrefusal and present only the prior
complete frame. Twelve full-frame comparisons match, ordinary native/RGBA
readbacks/viewport uploads arezero, and terminal native surfaces arezero.
The first selftest attempt used the wrong executable/DLL boundary for barriers;
a subsequent missing undecorated export aborted setup. Those failed setup logs
and prospective declarations remain underworking; neither is engine validation.

Before the read-only fix, the full TITAN route passed4476balancedWorlds with
casting/lethalcombat/spells/portraits/resume/Quit,19.12engine and50.11nativeFPS.
Its source fingerprints are historical after this repair. Separate repeats retain
an18.48engineFPSfailure, a menu-return timeout without a rendering error, and a
cold-shader-cache fullfunctional pass whose18.99486engineFPSfailed19floor.
Those failures are retained; no threshold was relaxed. The old public GAP's
exact site provenance and final-source public/FPS proof remain separate work.
