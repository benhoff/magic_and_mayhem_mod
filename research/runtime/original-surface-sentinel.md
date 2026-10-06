# Original cursor sentinel and bitmap-to-surface reload

Confirmed for No-CD build SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the bounded endpoint scope below. Preferred executable
addresses identify this build; they are not verified runtime pointers across launches.

## Evidence and scope

[Capture report](original-surface-sentinel-capture-20261006.json) retains 1,698
unchanged original x86 executions in Unicorn 2.1.4, across 12 bitmap inputs.
[Offline comparison](original-surface-sentinel-cpu-20261006.json) independently
models all 20,060 ordered endpoint events and submitted DIB buffer hashes from
recorded operation inputs. Ten regression tests cover global target identity,
call ordering, return values, truncation, allocation failure and provenance.

Original constructor/destructor, file parser, bitmap drawing helper, surface DC
loader, cursor sentinel and four recovery wrappers execute unchanged. CRT
open/read/close/allocate/free calls, DirectDraw COM calls, error dispatch and
StretchDIBits are explicit scripted endpoints. GetDC always supplies the fixture
HDC, even when its scripted HRESULT fails. This establishes original control
flow under that out-parameter contract, not real GetDC failure behavior. No
original CRT, real GDI rasterization, driver loss or Wine execution is claimed.
Every case uses a fresh private PE mapping, finite instruction/event budgets,
verified unchanged original function-range bytes, and checked SEH/allocation
cleanup. Collector budget exhaustion is a refusal, not an original return.

## Recovered cursor route

| Address/field | Captured contract |
| --- | --- |
| Recovery object +40 | Zero skips reload; callback pointer invokes vtable slot 0 with argument zero; `0xffffffff` selects the cursor sentinel. |
| `0x4a2f90` | Calls `0x58d1a0` with global receiver `0x642000` and fixed path at `0x5e04d0`, `bitmaps\cursors.bmp`; then returns zero regardless of loader result. |
| `0x642008` | Global receiver's surface interface slot; a null interface skips DC drawing after successful file loading. |
| `0x58d1a0` | Returns zero for file-loader failure, one for successful file loading, including a null surface or scripted DC/GDI failures. |
| `0x486950` / `0x486960` | Initialize local bitmap buffers and free the header buffer before the pixel buffer. |
| `0x486ae0` | Original bounded BMP file parser described below. |
| `0x486a80` | Submits header/palette and pixels to StretchDIBits. |

The sentinel always targets the global cursor surface. It does not select the
surface that triggered recovery. In a copy wrapper with both reload fields set
to `-1`, successful source Restore reloads that global resource first; successful
destination Restore reloads the same resource again. Callback and sentinel
routes can coexist. Failed Restore skips key and reload work for that receiver;
key failure does not prevent its reload. Partial fill has no draw retry; full
fill performs one additional draw after the lost-surface recovery branch,
including failed Restore. Copies retry every nonzero draw result. These extend
[the prior scheduling evidence](original-surface-retry.md), whose portable C++
adapter currently represents callback reloads only. Void wrapper EAX values are
retained as raw observations but are not compared as semantic return values.

## Recovered bounded BMP loader

The parser opens `rb`, reads a 14-byte file header, checks `BM`, then reads a
40-byte information header. Each read must return the exact requested count.
It allocates 1,064 bytes for the header plus palette. The captured palette read
size depends on bit depth: 1-bit reads 8 bytes, 4-bit reads 64, 8-bit reads 1,024,
and 24-bit reads zero. It computes pixel allocation/read length as unsigned
`bfSize - bfOffBits`. It reads pixels sequentially after the palette, without
seeking to `bfOffBits`. A synthetic eight-byte offset gap confirms that gap bytes
enter the submitted pixel buffer and the final eight bytes are omitted.

Missing files return failure without close. Bad signatures, short headers,
short palettes and short pixels close the file and fail; the outer loader frees
allocated buffers. First/header allocation failure closes and fails. Second
allocation failure, zero/underflow/oversized pixel lengths, exceptions, other
compression/header/palette configurations and malformed dimensions remain
uncaptured. Bounded endpoints refuse oversized allocations/reads instead of
inventing original behavior.

After successful file loading, a non-null global surface receives GetDC,
StretchDIBits and ReleaseDC in order. The loader ignores all three return
values. StretchDIBits uses destination/source origin zero, header width/height,
SRCCOPY `0x00cc0020`, and usage zero for indexed 1/4/8-bit inputs, one for 24-bit.
The corpus checks exact submitted header, palette and pixel SHA-256 values;
it does not contain independently captured GDI output pixels.

The installed `Bitmaps/cursors.bmp` input is 336,056 bytes, 400 × 280, 24-bit,
SHA-256 `009ea05c7fded93d8a2a10b34ad23ba34013a4a21839fb291d3630c70f6131c2`.
Its file header declares offset 54; the original parser submits all 336,002
remaining bytes. The retained corpus includes these input bytes so comparison
runs entirely offline without installed media or original execution.

## Reproduction

Offline verification needs only Python's standard library:

```sh
python3 tools/check-original-surface-sentinel.py
python3 -B tests/test-original-surface-sentinel.py
```

To produce new original evidence, install
[the pinned capture dependency](../../tests/surface-sentinel-requirements.txt)
in an isolated environment, prepare the pinned working executable and cursor
asset, and choose a new fixture directory/report path:

```sh
python3 tools/capture-original-surface-sentinel.py \
  --fixture-dir working/tests/new-sentinel-fixtures \
  --report working/tests/new-sentinel-capture.json
```

The collector verifies the immutable original manifest before and after,
including failure cleanup, checks the executable and input hashes, and refuses
retained evidence overwrite. It never starts Wine or patches original text.

## Remaining boundary

Native cursor resource routing, native BMP/DC conversion, independently captured
GDI raster pixels, real lost-draw wrapper composition, other reload callback
bodies, CRT/GDI failure out-parameters and live recovery remain pending. A
successful sentinel/loader return does not establish that the triggering surface
has regained valid contents. The native backend must continue to track explicit
pixel validity until an actual reload defines those pixels.
