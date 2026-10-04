# Native NOD navigation input

Reviewed 2026-10-04. Owned version-1 node records and JSON inspection, offline
only. Section graph assembly, connection interpretation and native pathfinding
integration are separate milestones.

## Confirmed layout

All 683 installed `.nod` files total 12,746,278 bytes and 25,769 nodes.
Each node stores 18 connection slots: 463,842 slots in total, 196,140 with a
nonzero first word. Complete independent decoding and byte reconstruction,
plus the [selected original reader and lookup](../runtime/nod-node-loading.md),
provide high-confidence layout evidence.

| File offset | Bytes | Representation |
| ---: | ---: | --- |
| 0 | 4 | `NOD\0` magic |
| 4 | 4 | Complete file size |
| 8 | 4 | Version, one |
| 12 | 4 | Node count |
| 16 | count × 494 | Packed node records |
| After records | 8 | Two opaque trailer DWORDs |

The records start at **16**, despite the total-size formula
`24 + nodeCount × 494`. Treating this as a 24-byte header misaligns every node.
The original reads 16 bytes, then 494 bytes per node, and its selected reader
leaves the final eight bytes unread. The native API preserves those bytes.
Every installed trailer contains `1000` and the node count, but their purpose
is unresolved; the native parser does not enforce those values.

Each 494-byte record is:

| Record offset | Bytes | Representation |
| ---: | ---: | --- |
| 0 | 4 | State word; all installed nodes have one |
| 4 | 4 | Opaque word; not copied by the selected reader |
| 8, 12, 16 | 4 each | Signed x/y/z positions |
| 20 | 18 × 25 | Packed connection slots |
| 470 | 24 | Six preserved tail DWORDs |

Each connection contains a DWORD `value`, signed DWORD `target`, then 17 raw
metadata bytes. The original copies all 25 bytes into a 28-byte runtime slot.
Its lookup uses `value != 0`, compares `target`, and returns `value`; whether
that value is a cost, mask or another quantity is not established here.
The loader preserves it without inferring a label or interpreting metadata.
The reader rebases nonzero-slot targets by the current section node base,
confirming their local ordinal role. Every installed nonzero target lies within
its file's node count. Native input preserves target values, including out-of-
range synthetic ones: it never dereferences them or constructs a graph.

The reader also copies record DWORD +478 to runtime +524. Remaining tail fields
are retained without assigned meanings. All integers are little-endian;
packed, unaligned fields are decoded explicitly instead of casting disk bytes
to host structs.

## Native API and policies

`assets/nod.hpp` exposes `decodeNod`, `loadNod`, `NodAsset` and `NodLimits`.
Node positions, all slots, metadata, tail words and trailer are owned. Native
services depend on the read-only asset layer, not Qt widgets or build-specific
reconstruction. The JSON inspector alone uses Qt.

Validation requires exact magic, version one, declared size and the complete
record/trailer extent. The selected original reader checks actual size against
count-derived size but does not test magic, version or the declared-size word;
these are explicit native validation policies. Empty-node input is covered by
synthetic tests, not the installed corpus. Unknown words, inactive slots and
noncanonical trailers are preserved rather than normalized.

Default limits are 16 MiB input, 32,768 nodes and 32 MiB decoded node storage.
The decoded budget uses host `sizeof(NodNode)`, including its alignment padding.
Counts and extents are checked before allocating or reading any records.
Errors include source byte offsets; file errors retain structured backend
information. No section rotation/translation/wrapping, graph target validation,
connection remapping, search, runtime pointers or live replacement are provided.

## Reproduction and validation

```bash
cmake -S assets -B working/build/nod -DBUILD_TESTING=ON
cmake --build working/build/nod --parallel 4
ctest --test-dir working/build/nod --output-on-failure
python3 tests/test-nod-loader.py working/build/nod/mnm-nod-inspect \
    --installation working/game-clean \
    --report working/tests/nod-loader/comparison.json
```

`mnm-nod-inspect ROOT PATH.nod` reports every field, including all metadata hex,
in JSON after closing the file. The independent Python comparison unpacks each
record, compares all fields and reconstructs every input byte. Its installed
runner verifies the original manifest before/after and rechecks input hashes.
Four synthetic cases cover zero/one/two/eighteen nodes, signed extremes, inactive
and noncanonical targets, all metadata/tail bytes and noncanonical trailers.
C++ tests cover ownership, exact allocation boundaries, malformed headers,
all truncated prefixes with repaired size words, missing trailer bytes and
trailing data. All 31 asset CTests and both NOD ASan/UBSan tests pass, with leak
detection disabled. [Retained evidence](nod-native-loading.json) records source
hashes, static evidence and the complete comparison's canonical digest.

Confidence is high for stored bytes and selected static consumer behavior.
No original NOD reader execution or whole-graph differential validation was
performed; this milestone supplies input for future graph reconstruction.
