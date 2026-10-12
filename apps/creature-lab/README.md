# Native creature lab

Four Crocodiles acquire enemies, pursue them with 2×2 footprints, attack only
when recovered reach allows it, apply melee damage, interrupt on knockback or
stun, recover, and retarget after a death. Play/Pause/Step/Restart and API or
window Save/Load preserve the complete native continuation. No Wine is required.

Build under the repository storage guard:

```sh
python3 tools/working-retention.py run --reserve-gib 2 -- \
  cmake -S apps/creature-lab -B working/build/creature-ns25/normal
python3 tools/working-retention.py run --reserve-gib 1 -- \
  cmake --build working/build/creature-ns25/normal --target mnm-creature-lab -j4
working/build/creature-ns25/normal/mnm-creature-lab --installation working/game-clean
```

This is a bounded ordinary-creature simulation, not complete original AI.
Recovered selectors and control routines use explicit admission endpoints.
Movement is native flat toroidal BFS, all creatures are visible, knockback
relocates in one tick, and animation selection explicitly chooses attack24 and
stun112 plus facing. Original movement timing, special species, spells, orders,
effect ownership and original full-encounter equivalence remain pending.
An encounter also stops after 160 ticks without control progress; this is a
native lab stalemate policy, not a recovered game rule.
