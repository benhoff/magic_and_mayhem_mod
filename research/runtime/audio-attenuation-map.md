# Positional audio map lookup and ownership

Reviewed 2026-10-04. Selected lookup reconstructed offline; shared-world
ownership established statically. No game execution or runtime replacement.

## Evidence and confidence

`python3 tools/export-audio-map.py` pins the No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`, exports
full assembly and 690 direct global references, and verifies all 2927 immutable
original files before and after. Evidence:
`working/decompiled/audio-map-jgd64h_b/{manifest.json,program.asm,references.json}`.
No executable was modified. Direct references alone do not enumerate indirect
world-field accesses; the ownership findings below also use the full assembly.
Confidence is high for these inspected instructions, not for the field's
physical meaning or agreement with live snapshots.

## Lookup contract

At `0x005717ae..0x005717d5`, nonzero `0x005e1404` enables the read:

```
index = rowOffsets[sourceY] + layerOffsets[uint32(sourceZ) >> 1] + sourceX
byte = sign_extend_int8(activeBytes[index])
```

Row offsets are DWORDs at `0x006cb942`; layer offsets are DWORDs at
`0x006cb8c2`; active byte storage is the pointer at `0x006c5c5c`. These tables
are shared with world/pathfinding consumers. Their entries are added directly
to the byte pointer here: no 12-byte pathfinding-cell multiplication applies.
The lookup uses the supplied source X/Y, not the wrapped listener deltas.
Z uses a logical DWORD shift; adjacent even/odd values select the same layer.
The original read does not validate bounds. It occurs after the strictly
greater-than-range early return but before the final volume cutoff: exactly
at range still reads. Disabled lookup does not touch tables or byte storage.

The signed value enters the existing threshold and attenuation formula in
[positional-audio.md](positional-audio.md). This branch also stores its value
in scratch global `0x006f4358`; the pure model does not reproduce scratch
globals. Other render/entity consumers use the array and enable flag. Calling
it terrain height or acoustic occlusion is unconfirmed; generation and
propagation semantics remain separate work.

## Original ownership

The world at `0x006c5490` owns these fields:

| Offset | Static role |
| --- | --- |
| +0x7cc | Active array borrowed by audio; global `0x006c5c5c` |
| +0x7d0, +0x7d4, +0x7d8, +0x7dc | Four companion work arrays |
| +0x7e0 | Shared byte count |

Constructor instructions `0x004bfbd1..0x004bfbef` zero these pointers and
length. `0x004ecdd0..0x004ecf01` allocates five arrays when active storage is
null, in ascending field-offset order. Length is
`trunc((world.field0c + 1)/2) * world.field10`, in the original integer domain.
This document does not independently establish the dimensions represented by
field10. Even when already allocated, initialization fills +0x7dc with byte
`0x005e18f0`, then copies +0x7dc → +0x7d8 → +0x7d4 → +0x7d0 → +0x7cc,
and calls `0x004f1890`. Allocation failure handling and that generator are
not reconstructed.

`0x004ecbe0..0x004ecc7d` frees these arrays if +0x7cc is nonnull, in order
+0x7cc, +0x7d4, +0x7dc, +0x7d0, +0x7d8, then zeroes all five pointers
and length. Other cleanup in that method is outside this selected contract.
`0x004f1be0` copies +0x7dc to +0x7d8; updates around `0x005670b6`,
`0x00567538`, `0x00567c84` copy +0x7d8 into active +0x7cc. Audio neither
allocates nor frees this mutable world storage.

## Offline implementation and validation

`reconstruction/audio/attenuation_map.*` provides an owned snapshot of bytes
and captured row/layer tables. It preserves arbitrary offsets and padding.
Copying, replacing or clearing snapshots has ordinary vector ownership, with
no game pointer retained. This is an explicit host policy, not a reconstruction
of all five original work arrays, their generator, or cross-thread synchronization.
The adapter supplying a snapshot must ensure coherent dimensions, tables and
bytes; no live capture protocol is implemented here.

`PositionalByteSource` defers access until the recovered distance gate has
passed. Provider input replaces the older optional decoded byte, including
when disabled. Both positional and camera helpers accept the provider.
The native audio library does not depend on this build-specific model.
Invalid coordinates, absent tables, cleared storage and offset overflow reject
explicitly rather than emulate original invalid memory reads or DWORD wrap.

Run `python3 tools/test-audio-map.py`. Evidence:
`working/tests/audio-map/run-bt3na_em/report.json`. All 18 audio/asset CTests
passed. Fixtures cover every signed byte value, padded tables, paired Z layers,
disabled and distance gates, exact-range reads, source coordinates across a
map seam, copied-snapshot lifetime, clear, invalid indices, overflow rejection,
and camera → lookup → attenuation integration. The standalone
`working/build/audio-output/audio-map-sanitize` fixture also passes address,
undefined-behavior and leak sanitizers. No game, game assets or audio device
are required. Live byte agreement, map updates/transitions, floating-point
edge equivalence and audible behavior remain unvalidated.
