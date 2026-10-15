# Native owned DC copies

NativeDrawBitBltV1 paints canonical SRCCOPY words between current native RGB565 DC leases. It clips destinations, checks source bounds before writing, handles self overlap in original snapshot order, flushes pending GDI before reading and forwards custom map/clip/ROP/unknown handles. The versioned NativeDrawPlaneV1 service resolves only current CPU storage, rejects active DC leases and retires with its surface; adapters never retain its proof across calls.

MnmDcCopy patches ten pinned BitBlt CALLs in the surface wrapper families, retains original preparation/DirectDraw calls/lifecycle and counts native versus original paint calls. MnmDraw already implements their selected DirectDraw copies/fills. A real native factory/thunk fixture passes512 GDI comparisons, both separate
and overlapping self sources,2,560 atomic paint refusals and locked/unlocked/
DC-busy/retired plane lookups. Its native thunk counts512 admissions and0
original forwards; teardown leaves0 DCs and0 surfaces. Pinned original wrapper
installation and live integration remain separate from these comparisons.
