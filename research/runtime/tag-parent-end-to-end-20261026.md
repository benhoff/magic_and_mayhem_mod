# TAG sprite-table parent: installed native application

The `AS.tag` parent now has an end-to-end native consumer: installed read-only
TAG and SPR acquisition, exact companion admission, name/occurrence selection,
timed playback, OpenGL drawing and Qt window presentation. This completes the
registered native sprite-table scope, including its existing permissive raw
reader. It does not establish an original-game TAG consumer. That distinction
is necessary: the original executable's limited literal search was negative,
and an editor-only role remains a hypothesis, not a recovered fact.

## Application

Build `apps/sprite-player` with CMake and run:

```sh
mnm-sprite-player /path/to/installed/game Realms/Celtic/Forest/Terrain.tag
```

The optional TAG argument opens immediately; otherwise use Browse/Open. Paths
remain inside the selected read-only installation through `AssetStore`. Names
are listed in first-appearance order, with exact eight-byte hexadecimal identity
beside the bounded display prefix. Distinct padding after NUL is preserved.
Each name's occurrences become an explicit clip with a native cadence of100ms.
Loop, Once, Ping-pong, Reverse, Pause and the frame slider operate on that clip.
No ANI timing is inferred from TAG, which contains no timing fields.

For installed companion selection, an existing same-stem SPR wins; only a
not-found result allows directory-local `Terrain.spr`. Corrupt, ambiguous,
unreadable or disagreeing existing companions refuse rather than falling through.
The entire ordered TAG must match the SPR's raw names and preceding same-name
counts. The raw parser continues to accept arbitrary whole records independently;
the strict companion rule belongs to this consumer.

Open constructs all source/clip state before publishing it. Reload reopens and
validates both files, preserves the selected explicit clip and full temporal
checkpoint, and publishes only after all admission succeeds. Missing occurrences
refuse. Old plans retain immutable decoded sources after files close, fail or
are removed. No installed file is written. Input/resource bounds are inherited
from the TAG/SPR loaders, strict4096-frame/16Mi-pixel table and bounded timeline.

`SpritePlayer` is independent of QWidget. `SpriteWindow` translates semantic
controls into service operations, samples monotonic host elapsed milliseconds,
and presents owned images. Native OpenGL draws clipped RGB565 and masks into a
320x240 offscreen surface; readback supplies the Qt preview. Each draw retires
its surfaces before returning. This synchronous viewer makes no resident-cache,
throughput or physical-hardware claim. Physical GL failure can retain a prefix
in its private surface; the visible image remains the last completed readback.

## Validation

`tools/test-tag-parent.py` declares parent/scenario fingerprints and source/input
hashes before execution, then builds fresh Release and ASan/UBSan applications.
Compiler dependency files must be a subset of the declared source closure.
Reports are immutable SQLite payloads; failed attempts retain source variants.

The independent Python oracle reads every installed TAG record with `struct`,
locates SPR frame names through the stored offset table and verifies exact
occurrence order. The native application must match all85 tables and130,891
records. Every table also selects first/middle/last name groups, renders their
last occurrences, compares GPU output with a separate decoded-plane raster,
and reloads the paused selection. Two actual installed-file Qt window journeys
cover same-stem and directory-terrain companion selection, select/seek a named
clip, compare every presented pixel and retain that image across paused reload.
This is installed-data agreement, not an
original executable comparison. Inputs and immutable originals are checked
before and after execution, including failure.

Controlled byte fixtures distinguish raw names after NUL, interleaved
occurrences and signed origins. The actual Qt window receives control actions
and must display the expected pixel, retain it across malformed reload, recover
on valid reload and complete a one-shot through its real timer. Every truncated
TAG prefix refuses against the complete companion. Same-stem corruption cannot
hide behind a valid terrain fallback. Old plans survive file removal and reload.
Per-render surface counts must balance. Existing raw TAG, independent installed
parser, sprite-session and sprite-timeline regressions run in both build modes.
External uninstrumented Qt/driver leak scanning is disabled; ASan/UBSan remain
enabled and GPU surface ownership is checked explicitly.

Original allocation/file failures, original/editor TAG use, original ANI timing,
world/entity construction and live replacement remain unclaimed. Parent
implementation is `scoped`, integration is `preview`, and comparison/replacement
remain `none`, independently. Historical N27/N29 results retain their earlier
limited claims and source hashes.
