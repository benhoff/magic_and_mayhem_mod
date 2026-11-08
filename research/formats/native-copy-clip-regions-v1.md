# Optional native provider clip-region observation v1

This is an intentional native observation contract, not a recovered original
game file format. Its producer is `runtime/native-draw/clip_region_census.h`;
the portable layout/recorder is
`protocols/include/mnm/native_copy_clip_regions_v1.h`. It is disabled unless
`MNM_NATIVE_COPY_CLIP_REGIONS=1`. The stopped-file reader is
`tools/native_copy_clip_regions.py`.

The DLL creates `native-copy-clip-regions-v1.bin` exclusively at startup. Each
callback copies the immutable RGNDATA snapshot already obtained for that
operation. It performs no additional GDI query, pixel access, clipping mutation
or file I/O. Startup/retirement preserve LastError. Mapped integer stores are
bounded; contention, capacity, malformed input and owner-thread changes retain
failure counters rather than changing the operation. DLL retirement closes,
flushes and unmaps the observation. Missing/unclosed/failed output is refused.

All integers are little-endian 32-bit words. Numeric object/window identities
are launch-local opaque tokens, never host pointers to dereference.

| Header words | Meaning |
|---|---|
| 0–1 | Eight bytes `MNMCLP01` |
| 2–5 | Version 1, header 128 bytes, row 4384 bytes, capacity 8192 |
| 6–11 | Published, missed, completed, error bits, closed, owner thread |
| 12–31 | Zero |

The fixed file contains the header and 8192 rows, including a zero unused tail.
Each row has 1096 words. Published equals completed; missed/error must be zero
and closed must be one for a usable census. Zero rows require owner zero.

| Row words | Meaning |
|---|---|
| 0–2 | One-based ordinal, phase, thread |
| 3–8 | Destination/source aliases, alias[0] bases, incarnation identities |
| 9–15 | Attached clipper, HWND, references, flags, result, region count, RGNDATA byte count |
| 16–23 | Signed destination/source rectangles, left/top/right/bottom |
| 24–29 | Destination/source width, height, bits |
| 30–34 | Keyed, key WORD, plan pieces, fill value, fill observation |
| 35–63 | Zero |
| 64–71 | Actual RGNDATA header: 32, 1, count, count×16, signed bounds |
| 72 onward | Up to 256 signed rectangles in actual order, followed by zeros |

Phases 1/2/3 are CPU-copy/GPU-copy/fill plan inputs. Phases 4/5/6 are
CPU-copy/fill/direct-GPU actual operation inputs/results. Actual phases use
`0xffffffff` for plan count. Fill observation is zero (not read), one (a
constant consumed value), or three (varied consumed values). The optional fill
value is observed from the existing local value inside the existing loop;
there is no extra read of caller effects memory.

The independent scalar oracle intersects the requested destination with each
recorded region and destination bounds in order, translates to source
coordinates, and checks source bounds. A later invalid source region retains
prior pieces and returns `0x88760096`. This oracle consumes input region data,
not the native plan's emitted rectangles. Supplied controls cover multiple,
empty and 256 regions, later source failure, alias/key and fill paths; they
establish the observation mechanics separately from natural caller reachability.

The observation alone proves neither actual output nor an original-driver ABI.
Operation acceptance additionally needs exact caller order, canonical/live
storage identity, returned result and independently checked complete output
planes with untouched exterior. A windowed private desktop is an explicit
stimulus; it supplies no exclusive/fullscreen or physical-driver claim.

The first live census has 1084 published owner-thread rows and 274 refusals
from other threads (error bit 4). The strict whole-census reader correctly
refuses it. `native_copy_clip_regions_scoped.decode_owner` reads the original
bytes without changing that failed header. It admits only an exact owner-thread
scope when the sole error is other-thread refusal, every published row has the
actual World thread, and the closed application census independently balances
all rejected copy-plan/actual callbacks. Capacity, malformed, contention,
closure, count, format and tail errors still refuse this scope. Other-thread
operations receive no clip-input or pixel credit.

Observer words 5/6 name alias[0] bases, despite the initial Python decoder's
`*_canonical` field labels. In the pinned PE32 provider, `NsObject` occupies
three DWORDs and `ns_query(IUnknown)` returns alias[1]. The join freshly checks
each actual alias against its observed base, then checks base+12 against the
intercepted canonical IUnknown. No address is assumed stable across launches.
Provider incarnation and bridge generation/handle are distinct domains: bridge
storage can be reconstructed without destroying the provider object. Only
operation-bound live storage joins are claimed.
