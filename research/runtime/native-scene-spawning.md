# NS20: paused native scene spawning

In the native scene window, left-click an exposed terrain cell and press
**Spawn creature** while paused. The new same-profile creature receives the NS19
static initial pose immediately, without advancing time or issuing movement. It
becomes selected, so right-click terrain or the numeric target controls can
order it. The spawn-cell coordinates remain available for another attempt.
Left-clicking a creature selects that identity; clicking outside either kind
clears transient selection. Placement and creature selection are UI state and
are not stored in a checkpoint.

The widget emits a semantic spawn action. `Orders` requires a paused session and
a selected cell, delegates admission to `MovementSession::spawnCreature`, and
changes selection only after successful admission. The native session creates
the bound creature profile through the existing transactional initializer;
`Navigation` owns terrain placement validation behind the application adapter.
The frozen adapter checks ordinary terrain height, footprint cell validity and
support. The first installed attempt exposed an overly strict resource guard:
occupancy marker bits with occupant `0xffff` are empty in the recovered occupancy
check. The adapter now admits that sentinel case while refusing any marked real
occupant ID. It preserves cell bytes and terrain flags, and does not clear
occupants or bypass navigation. Failed installed attempts and their manifest
checks are retained under `working/tests/scene-spawning/`. A synthetic regression
checks both the empty-sentinel and real-occupant branches. Its existing occupancy/reservation policy then rejects overlapping
logical footprints and cells swept by unfinished fine-motion edges, preserving
state, pending orders, tick and selected identity on failure.

New `spawn-terrain-ani` scene checkpoints use the already versioned stationary
occupancy plus multi-creature policy and 32 owned slots. The existing policy hash
is preserved in the checkpoint binding, and the scene's normal resource rebound
loads it in fresh processes. This is a native policy choice; existing checkpoint
admission limits remain encoded by their own bindings. The UI disables Spawn
while playing, without a selected cell, or when its actor/pool capacity is full.
The semantic action also rejects playing calls, so widget enablement is not its
only guard. An occupied or unsupported cell reports a placement error and keeps
the previous creature selected.

Evidence is recorded in `native-scene-spawning.json`. Synthetic tests use the
production Qt controls, orders, native session and frozen adapter for initial
spawn, invalid/occupied/unsupported/reserved placement, paused/playing guards,
32 actors with spare pool slots, selected actor orders and exact two-actor
checkpoint continuation. The installed Forest/Redcap experiment executes the
actual entry-point window in all four diagnostic views: select terrain, spawn a
second body at tick zero, reject a repeated occupied placement, Save with the real
modal dialog, order the selected new actor, and replay that order after picking
its body in a fresh window. Real-timer playback disables spawning. Complete frame
pixels match an independent decoded-SPR rasterizer; saved state and canonical
JSON/RGB565/PNG continuation artifacts match exactly. Prior initial-body,
mid-motion and stopped-body window scenarios remain exercised.

Installed inputs are read only and original-manifest checks bracket their use.
Historical NS18/NS19 evidence remains unchanged and can be source-stale. These
are native bounded preview claims. Original spawn lifecycle/admission, factions,
creature configurations beyond the bound profile, animated idle, initial facing,
full-game spawn integration, gameplay balance and live replacement remain
pending. No recovered original-comparison or replacement status is promoted.
