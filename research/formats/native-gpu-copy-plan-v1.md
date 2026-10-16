# Native GPU copy plan v1

Process-local PE32 export, not a shared channel or host pointer serialization:
`NativeDrawGpuCopyPlanV1(dst, dr, src, sr, flags, plan, bytes)` has seven stdcall
arguments. Object/rectangle/output pointers stay in the calling Wine process.
Flags are 0 or source-key 0x8000; only current owned matching RGB565 objects with
stale GPU authority are admitted. Function HRESULT 0 means a complete output plan;
the plan's own result is the application's Blt HRESULT and may be negative with
an ordered successful prefix. Service errors leave output unchanged.

The scalar output is 6,192 bytes, uint32-aligned: 48 header bytes followed by 256 slots
of 24 bytes. Header DWORDs are version 1, bytes 6192, count 0..256, application HRESULT,
source width/height, target width/height, Boolean physical self identity, Boolean
keyed, RGB565 key (zero if unkeyed), reserved zero. Each piece is source x/y,
width/height, destination x/y. Unused slots are zero. Source and destination
coordinates are normalized in-bounds samples after the one current clip snapshot.

The bridge validates the complete plan before emitting anything. Native stream
COPY records remain unchanged. Aliased sources use one temporary CREATE, one full
opaque source-to-temporary COPY, all keyed/opaque pieces from that temporary, then
DELETE. No scratch is created for a no-op or distinct source. A failed publication
cannot return a successful operation; the session terminates. Capability requires
the new export; absence never silently selects the old unsupported path.
