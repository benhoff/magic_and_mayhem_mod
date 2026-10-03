# Creature move acceptance: 0x00514360

2026-10-02. No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred VAs; relocate against the loaded image for live work.
No executable patch or game launch was performed.

## Evidence and confidence

`./tools/export-search-support.py` exports complete instruction-aligned ranges
`0x514360..0x51462b` and `0x4f5a40..0x4f5aec` after checking the exact executable
hash. This run is `working/decompiled/search-support-rastmtx0/`; its manifest
records artifact checksums and `input_unchanged: true`. The existing 14 pinned
artifacts and 1,686 assembly byte chunks also passed verification against the
working executable. Original media was not consumed.

Confidence is **high, static** for the instruction-level findings below.
Collision/occupancy interpretations are provisional: lower helper semantics
and live equivalence remain unconfirmed.

## Boundary and member layout

`0x514360` takes the creature in ECX and eight DWORD stack arguments: source XYZ,
destination XYZ, category output pointer, and a flag. `ret 0x20` confirms this
size. After locals and saved registers, steady ESP offsets are source
`+0x4c/+0x50/+0x54`, destination `+0x58/+0x5c/+0x60`, category pointer `+0x64`,
and flag `+0x68`. Only the flag's low byte is tested; the generator supplies 0/1.

`0x5143b2..0x514418` constructs/copies a 22-byte member using `0x40e250`:

| Offset | Source |
| --- | --- |
| +0x00 DWORD | `[creature +0xac] +0x0c` |
| +0x04 DWORD | `[creature +0xac] +0x08` |
| +0x08 DWORD | `creature +0x104` |
| +0x0c DWORD | `[creature +0xac] +0x44` |
| +0x10 byte | zero |
| +0x11 DWORD | `creature +0xa8` |
| +0x15 byte | 1 if creature byte +0x723 is nonzero OR creature DWORD +0x5ec equals 14; otherwise 0 |

Fields retain source-offset names. Flying/swimming/size/terrain labels have not
been established. The host member is packed with compile-time size/offset
checks. Object state comes from a provider, never a host dereference of an
engine pointer token.

## Acceptance sequence

1. `0x514369..0x5143b0`: calculate `wrapped_dx² + wrapped_dy² + dz²` with DWORD
   multiplication/addition. The eventual `<= 1` comparison is signed. No sqrt.
2. `0x514414..0x514477`: call `0x4f3990` with source/destination XYZ, the member,
   category output, and final override -1. Zero rejects and sets category 5
   (`0x51447c..0x51448d`).
3. `0x514490..0x514496`: nonzero flag returns success after that base test,
   bypassing all cell tests. It does not bypass the base movement test.
4. `0x51449c..0x5144d7`: call `0x4f5a40` on the destination. Zero rejects and
   resets category to 5. This check precedes both exemptions below.
5. `0x5144da..0x5144ed`: reread `[creature +0xac] +8`. If it equals 2, return
   success. Otherwise return success if the signed squared distance is <= 1.
6. Test the source-plane AND group below, stopping at its first failure. If
   all pass, return success; otherwise test the destination-plane AND group,
   stopping at its first failure. Neither group passing rejects with category 5
   (`0x51460b..0x51461a`). Success preserves the base helper's category.

For source `(sx,sy,sz)` and destination `(tx,ty,tz)`:

| Group | First | Second | Third | Calls |
| --- | --- | --- | --- | --- |
| Source plane | `(tx,sy,sz)` | `(sx,ty,sz)` | `(tx,ty,sz)` | 0x514518, 0x514546, 0x514574 |
| Destination plane | `(sx,sy,tz)` | `(tx,sy,tz)` | `(sx,ty,tz)` | 0x5145a6, 0x5145d4, 0x514602 |

The combination is `source_group_all_pass OR destination_group_all_pass`.
These are mixed triples, not interpolation points. Repeated cells on planar,
vertical, and seam moves are not deduplicated. The engine can test the same
cell repeatedly. Interpretation as corner-cut prevention remains a hypothesis.

## Cell wrapper: 0x004f5a40

This wrapper takes XYZ plus creature ECX, rereads the same state, constructs
the same member, and calls `0x4f47d0` at `0x4f5ae0`. The lower helper receives
XYZ, creature DWORD +0, and the member; its receiver is map context `0x6c5490`.
The wrapper returns EAX and pops 12 argument bytes.

`test_creature_cell` reconstructs that wrapper. It obtains fresh state for
every invocation. The acceptance function also rereads type +8 after the
destination test. Member values are not cached across callbacks, so state
updates between checks can be represented.

## Implementation and remaining dependencies

`reconstruction/pathfinding/route_creature_acceptance.hpp/.cpp` implements
member construction, `test_creature_cell`, and `accept_creature_move`.
`with_creature_acceptance(neighbors, acceptance)` replaces mode-zero generator
acceptance and forwards mode-nonzero requests to its previous callback. Map
dimensions must match. Supply `CreatureAcceptanceHelpers::object_state`,
`movement_test` for `0x4f3990`, and `cell_test` for `0x4f47d0` before using it.

These two lower helpers remain explicit dependencies, not generic walkability
guesses. Record resolution, `0x5205b0` scalar computation, and real map loading
remain separate work. The top acceptance routine and its cell wrapper are
reconstructed; complete collision rules are not yet independently runnable.

Follow-up: [the lower movement model](pathfinding-movement-test.md) now implements
`0x4f3990` and connects it through `with_movement_test`. Its own record, validity,
band, and clearance queries remain deeper providers. `0x4f47d0` is now reconstructed as an ordered cell-footprint wrapper; its lower
`0x4f3550` is supplied by the [occupancy model](pathfinding-occupancy-test.md). See [cell findings](pathfinding-cell-test.md).

## Validation

`tests/route-creature-acceptance-test.cpp` enumerates all 128 destination/six-cell
result combinations with an independent Boolean oracle and asserts exact
short-circuit call order and category stores. Other cases cover member fields,
special-state flags, base rejection with bypass enabled, destination failure
before exemptions, axial/zero/seam distances, signed squared overflow, state
rereads, generator integration, and forwarding to the other standard path.

```bash
./tests/test-reconstruction.sh
python3 tests/test-search-support.py
cmake --build working/build/pathfinding --parallel 2
ctest --test-dir working/build/pathfinding --output-on-failure
```

All four C++ tests, ASan/UBSan (fatal UBSan for the new test), and evidence-export
tests passed. Checks use synthetic inputs, not live collision traces. Canonical
XY/positive dimensions are host preconditions; missing providers throw rather
than modeling invalid engine pointers. Lower helper side effects beyond state
and category updates remain outside this model. The installed game's gameplay
has not changed.
