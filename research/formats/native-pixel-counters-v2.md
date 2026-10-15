# Native pixel producer counters, version 2

Launch-local `native-pixel-producers.bin` is a diagnostic mapping written by the
native drawing facade. All fields are little-endian unsigned 32-bit integers;
there are no host pointers. Version 2 is exactly 2,320 bytes.

| Byte offset | Value |
| --- | --- |
| 0 | Magic `0x50504e4d` |
| 4 | Version `2` |
| 8 | Byte extent `2320` |
| 12 | Family count `6` |
| 16 | First route's installed/native/forwarded counters |

Each family reserves 32 route slots. Each slot contains three DWORD counters,
in installed/native/forwarded order. Its DWORD index is
`4 + (family * 32 + route) * 3`. Family IDs are panel=0, pixel=1, JPEG=2,
minimap=3, colour rectangle=4 and raster=5. The canonical ordered route names
are defined in [native_pixel_counters.py](../../tools/native_pixel_counters.py).
There are 43 active routes. Unused slots must remain zero, and each active
route must be installed exactly once. Producer event increments are atomic;
concurrent snapshots are approximate. Final admission reads after producer exit.

Version 1 included a seventh family with ten alleged DC-copy sites. Those
sites actually call two-argument ClientToScreen. Its original files/reports stay
historical; current live admission rejects the old version and byte extent.
Removing invalid labels provides no execution credit.

Required-route admission demands positive native calls for every named route,
zero forwards across every active route, and no terminal history refusal file.
Installation alone and a forwarded call cannot satisfy a requirement. Counts
establish bounded execution, not output equivalence or complete replacement.
See [current evidence and limits](../runtime/native-pixel-route-scenarios.md).
