# Native owned DC copies

NativeDrawBitBltV1 paints canonical SRCCOPY words between current native RGB565 DC leases. It clips destinations, checks source bounds before writing, handles self overlap in original snapshot order, flushes pending GDI before reading and forwards custom map/clip/ROP/unknown handles. The versioned NativeDrawPlaneV1 service resolves only current CPU storage, rejects active DC leases and retires with its surface; adapters never retain its proof across calls.

**Correction, 2026-10-15:** the former ten production targets call two-argument
USER32 `ClientToScreen` through IAT `0x5c51fc`, not nine-argument GDI `BitBlt`.
The pinned EXE imports no `BitBlt`. Their production hooks are removed and
obsolete enabled staging is rejected. See [route investigation](native-pixel-route-scenarios.md).
Historical standalone service comparisons never established these hook targets.

MnmDraw implements the selected DirectDraw copies/fills. A standalone native
factory/thunk fixture passes512 GDI comparisons, both separate
and overlapping self sources,2,560 atomic paint refusals and locked/unlocked/
DC-busy/retired plane lookups. Its native thunk counts512 admissions and0
original forwards; teardown leaves0 DCs and0 surfaces. There is currently no
reviewed original game BitBlt target. The selftest-only thunk uses a genuine GDI
import for unsupported calls.

The [resource/state extension](native-resource-state.md) additionally admits
RGB555 and proves 1,024 GDI/native copies plus 5,120 refusals. CPU plane lookup
reports the actual 555/565 format and rejects non-WORD storage. Native painting
verifies that the owned bitmap remains selected; replacing it preserves the
lease and refuses native writes until the caller restores it.
