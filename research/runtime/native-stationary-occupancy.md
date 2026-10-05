# Stationary occupancy milestone (NS10)

## Refreshed recovered predicate

The accepted isolated comparison preserves original `004f3550` occupancy and
`004f47d0` footprint instructions. All **9,509 direct cell cases** and **576
footprint cases** match the recovered model: low flag mask 3, exempt `ffff`,
full-DWORD self identity, foreign identity exclusion, vertical extent/clipping,
signed overflow, nonstandard tables/stride and width-two seam wrapping. Fixture
cells, parameters and context remain unchanged. The existing private PE loader
redirects five unrelated lower movement helpers; neither compared function calls
them. No disk executable is patched and all 2,927 original files are verified
before and after. Invalid pointers, original occupant creation, lifetime and
movement reservation production remain unvalidated.

[Immutable original record](native-stationary-occupancy-original.json) preserves
source hashes and the accepted working report. Reproduce with
`python3 tools/test-dynamic-occupancy.py`; Linux x86 and `g++ -m32` are required.
A sandbox must permit disposable private PE32 mappings.

## Native integration boundary

The opt-in layer admits one moving creature with stationary same-profile
blockers. Native generational handles, logical-cell box placement, overlap and
boundary refusal, cleanup/release and checkpoint rebuilding are intentional
native policies. They do not recover original occupancy production. Full moving
creature scheduling, reservations, fine-position collision/combat, original
orders, camera mapping and live replacement remain separate milestones.


## Owned blocker policy and checkpoint continuation

`game/simulation/occupancy.*` builds owned, nonwrapping logical-cell boxes from
live native creature slots and one explicit bound profile. It checks generational
handles, coordinates, extent, same type, nonoverlap and a 4,096-cell limit.
Cleaned/released creatures contribute no box; stale handles never remove a
replacement. Other entity families do not contribute occupancy in this slice.

The frozen adapter opts in with `stationaryOccupancy=true`. It copies recovered
navigation inputs and projects each admitted handle slot to a WORD occupant token
with low flag bit 1. Slot 65535 and higher refuse; the full native generation is
checked before token projection. Other cell bytes and immutable terrain remain
unchanged. Self occupies its current logical box and remains exempt. This token
projection/flag choice is native policy, not recovered original placement.

The original selected-goal `special` acceptance branch can skip occupancy after
terrain admission. Native blocker mode deliberately uses ordinary acceptance for
all candidates, including the goal, and refuses candidate boxes crossing XY/Z
bounds. Original wrapped footprint and special-goal semantics remain unchanged
in the recovered model and in the default adapter mode.

Native movement plans using the current world view and rechecks the next edge
on every maintenance tick. A late blocker stops the creature, clears the route
and fine continuation, and returns display position to the logical origin. This
stop/reset behavior is deliberate policy; there is no automatic retry, collision
resolution or fine-space reservation. A new move after cleanup/release can plan
again. Only one creature owns a motion driver; stationary blockers cannot receive
movement commands. Spawn/refusal and restore remain transactional.

The existing checkpoint entity records preserve blockers, generations, cleanup,
commands and motion. Occupancy is rebuilt after restore; no occupancy cache or
new wire version is persisted. The resource fingerprint includes the versioned
`stationary-occupancy-v1` policy, so default-mode resources cannot accidentally
admit its checkpoint. Saved route geometry is validated against the immutable
baseline; a saved late obstruction is handled on the next tick rather than
silently dropped or making a valid continuation unloadable.

Use the headless sandbox with a bounded occupancy-free frozen map:

```sh
mnm-world-sandbox move-occupied MAP OUTPUT SX SY SZ TX TY TZ BX BY BZ TICKS
mnm-world-sandbox resume OUTPUT CONTINUED TICKS
```

The `move-occupied` command authors one mover and one stationary blocker. The
native API `MovementSession::spawnBlocker` admits additional stationary boxes.
The checkpoint-aware sandbox resolver rebinds the correct policy automatically.
The current scene preview still admits its existing single-creature presentation;
showing multiple creatures is a separate integration milestone.

## Native validation

`tests/native-occupancy-test.cpp` checks owned height/width coverage, self and
stale-generation identity, width-two self overlap and blocker exclusion,
nonwrapping bounds, malformed identities/baselines, blocked goals, cleanup and
release, handle reuse, transactional spawn/restore refusal, a newly blocked edge
and cancellation during sample-driven fine movement.
`tests/test-native-occupancy.py SANDBOX TEST [NEW_OUTPUT_DIRECTORY]` adds a direct
path blocker with detour arrival and three fresh-process checkpoint continuations
per build, including a saved obstruction and an occupied goal. Complete final
checkpoint bytes agree between split and uninterrupted runs. It uses synthetic
owned inputs and does not consume original game artifacts. All 101 normal CTests and eight focused ASan/UBSan suites pass. Accepted normal
and sanitizer runs preserve three fresh-process continuations per build in the
[immutable native record](native-stationary-occupancy-native.json); earlier
sandbox restrictions and exploratory runs are not counted as proof.

Remaining boundaries: original occupant insertion/removal and slot/flag meaning,
world pointer lifetime, fine-position reservations, moving-creature scheduling,
path retry policy, effects/map-linked blockers, multiple-creature presentation,
original orders/actions and live observation/replacement.
