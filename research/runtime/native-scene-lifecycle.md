# Owned native scene lifecycle

This is an intentional native application policy, not a recovered original-world
constructor, scheduler or shutdown contract. It closes the scene track's owned
session lifetime within the existing bounded frozen-navigation preview.

## Ownership and boundaries

`apps/world-scene/session.*` owns the navigation provider, native MovementSession,
transient Orders selection/placement and Playback timer. It depends on Qt Core,
native simulation and the existing frozen adapter; it contains no widgets,
renderer, original-game pointers, Wine process or wire layout. Widget controls
remain semantic actions in the presentation layer. Native simulation remains
independent of Qt and build-specific reconstruction.

Snapshot loading preserves the existing single-creature, stationary-occupancy
and multi-mover admission policies and base geometry fingerprint. World and
navigation validation finish before the owner is exposed. A failed construction
cannot mutate an independently active scene. An explicit zero geometry
fingerprint is retained as a value.

The owner begins paused with no selection. Playback is the simulation handoff;
rendering does not run another simulation or animation clock. Checkpoints retain
native state and queued commands. Selection and placement are transient and are
not inherited by a fresh window. Restart creates a new owner rather than
replacing storage behind outstanding borrowed references.

Terminal close retires timer and direct tick admission, clears selection and
placement, and releases simulation/navigation. Repeated close is harmless.
If close occurs inside a simulation step, storage remains alive until that step
returns; no later batch tick or refresh is admitted. Failed ticks retain the
last committed native state and pause, as before. These are thread-confined
application contracts, not concurrent-access guarantees.

The production window withdraws presentation callbacks on `aboutToQuit` before
closing the owner. The test-only window driver records shutdown after the real
Qt application loop returns, including a window closed while its playback timer
is active. The existing native scene renderer's regression independently checks
that its scoped GPU surfaces are released on destruction.

## Reproduction

```sh
python3 tools/working-retention.py run --reserve-gib 1.2 -- \
  python3 tools/test-scene-lifecycle.py
```

The runner declares exact behavior/scenario identities and source hashes before
execution. It builds privately with Debug assertions, checks compiler dependencies
against that declaration and checks source/input stability afterward. Generated
SPR, TTD, ANI, frozen-map and native-checkpoint inputs drive the production entry
point in four views and all three admitted occupancy policies. A private software
X server is allocated atomically for each display check; only that owned child
is stopped. No manual playtesting, original media, installed game input or Wine
session is used.

The matrix includes native lifecycle/wire, semantic Qt controls, stop, playback,
spawning, picking and scene-renderer controls; fresh-process native checkpoint
continuation; fresh-window pending Stop continuation; independent complete
RGB565-plane comparisons; failed checksum/map/binding admission; destructor and
in-flight close; and address/undefined sanitizers on the owner/controls fixture.
LeakSanitizer is explicitly disabled under tracing and is not claimed.

## Evidence and remaining work

Evidence `SCENE.lifecycle-preview-20261020` points to the immutable
[execution record](native-scene-lifecycle-controls-20261020.json). The private
run `working/tests/scene-lifecycle/run-zaad2xi4/` passes all nine selected CTests,
eight production-entry-point window runs, four exact fresh-window continuations,
28 complete RGB565-plane comparisons and three exact startup refusals. Both normal
and address/undefined-sanitized owner fixtures pass exact fresh-process native
checkpoint continuation. All 485 declared sources and generated inputs remain
stable. Four windows close while playing; four close paused. Every application
shutdown clears selection/placement, retires playback and closes its world owner.

The first sandbox run `run-vrr1m1ur` passes native lifecycle, wire and owner checks
but aborts six display checks before usable Qt presentation. Its source snapshot,
logs and failure report are retained. The subsequent run uses owned atomic
display allocation with host-local X11 access; no original-game work is involved.
Prior historical evidence and hashes are unchanged.

`WORLD.owned` and `WORLD.sandbox` have scoped implementation and preview
integration with current execution support. The scene dashboard's five declared
milestone requirement lists are now satisfied. Its four earlier milestones
retain their historical evidence/current-validation boundaries; changing the
shared entry point, playback and test driver does not renew their complete
installed-profile scenarios. `NP.scene-playback` records the added retirement
test/display dependencies with validation pending for that broader domain.

Complete original-world construction, all native CLI commands, arbitrary creature
and map profiles, original input/pause/cadence, original-game visual equivalence,
hardware/performance/fault recovery, full saved-world restoration and live
simulation replacement remain separate. Completing the five declared scene
milestones does not convert every overlapping protocol or hosting behavior into
a complete engine implementation.
