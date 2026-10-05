# Empty-world effect cell transitions

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

The selected `004883f0` movement path now includes cell changes with membership
updates enabled (stack argument 1). Scope retains emitting types 3/13/22/24/36,
creator -1, metadata kind 0/68 and zero distance allowance, 0..8 iterations,
initial counter 0..32, authored trajectory, empty terrain bits and no creatures.
Touched terrain entries must have catalog ordinals 1..3 and lack `0x40000000`.
These are controlled comparison inputs, not a claim about installed effect setup
or arbitrary original worlds.

After fine-unit stepping and one-period X/Y wrapping, recount coordinates at
`+0x1c2/+0x1c6/+0x1ca` differ from current cells `+8/+0x0c/+0x10`. The parent
calls the real unlink helper `00488330` with the old cell. For a head removal,
it installs the removed record's next WORD as the cell head. For a middle/tail
removal, it finds the predecessor and replaces its `+0x1a0` WORD with the removed
record's next link. A found record's next is cleared to `0xffff`.

The helper has additional terrain cleanup: ordinal zero, all three cell heads
empty, and no `0x20000000` flag can call `00534520`, potentially setting flag
`0x80`. Nonzero terrain ordinals skip this callback entirely. This chunk excludes
that branch; it does not emulate or stub the callback.

The parent saves prior cells at `+0x202/+0x206/+0x20a`, updates current cells,
and replaces its `+0x194` cell pointer. It inserts the effect as the destination
head or appends through the existing chain, then clears destination flag `0x80`.
The original skips insertion on flag `0x40000000` and later exits with result 1;
blocked destinations are an atomic native refusal in this scope. There is no
native successful blocked-cell or membership-disabled movement contract yet.

The new terrain ordinal selects a 356-byte catalog object and replaces record
`+0x190`. Native storage retains an owned ordinal and cell index, rather than
host or original pointers. The oracle checks actual original pointers against
its private cell/catalog allocations after every call.

The real `004ead30` helper compares current cells with the cached cells at
`+0x1f6/+0x1fa/+0x1fe`. X/Y differences greater than half the map period receive
one wrapped correction; absolute differences must each be at most one, including
Z without wrapping. If any difference exceeds one, the parent replaces that
cache with the immediately previous cells from `+0x202/+0x206/+0x20a`. The
existing owned field named `initialPosition` therefore becomes a moving cache
on this path; it is not an immutable spawn position. Adjacent wrapped transitions
can preserve the cache, and later transitions can advance it repeatedly.

Subcell/terrain bit lookup, recount updates, iteration continuation and return 3
retain the previously compared empty-world projection contract. The parent
rebuilds nearby creature pointers after cell changes, including kind 68; empty
heads keep them zero. The original trajectory, unlink, wrapped-neighbour test
and terrain bit lookup remain unmodified throughout the comparison.

`transitionEffectEmptyWorld` advances candidate copies of the entire owned pool
and motion/cache/terrain state. It reuses existing projection admission, checks
pool dimensions, cache bounds, terrain reference and complete membership chains,
then commits both inputs only after all iterations succeed. Missing membership,
invalid/cyclic/duplicate links, unsupported touched terrain/flags, height exits
and excessive coordinate excursions are explicit atomic native refusals. A later
failure also restores earlier valid transitions, links, flags and counters.
The caller must supply empty terrain bits/no creatures; this helper does not
inspect or certify a live game's occupancy.

The [comparison record](effect-cell-transitions.json) binds source fingerprints,
executable/helper hashes, counts and normal/sanitized outcomes. The oracle runs
whole unmodified `004883f0` with real child routines and no patches, stubs or live
game. Fixtures include head/middle/tail removals from a three-effect chain,
append into an existing destination chain, subsequent empty heads, repeated
calls, horizontal wraps, diagonal/large cell jumps, vertical oscillation,
zero iterations and same-cell continuations. It compares the full guarded
four-record allocation, all guarded cell bytes, empty catalog bytes, native
state and original pointer selections after each call. A 32-bit native model
inside the reference helper and separate 64-bit and ASan/UBSan output streams
must match. Native tests verify rollback and that a moved light source clears
its old location and illuminates its new location in the existing headless cycle.

```sh
python3 tools/test-effect-transition.py
```

The recorded run matches 2,048 fixtures and 3,232 whole-parent calls covering
10,288 trajectory steps; the owned one-step trace counts 8,911 cell changes and
5,784 cache advances. The oracle checks 86,639,360 cell-allocation bytes. Strict
CMake/CTest, normal/sanitized unit checks and immutable-original verification
before and after all pass.

Confidence: high within this membership-on, nonzero-empty-terrain path. Terrain
ordinal-zero cleanup, blocked cells, membership-disabled moves, occupied terrain
and creature collisions, other metadata/types/termination, original trajectory
setup, slot recycling/removal and live movement/lighting scheduling remain open.
