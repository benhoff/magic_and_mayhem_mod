# Original padded-canvas presentation error — 2026-10-24

## Finding and confidence

High confidence within the admitted NoCD build and Wine 11.16 software-provider
cohorts: enabling `DEBUG/UseMallocedBackBuffer` causes an original screen
presentation copy to request a rectangle 96 pixels wider than the primary
surface. The observed operation is Surface2 `BltFast`, not the World alternate
copy. No original game instruction or graphics argument is repaired by this work.

| Video | Setting | Source rectangle | Primary extent | HRESULT |
|---|---|---|---|---|
| 640x480 | disabled | (0,0,640,480) | 640x480 | 0 |
| 640x480 | enabled | (0,0,736,480) | 640x480 | 0x88760096 |
| 800x600 | disabled | (0,0,800,600) | 800x600 | 0 |
| 800x600 | enabled | (0,0,896,600) | 800x600 | 0x88760096 |

All direct-call cohorts observe destination (0,0), flags0x10, the actual source
and destination COM pointers, caller stack, original wrapper dimensions and
source pitch. Enabled cases retain the child-owned original modal text
`Rectangle provided was invalid.` before any World dispatch. Disabled controls
continue through Quick Battle and at least12 actual dispatches. This establishes
the failing original request/result on this host; legacy physical-driver behavior
and other configurations remain unvalidated.

## Original chain

The immutable executable SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

1. Initializer4e40d0 reads config6e1f78 at4e415a and sets x/y padding96/0.
   Offscreen construction adds padding once at4e41be/4e41da and creates wrapper
   6a49e0 through58ad90. Primary wrapper6a4fe0 retains video extent.
2. Screen presentation at5597ea..5597f8 calls58bf40 with source ECX6a49e0,
   destination argument6a4fe0 and x/y0; physical return address5597fd is observed.
3. Original58bf40 builds the source rectangle using source wrapper+10/+14.
   Captured global6f68d8 is1, selecting Surface2 `BltFast` slot7 at58c05a.
   Its real COM arguments are `(destination,0,0,source,&sourceRect,0x10)`.
4. Hardware execution watches bracket the actual unchanged indirect call at
   58c05a and its return58c05d. Return EAX is0 for disabled controls and
   88760096 for enabled cohorts. The latter asks a736/896-wide source to fit
   into a640/800-wide destination at x0. Wrapper metadata and the direct API
   request/result agree; no additional GetSurfaceDesc/Lock calls are injected.
5. The shared error branch at58c067..58c06d passes the HRESULT to484c00 with
   reporter return58c072. A separate reporter-watch execution verifies that
   stack and the upstream5597fd return at both resolutions. The original modal
   is read but never dismissed. The static non-lost error branch returns to
   the copy loop after reporting; bounded termination avoids asserting Quit.

## Reproducible observation

`runtime/diagnostics/alternate_canvas_debug.c` retains the original two DWORD
publication watches and dispatch watch on every reported created thread.
DR3 can observe shutdown (default),484c00 (`--trace-errors`) or the first selected
58bf40 copy entry/call/return (`--trace-blt`). In copy mode only the relevant
thread's execution watch advances through the real call; unexpected overlapping
selected calls fail instead of combining records. Hardware debug registers and
the resume flag are the only child context changes. There are no software traps,
WriteProcessMemory, allocator substitutions, pointer publications or pixel reads.

Pinned original bytes are checked at process creation for dispatch,reporter,
copy entry and both API call alternatives. The existing main/worker transient
publication positive control remains mandatory. Diagnostic output is a separate
3072-byte version2 record:16DWORD control/register header,512DWORD stack,
32DWORD source/destination wrapper snapshots each,six API arguments,RECT fields,
explicit successful-read flags and original fast-route value at DWORD650.
A completed return/result marker is mandatory; a partial entry is not accepted.
BltFast has no destination RECT,so its destination-RECT read flag is zero.

Run the existing live runner with a new output/report path and either
`--trace-blt --enabled-first --frames 12` or
`--trace-errors --enabled-first --frames 12`,alongside its required source-game,
marked idle prefix and prefix-owner arguments. The installation supplies assets
only; staging replaces its executable with the pinned immutable build and the
established explicit menu adapter. Source fingerprints,all4848 installation
files,provider hashes and original manifests are checked before/after.

Final prospective records are [direct call/return](alternate-error-results-20261024/copy-bound02.json)
and [independent reporter](alternate-error-results-20261024/reporter-bound02.json).
Earlier discovery/failure reports are preserved under the same result directory:
compilation failures live01/live02; live03's completed enabled observation plus
rejected disabled termination race; blt01's missing narrow-Blt witness (the actual
route is BltFast); and blt02's complete discovery run before final assertions. Initial copy-final/
reporter-final executions passed; the full audit then rejected a function link
for5597f8 outside the discovered inventory. The current contract retains that
raw caller witness in research and leaves whole-function recovery pending;
both modes were rerun under the corrected prospective accounting contract.
The debugger now ignores already-pending exceptions after bounded termination
rather than requesting context from a terminating thread. This is observer
cleanup,not an original game teardown fix.

The observed5597f8 call is outside the discovered-function inventory. The
existing5595d0 recovered exception covers only eight entry bytes and is not
expanded to claim this caller body. Exact call bytes/stack witnesses are retained
in the execution reports; complete caller-function boundaries remain pending.

## Remaining ownership boundary

This attributes the prior unclassified error; it does not remove the original
failure or establish alternate allocation/publication. Both alternate slots
6e1f68/6e1f6c remain zero with no observed writes in these cohorts. The screen
copy runs before the enabled World alternate route could be observed. Zero
writes here do not establish an absent allocator or global unreachability.
Original alternate allocator,publisher,initial pixels,base/source relationship,
all writers and retirement remain unknown; broader configurations and incoming
indirect paths remain candidates. Any future cropped presentation or allocator
policy must be stated separately and cannot be credited as recovered behavior.
