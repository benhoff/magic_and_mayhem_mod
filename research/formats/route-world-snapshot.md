# Frozen route world: MNMWLD01

This is a toolkit-designed replay format, not an original game file format.
Writer: `tools/route-world-snapshot.py`, called by the existing WineDbg controller
with `--world-snapshot`. Reader: `reconstruction/pathfinding/route_world.cpp`.
All integers are little-endian DWORDs; signed fields keep their original bits.

The 92-byte header contains:

| Offset | Value |
|---|---|
| 0 | Eight ASCII bytes `MNMWLD01` |
| 8 | Creature token |
| 12,16,20 | X/Y dimensions and global layer count |
| 24 | Plane stride in cells |
| 28 | Global map boundary (`+0x7f0`) |
| 32 | Search budget before call |
| 36 | Unknown search argument |
| 40,44,48 | Canonical target XYZ |
| 52 | Unaligned global default scalar (`0x6c007d`) |
| 56 | Raw float bits of mutable scalar global (`0x5e15f0`) |
| 60 | Original cell-array pointer, retained solely as node-token base |
| 64..88 | Seven block lengths, DWORDs in the order below |

Seven consecutive blocks follow without padding:

1. Y row offsets, signed cell offsets (`global map +0x64b2`, Y * 4 bytes).
2. Z layer offsets (`+0x6432`, Z * 4 bytes).
3. Twelve-byte cells from global map pointer `+0x4c`, through the maximum
   reachable row + layer + X index. This preserves gaps in nonstandard tables.
4. Terrain records, stride `0x164`, from global `0x65660c` through the highest
   unsigned first-WORD cell index inclusive. Index zero is retained.
5. Creature prefix, `0xd07` bytes.
6. Creature `+0xac` record prefix, `0x198` bytes, for acceptance and scalar data.
7. Indexed generator type record, `0x5c9` bytes, from
   image-relative `0x6a5f80 + signed(creature[+0xa8])*0x5c9`.

The indexed generator record and the creature's record pointer are separate
inputs even if they happen to agree. Costs use generator DWORD `+0x589` and
penalty DWORD `+0x5ad`; scalar/acceptance use the captured `+0xac` record.
Pointers embedded in raw blocks are never dereferenced by the host reader.
Node tokens use original cell base plus cell index * 12; the inverse coordinate
map rejects aliasing coordinate tables. Callbacks retain shared ownership of
all input storage.

The companion same-stem `.json` stores SHA-256 of the entire binary, format,
executable SHA-256, source image base, block lengths, pointer provenance,
dimensions and scope. `tools/replay-route-world.py` checks hash/size and header
lengths before building or running the host reader. The C++ reader independently
checks exact block sizes, record strides, table bounds, terrain indices, token
range and total file length. Direct C++ invocation omits the companion hash
check; use the Python launcher for captured evidence.

Capture bounds: positive XY <=1024, layer count <=32 (the layer table ends at
row table), type index 0..1024, maximum 64 MiB file, maximum 32 MiB cell span.
These are toolkit guards, not discovered gameplay limits. Every memory command
is bounded to 256 DWORDs plus at most three trailing bytes; the trace deadline
is checked before every chunk to preserve cleanup on long captures. Static globals use
loaded module base + RVA; dynamic pointers come from that stopped launch.

Wrapper world mode captures fresh creature searches. The new `--search-sequence`
mode captures fresh and continuation inputs at common search `0x54b800` entry,
including direct callers. The companion metadata adds actual `context` token and
`flag_before`. The binary header and seven block layouts are unchanged.
The format does not contain heap continuation queues or arbitrary mode-nonzero
descriptors. Continuation replay reconstructs state from preceding captured calls;
an initial zero-flag call alone cannot supply that state. Capturing a world pauses the game longer
than the old wrapper-only trace. There is no executable patch or live hook.

## Capture and replay

```bash
./tools/trace-route-experiment.py --world-snapshot --calls 1 --seconds 180
./tools/replay-route-world.py working/experiments/route-trace/run-REPLACE/world-0001.bin
```

Load a map/save and issue orders after the trace prints that it is armed. The
capture writes `world-0001.bin/.json` and names it inside the completed
`calls.jsonl` entry. Replay writes `world-0001.replay.json` with path coordinates,
per-node category/scalar/priority, expansion count, remaining budget and output
snapshot bytes. If a matching completed call exists, it compares all 524 route
bytes, budget and flag, returning failure for any difference. Without a reference,
status is `no_reference`, never a claimed match. This comparison is not timing data.

Implementation tests use synthetic frozen memory, including a relocated image,
separate type-pointer sources, ground support, all movement decisions, real
reconstructed costs, full search and malformed captures. These verify integration
and protocol behavior; they do not establish end-to-end live agreement.

## Sequential capture companion

`--search-sequence` writes `search-calls.jsonl` instead of wrapper `calls.jsonl`.
Each completed record includes entry `sequence`, `completion`, thread, entry ESP,
return address, loaded image base, search inputs, world filename, context prefix
before/after (hex, each `0x269` bytes), result snapshot hex, budget after and flag
after. The actual receiver supplies flag/snapshot addresses. Replay sorts by entry
ordinal and checks snapshot metadata/header against call arguments. All records
must belong to one loaded image; context tokens have launch-local meaning.

```bash
./tools/trace-route-experiment.py --search-sequence --calls 20 --seconds 300
./tools/replay-route-world.py --sequence working/experiments/route-trace/run-REPLACE
```

The replay creates `sequence-replay.json`, comparing route bytes, budget, flag and
known best-node fields for every synchronized call. Zero-flag calls before a
captured fresh start are inconclusive. Mismatches taint subsequent continuations
of that context until a reset. See
[common search evidence](../runtime/pathfinding-search-sequence.md).
