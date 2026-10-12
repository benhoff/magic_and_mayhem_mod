# Native combat sandbox

`mnm-combat-sandbox` runs a bounded autonomous two-creature melee encounter
without Wine, Qt or original assets. It uses the existing recovered navigation
adapter with synthetic inputs and explicit native combat rules. See
[scope and evidence](../../research/runtime/native-combat-encounter.md).

Build as part of `game`, with the working-retention guard:

```bash
python3 tools/working-retention.py run --reserve-gib 0.2 -- \
  cmake -S game -B working/build/native-combat-ns21 -DCMAKE_BUILD_TYPE=Debug
python3 tools/working-retention.py run --reserve-gib 0.2 -- \
  cmake --build working/build/native-combat-ns21 -j4
```

For a new output directory, generate a synthetic frozen map, create the encounter
and trace it. `create` starts at tick zero; `resume` saves after a bounded tick count.
Each trace row reports positions, targets, health, action clocks and combat events.

```bash
python3 tools/create-movement-fixture.py working/combat-fixture.frozen
working/build/native-combat-ns21/mnm-combat-sandbox create \
  working/combat-fixture.frozen working/combat-start.mnms
working/build/native-combat-ns21/mnm-combat-sandbox trace working/combat-start.mnms 40
working/build/native-combat-ns21/mnm-combat-sandbox resume \
  working/combat-start.mnms working/combat-half.mnms 7
working/build/native-combat-ns21/mnm-combat-sandbox resume \
  working/combat-half.mnms working/combat-finish.mnms 33
```

Use `create-fine` instead of `create` to use the recovered intra-cell terrain
motion driver. `inspect` reports the current state without advancing it. Saves
retain both actors, movement and attacks in progress. Movement-only advancement
refuses these checkpoints; use the combat executable to continue them.

The demonstration's health 20, damage 6/4 and windup/recovery 2/3 are native test
values. This is a headless encounter milestone; full AI, original combat
equivalence, spell/mana systems and campaign simulation remain outstanding.
