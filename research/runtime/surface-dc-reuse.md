# Default DC palette context and repeated Surface2 leases

## Confirmed execution

The [capture](surface-dc-reuse-capture-20261006.json) runs real Surface2
GetDC/ReleaseDC on 8×6 offscreen system-memory surfaces under normal cooperative
mode in one reserved Wine11.16 session. It records192 cases and576 leases:
six retained original loader submissions, indexed8 surfaces with absent/gray
bindings and seven mutation schedules, plus RGB565/RGB32 controls. Each case
acquires/releases three DCs; the first may select a rectangle, the second does
not select any region, and the third explicitly clears the region.
Original helper0x486a80 identifies the previously captured input boundary only;
no game instructions execute. Immutable manifests pass before and after.
A preliminary attempt failed compilation on unused probe arrays, completed
manifest cleanup and produced no retained evidence. The corrected probe passes
strict compilation and the successful run preserves independent output bytes.

Before any case rasterization, a separate unbound indexed surface supplies a
256-entry GetDIBColorTable read. This is recorded environment context, not an
inferred portable Windows default table or a painted expected image. The input
context remains explicit in offline/native replay. Each case then independently
records its before/draw DC tables, acquired clip box, normalized DC identity,
operation results, native Lock words and full DC GetPixel RGB after drawing.
Source/driver/GDI/probe/corpus hashes pin the execution. Handle equality is only
within-run evidence; absolute handles are not retained or reused as host pointers.

## Palette and region behavior

All three GetDC calls succeed in each case. The second/third DC handles differ
from the first within each run, and each acquired clip box is the entire target.
No application clip survives ReleaseDC. Clearing the third DC's region succeeds.
The first rectangle constrains its writes; subsequent unclipped leases fill the
complete cropped source region. Native bytes written earlier remain intact
until overwritten; context reset does not clear surface pixels.

For indexed surfaces, acquire uses the currently bound palette, or the separately
recorded default context when unbound. Detaching an attached palette restores
default colors on the next acquire. Updating an unbound palette object does not
change a surface's DC colors. Updating bound entries64..95 between leases appears
in the next DC table. Rebinding or updating bound palette entries during a lease
leaves that lease's table unchanged and appears in the next acquire.

SetDIBColorTable changes32 entries in the active DC and affects that lease's
conversion and GetPixel colors. It does not modify the surface palette binding:
the next acquire rebuilds the table from the binding or default context. The
probe's table mutation uses supplied deterministic RGB inputs, not native output.
RGB565/RGB32 controls have no indexed table and also start each lease unclipped.
Every StretchDIBits returns the source height, GdiFlush succeeds, and native
readback after release succeeds. These are bounded Wine Surface2 observations,
not universal Windows driver guarantees or proof of primary/video-memory behavior.

## Native implementation

`renderer/bitmap_dc.hpp` adds an owned `BitmapDcState` raster adapter. It accepts
an explicit256-entry default palette context and a separately owned optional
binding. Acquire snapshots the binding/default and resets application regions.
Binding updates remain separate from the current snapshot. Direct DC color edits
apply only to the acquired snapshot; release ends its use. The adapter delegates
bounded region writes to existing GlBlitter DIB conversion and installs snapshot
colors on an associated renderer-owned raster target for DC RGB presentation.

The target represents DC raster colors, not a live surface's shared COM palette
identity or post-release hardware display. No HDC, game address, Wine dependency,
global surface lock, busy gate or wire operation enters this API. Non-indexed
palette operations, missing default context for an unbound indexed acquire,
repeated acquire/release misuse, invalid palette ranges and invalid/excess regions
refuse explicitly. Contexts are noncopyable. Existing32-region/in-target geometry
bounds are native policy, not a claim about all GDI region admission.

- [Offline comparison](surface-dc-reuse-offline-20261006.json):192 cases/576leases,
  27,648 native words and27,648 DC RGB pixels,258,048 before/draw palette entries.
  228 indexed leases use the supplied default context. Nine mutation tests pass,
  including wrong default input, prematurely applied binding, persistent DC edits,
  persistent clips, detach semantics, altered output and incomplete case matrices.
- [Native comparison](native-dc-reuse-20261006.json):192cases/576leases compare
  55,296 independent native/DC pixels and258,048 palette entries.1,928 refusals
  cover state/range/region/lifetime guards and unknown-content policy. Empty
  regions after invalidation do not upload pixels or permit read/presentation;
  reacquire resets them and a full reload reestablishes known bytes. No surfaces
  remain allocated. Existing renderer implementation/test files are unchanged.

The previous [indexed/DC findings](surface-dc-palette.md) retain their exact
historical source hashes. Their60 absent-palette conversions remain excluded
from that earlier comparison; this new capture validates default-context
conversion with explicitly recorded environment inputs rather than changing an
old evidence claim.

## Reproduction and remaining limits

```sh
python3 tools/check-surface-dc-reuse.py
python3 -B tests/test-surface-dc-reuse.py
xvfb-run -a python3 tools/check-native-dc-reuse.py \
  --report working/tests/new-native-dc-reuse.json
```

Offline checks need Python and retained fixtures, with no Wine or original
installation. Native checks use the existing renderer library; specify `--build`
for another build directory. Recapture with new report/corpus paths using
`tools/capture-surface-dc-reuse.py` under Xvfb; it reserves a single Wine session,
uses a disposable prefix, preserves evidence and verifies immutable inputs.

Default context across other environments, logical/flag-bearing/shared palettes,
other GDI selected objects/transforms/regions, cross-surface leases, primary/video
surfaces, native global COM/lock admission, original wrapper/real lost-draw/Restore
composition, Windows hardware and live/wire replacement remain pending. Native
exceptions are intentional API guards, not the measured COM HRESULT contract.
The runtime CPU frame adapter's earlier RGB565 floor policy remains a separate
bridge-color boundary.
