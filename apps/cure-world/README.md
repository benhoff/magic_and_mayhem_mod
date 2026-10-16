# ANI-driven Cure on native World

This headless adapter connects the compared direct Cure creator/setup/animation/
release contract to `game::World` identities, effect phases and native snapshots.
It runs without Wine. The native simulation, renderer and asset services acquire
no dependency on reconstruction; the application owns the build-specific adapter.

```sh
python3 tools/working-retention.py run --reserve-gib 0.5 -- cmake -S apps/cure-world -B working/cure-world -DCMAKE_BUILD_TYPE=Debug
python3 tools/working-retention.py run --reserve-gib 0.5 -- cmake --build working/cure-world -j2
python3 tools/working-retention.py run --reserve-gib 0.05 -- ctest --test-dir working/cure-world --output-on-failure
```

The installed demonstration reads `effectani.cfg` and its declared ANI catalog,
selects Cure recipe74, and copies the selected records into the owned session.
Costs3/magnitude8 and the two actors are authored fixtures; installed spell
configuration and balance are separate. If consuming original/derived assets,
verify `tools/original-manifest.sh verify` before and after the experiment.

```sh
python3 tools/working-retention.py run --reserve-gib 0.05 -- working/cure-world/cure-world --assets working/game-nocd --checkpoint working/cure-world-in-flight.bin
working/cure-world/cure-world --resume working/cure-world-in-flight.bin
```

The checkpoint is published through the existing POSIX native snapshot store,
which refuses existing destinations. Resume requires the snapshot and executable;
the selected ANI records and cursor are embedded, so game assets are not reopened.
The demo heals9→17, debits20→17 resource units and releases its native effect entity.

Each admitted World effects phase ticks a deferred Cure's real forward ANI
controller. Type11 transitions/binds77 first, with playback beginning on the next
phase. Exactly raw result1 completes delivery. No caller completion Boolean or
guessed wall-clock timer is supplied. Missing target identities become inactive;
generation reuse never retargets an old spell. World cleanup/release commands own
entity retirement and peer-reference clearing. Dead/inactive targets release
without refund; blocked targets release at completion without healing/cleanup.

`World::State.systems` stores the versioned Cure continuation, fixed actor/effect
handle maps, selected ANI definition, spatial pool/cell chains and animation
cursors. Health/resource/status data live in this owned component; World's entity
slots own identities and positions. Cast/advance/relocation/ordinary health inputs
stage both components and publish together. A refused cast, exhausted World pool
or failed input leaves World, resource and animation state unchanged. The adapter
exclusively owns `systems`; motion components and concurrent creature-AI/campaign systems composition are
not implemented. Generic World snapshots preserve their other fields unchanged.

Native policies are explicit: fixed actor enrollment and effect pool, one ANI
tick per admitted effects phase (including World's alternate mode), validated
generation handles, direct grid relocation/following, terrain1 cell storage,
empty creator contributor chains and cancellation without refund when an external
World effect is cleaned/released. Removed actor slots may be reused by other World
entities but are never silently enrolled into this session. A sequence that starts
stopped can remain pending, matching the recovered raw controller behavior; the
demo refuses if no completion appears within4096 ticks.

Original allocator/reclamation, auxiliary child particles, target-follow helper,
full update dispatcher and real-time cadence remain separate comparison work.
Associated effect audio, physical status audio playback, projectile types6/7/30,
wizard/special targets, negative Cure magnitude, SPR drawing and live campaign
replacement remain outside this integration. The health fixture input uses the
existing ordinary-health reconstruction; it does not add new damage/balance rules.

Reproduce source-bound native32/native64/sanitizer, snapshot and installed-asset
checks with `tools/test-cure-world.py --output NEW-DIRECTORY`. The optimized runner
keeps GCC16's `maybe-uninitialized` warning nonfatal only for the unchanged
movement dependency; new sources retain strict warnings. This run does not validate
movement behavior. See [format](../../research/formats/native-cure-world.md) and
[recovered presentation](../../research/runtime/cure-presentation.md).
