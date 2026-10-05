# Authored lighting snapshot fixture, version 1

This is a native offline test/preview input, not a recovered game file format.
It supplies owned records to the [combined lighting contract](../runtime/terrain-object-lighting-scenes.md).
The local file is selected with `--light-fixture`; installed asset requests still
use the existing read-only asset store. No game memory or executable is needed
by the Qt preview.

Tokens are separated by whitespace. Comments, unknown tokens and trailing data
are refused. Integers must be complete decimal tokens within the ranges below;
booleans are exactly 0 or 1. The preview reads at most one MiB and accepts
1..64 complete snapshots, with at most 256 records in each collection per tick.

The file starts with `MNM_LIGHTING 1`, followed by `relations` and **128 bytes**
written as decimal integers 0..255: the first 8-by-8 table in owner-major order,
then the second. Both relation bytes `[owner][player]` must be nonzero to admit
a different-owner creature; the creature's own player bypasses these tests.
Their broader semantic names remain unconfirmed. The two tables are held fixed
throughout the fixture. All parsed snapshots own their records and relation data.

Each snapshot begins with:

```text
tick player changed creaturesEnabled objectsEnabled viewColumn viewRow viewExtent creatureScan objectScan creatureCapacity objectCapacity
```

| Field | Accepted values and meaning |
| --- | --- |
| player | 0..7; spectator/sentinel paths excluded |
| changed | signed 32-bit; nonzero restarts; exactly 1 can force completion |
| creaturesEnabled, objectsEnabled | 0 or 1, separate updater controls |
| viewColumn, viewRow | 0..127 and in bounds when stepped |
| viewExtent | 0..128 and no greater than the smaller map dimension when stepped |
| creatureScan | 0..260; may exceed owned creature capacity, matching the separate original scan bound |
| objectScan | 0..256 and no greater than objectCapacity |
| creatureCapacity, objectCapacity | 0..256; exact counts of following records |

Next come `creatureCapacity` creature records, followed by `objectCapacity`
object records:

```text
creature active lightEnabled owner column row layer
object active lightIndex column row layer admissionColumn admissionRow
```

The boolean fields are 0/1, owner is 0..7, XY coordinates are 0..127, and layer
is 0..31. Object lightIndex is 0 (no light), or 2..33 selecting kernel extent
`index/2 + 1`; special index 1 is refused. Admission coordinates belong to the
recount predicate; position coordinates belong to visitation/stamping. Active,
admitted sources must also satisfy the recovered field's coordinate/kernel
bounds when stepped. Unsupported snapshots leave the composition unchanged.

A following `tick` starts a new complete snapshot; there is no implicit record
carry, motion or clock advancement. `end` terminates the file. All snapshots are
parsed for syntax and structural bounds. Field-dependent admission is validated
for the selected prefix that actually runs, allowing the file's unused later
snapshots to be replayed separately.

`--light-tick N` selects a zero-based snapshot after applying snapshots 0..N.
Without it, all snapshots run and the final tick is rendered. Fixture mode
requires `--terrain-lighting`, world mode, palette shading and global lighting
configuration; it excludes explicit `--light-source` requests and uniform
`--light`. The fixture's admission view is independent of the render camera:
this is an explicit test input, not a recovered region/view lifecycle.

The [small example](../../tests/fixtures/terrain-lighting.lighting) stamps one
creature and one static source, then supplies two empty snapshots. It fits maps
at least 17-by-17 with a valid layer zero. Output JSON records fixture SHA-256,
selected tick, parsed tick count, consumed changed flag and both cycle states,
as well as all five buffer SHA-256 values for each applied tick. Tiles sample
only the final published field; pending prior/target bytes never leak into
presentation. The preview owns no original pointers or Wine state.
