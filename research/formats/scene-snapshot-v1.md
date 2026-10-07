# Bounded scene snapshot wire format, version 1

`protocols/include/mnm/scene_snapshot_v1.h` specifies `MNMSCNE1`, an intentional
native observation protocol. This is not an original disk format. Little-endian
fixed-width fields and owned frame bytes cross the process boundary; original
host pointers, owner addresses and C++ object layouts do not.

The 64-byte header has magic at 0; DWORDs at offsets 8 version (1), 12 header
size (64), 16 exact total bytes, 20 nonzero sequence, 24 record count, 28 blob
count, 32 observed view (0..3), 36 original consumer mode flag (0/1), 40 pinned
build tag (0x40209ca7), 44 reserved zero, 48 record offset (64), 52 blob offset
(64 + record count * 32), 56 viewport width, 60 viewport height. Viewport zero
uses the diagnostic host fallback 640x512; dimensions above 2048 are refused.
The full executable SHA-256 is checked by preparation, not inferred from the tag.

Each 32-byte record contains DWORD token at 0, signed depth at 4, signed anchors
at 8/12, signed original draw kind at 16, packed original parameters at 20,
packed signed WORD depth sentinels at 24, and flags at 28. Flags are exactly
0 (normal), 1 (hidden kind -2) or 2 (no readable frame). Hidden/no-frame tokens
are zero. Normal tokens refer to the snapshot-local 1-based blob list. Tokens
are recreated for each snapshot and do not imply stable entity identity.

Each blob has token, byte length, indexed-storage flag (0/1), reserved zero,
then that many version-4 encoded SPR frame bytes. Tokens are contiguous ascending
1..N. The original runtime palette pointer at frame offset 28 is normalized to
zero; storage distinction is retained separately. Width, height, origins,
row offsets, opaque/transparent runs and encoded pixels remain byte-exact.
The host identity is SHA-256 of the one-byte storage flag followed by these
normalized bytes. Embedded native palettes come from a separately pinned asset;
matching encoded bytes alone does not establish original lighting equivalence.
Aliases with different native opaque pixels or origins are refused.

Limits: 8 MiB total, 12,320 records, 4,096 blobs, 40..1,048,576 bytes per blob.
Exact offsets, record flags, references, lengths, reserved fields, normalization,
version and trailing bytes are checked before rendering. Files are created once
with exclusive writer sharing; consumers require the full declared length.
Write failures can leave incomplete files, which readers refuse. This is bounded
immutable capture, not a shared-memory live channel or an atomic multi-file
publication guarantee.

Evidence/confidence: synthetic serialization, every-byte truncation, malformed
flags/counts/references and PE32 duplicate-frame/hidden records are automated.
Live No-CD battle captures confirm readable frame identities and ordered queues
at the selected consumer boundary. Semantic entity ownership, tick timestamps,
other builds and concurrent producer races remain unvalidated. See
[the runtime record](../runtime/native-scene-snapshot.md).
