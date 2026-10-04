# Owned terrain definition catalog

Installed version-4 `Terrain.ttd` files have a 16-byte header: `TTD\0`, DWORD
total byte size, DWORD version 4 and DWORD record count. Payload is exactly
count*356 bytes. A static inventory check confirms this extent for all 17
installed catalogs. Celtic Forest has 1,753 records and 624,084 bytes.

The native `assets/terrain_catalog` service validates signature, version,
declared size and exact payload extent before allocation; count is bounded to
65,536 and input to 16+65,536*356 bytes. It returns independent owned arrays,
retaining all record bytes. File errors retain the AssetFile error. Unsupported
versions, truncation, trailing bytes and inconsistent counts are rejected.
The original file loader is not executed by this milestone.

Build-specific rendering interprets three four-view frame arrays at offsets
0x84, 0xd0 and 0xe0. Those offsets are consumed by the unchanged original terrain
producer and tested against every Celtic Forest record; additional selected geometry fields are described below. Remaining bytes
stay opaque. See [producer evidence](../runtime/terrain-submission.md).

`terrain-catalog-owned-records` checks complete ownership and malformed extents
in normal and ASan/UBSan builds. These tests and installed producer comparisons
are separate from original loader equivalence and live replacement.

Selected map initialization also consumes DWORD +0x94 (classification; the
selected cell initializer tests equality with 0x10) and byte +0xa8 (neighbor
connection predicates used by geometry admission). The catalog still owns and
preserves these bytes without applying engine policy. See
[map initialization and geometry evidence](../runtime/terrain-map-initialization.md)
for field conditions, original execution and remaining boundaries.
