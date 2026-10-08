# Exact live history kind refusals

Live startup history now enables the existing bounded raw startup queue journal.
After the original producer stops, the launcher correlates the failed input with
its exact raw queue and writes `native-world-refusal.json`, naming every
unsupported kind and draw row. The raw journal remains diagnostic output only.
Missing or inconsistent artifacts refuse attribution; native admission stays
strict.

The earlier user session `run-1fjrk507` published twelve queues and failed source
queue thirteen with reason 8, after presenting one exact 480,000-pixel frame. It
has no raw journal, so its kind cannot be recovered from the retained channel.
Its saved Quick Battle settings select regionIndex 1 and magicItems 15; the
automated fixture selects regionIndex 2 and magicItems 0. This is a confirmed
scenario difference, not proof of which setting causes the refusal.


## Reproduction and source limits

Run `./tools/run-native-world.py --startup-history --verify` and use the original
menus/input normally. A reason-8 exit now prints the source queue, unsupported
kinds, first zero-based row and coordinates, and retains a separate
`native-world-refusal.json`. The producer is stopped before attribution; neither
the raw rows nor original destination pixels enter native rendering. The
existing journal writes at most sixteen 443,584-byte files on the original
thread; this optional diagnostic IO is bounded but has no hard latency promise.
The channel format and kind admission are unchanged.

`tools/capture-scene-game.py --world-live history-normal --live-frames 16 --map 1`
also enables the journal. Finite diagnostics can include original spell
selection using `--magic-items 15`: the existing V3 menu bridge accepts its
initial assignments. The default remains the established zero-item diagnostic
fixture. This is isolated scenario setup, not a gameplay balance change.

The source-stable map-1 normal run `run-chc5oje2` presents all sixteen queues
with no capture refusal and no framebuffer readback. The finite map-1/item-15
run `run-19ynq2s1` also captures sixteen complete queues without kind refusal.
The earlier map-2 finite capture `run-3w54zr31` has the same negative result.
These observations establish only that those automated paths do not reproduce
the manual failure; changing the map/items alone has not identified its kind.

## Static dispatch evidence

`tools/inspect-world-dispatch.py` verifies the immutable manifest before/after,
checks the original No-CD executable SHA-256 and consumer/table signatures, and
exports all 35 numeric targets of each existing table. The retained
`native-world-dispatch-targets-20261008.json` keeps the exact disassembly.
Kind 33 takes a terrain branch before the first table. Selected first-table
branches show different recovery costs: kinds 15..30 call wave `0x5806f0` with
amplitudes 1..16; kinds 12..14 call `0x581200` with literal colour parameters;
kind 8 invokes a virtual method, and kinds 32/34 use indirect slot `0x6903dc`.
A target or familiar primitive does not establish full mode/branch support.
The two other tables, out-of-range defaults, auxiliary-plane side effects and
per-kind full raster equivalence remain explicitly unresolved. No new kind is
admitted from this static inventory.

The stopped synthetic journal tests cover a producer twelve frames ahead with
one presented canvas, partial/complete journals, signed kinds, startup wave
admission, ENDED failure retention and strict missing/gapped/build/active/partial
input refusals. They exercise diagnostic attribution, not original raster code.


## Bounded result and next decision

The frozen 256-entry pointer fixture `run-pgiwnrbq` captures 1,726 kind-8 rows
from queue 133 through queue 256. Its first unsupported rows include ordinals
833..835 at (366,307), (367,309) and (364,305). Pointer input was fixed at
screen (400,300). This establishes a reproducible unsupported-object fixture,
not causation from pointer position or equivalence to the earlier manual
queue-13 failure. Confidence is high for the numeric rows and separate static
virtual-dispatch branch; concrete object/method semantics remain unresolved.
The raw captures, frozen source manifest and source-stable execution report are
retained under `working/tests/world-kind-pointed/run-2svynvnb`; the durable
`native-world-kind8-pointed-20261008.json` preserves those exact fingerprints.

Kind 8 is a larger object-producer contract, rather than permission to admit an
unknown row into a complete native frame. Keep this fixture and strict refusal
while prioritizing GPU atlas/palette reuse. The current cache has a hard
32-entry ceiling and the renderer has a 64-surface budget: increasing the cache
entry count alone cannot retain hundreds of palette-expanded sprites. Atlas
pages or shared index/coverage storage with separately supplied palettes need
new pixel-equivalence and resource-budget validation.

The initial pointed source snapshot accidentally staged through a directory
symlink. The source hash guard detected the mutation before launch. The working
No-CD executable/preferences were restored to exact prior hashes, staged DLLs
were retained under its `source-restoration`, and original media verified
unchanged. Staging now resolves either default or explicit source directories,
requires a new destination, and rejects copied executable/configuration/DLL
write targets escaping the disposable game directory before any patch. Four
real-copy/symlink regression cases cover these boundaries. The successful repeat
uses a real directory copy and retains the failed attempt as negative evidence.
