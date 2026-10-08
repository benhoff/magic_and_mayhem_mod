# Native World canvas history, 2026-10-07

## Confirmed recovered boundary

Live before/after captures establish that the World consumer can leave nonzero
canvas pixels untouched. Private unmodified original execution initialized from
the captured diagnostic before-canvas reproduces the live after-canvas. Native
replay and private original replay starting from zero agree with each other;
their remaining live differences can be pixels already present before the queue.
The diagnostic before-canvas is used only by the private original reference.
It never initializes the native renderer.

The retained four-frame capture `run-bk_pvd0g` has 15 differences at each sample,
all unchanged from before to after. Coordinates span (617,217)..(622,221),
with word 32. These observations confirm retained pixels for these frames;
they do not identify the writer that originally produced those pixels.

The bounded lifetime observer logs from the first queue entry, before sampling
skips. It records dimensions, stride, pointer-identity tokens, nonzero counts,
an FNV-1a content fingerprint and selected raster backends. Raw addresses remain
diagnostic only. A token identifies a pointer value within that process;
allocation generations and ownership are not established by matching pointers.
The first observed World canvas is already nonzero. A clear initial state cannot
be assumed merely because this is the first observed World queue.

For the pinned No-CD build at preferred base 0x400000, the observer reads
`0x658174` (canvas pointer), `0x6a2dc8` (word stride), `0x6a49b8` (right clip
bound/full-capture width) and `0x656618` (bottom clip bound/full-capture height).
These are runtime globals outside the inventory's file-backed data ranges,
documented as observed addresses rather than file-backed original data links.
Static retained disassembly has one direct canvas-pointer store at `0x57dc24`
inside setter entry `0x57dc20`; this corroborates a setter but does not establish
allocation ownership or all indirect canvas writers. Confidence is high for the
observed reads and bounded pixel comparisons, partial for lifecycle recovery.

Startup observation also retains refused drawing paths explicitly. Reason 7
includes the lazy displacement-table initializer boundary; reason 8 includes
unadmitted queue kinds. These requests are still executed by the original game.
`--observe-world-refusals` labels them as diagnostics, without admitting partial
requests to native drawing. Whole startup reconstruction remains pending.

## Implemented native policy

`SceneHistory` owns a `SceneRenderer` and its GPU canvas. It starts from an
explicit caller-owned native background and preserves completed native pixels
between frames. A logical canvas identity and contiguous source sequence are
required for continuation. Gaps and identity changes require an explicit native
reset; regressions and duplicate sequences are refused. This admission precedes
drawing. Unknown resources preserve the previous completed frame. Partial
drawing cannot be read or presented. An execution failure poisons continuation
until a successful native reset. Batched drawing and ownership cleanup remain
bounded.

`mnm-world-history-preview` exposes this service for closed, pinned request
timelines. It accepts no original pixels. Six composition modes, clipping,
masked holes and ordered retained destination pixels are compared with a private
unmodified original process over an explicitly zero-initialized synthetic chain.
This is a bounded reconstruction comparison, separate from original startup and
interactive live history integration.

Reproduce:

```sh
xvfb-run -a -s '-screen 0 1600x1024x24' python3 tools/capture-scene-game.py \
  --world-frames --world-lifetime --observe-world-refusals --skip-queues 0 --samples 4
xvfb-run -a -s '-screen 0 1600x1024x24' python3 tools/capture-scene-game.py \
  --world-frames --world-lifetime --samples 4 --interval 10
xvfb-run -a python3 tools/test-world-history.py \
  --capture-report <later capture report> --startup-report <startup capture report>
```

Current evidence is `native-world-history-comparison-20261007.json`: eight
contiguous native/private-original canvases match all 3,072 pixels, and four
retained older live inputs each have the same 15 unchanged pre-consumer pixels.
Their 1,920,000-pixel private original before-state replay matches actual output.
The separate `native-world-history-current-capture-20261007.json` compares four
freshly captured canvases with no zero-initialized differences. This positive
case does not erase the retained failing case. The startup record
`native-world-startup-capture-20261007.json` starts with 365,306 nonzero pixels
and preserves three refusals in four sampled queues. All 66 native regression
checks pass in `native-world-history-regressions-20261007.json`.
The current ordinary viewer presents 24 complete native frames with zero CPU
pixel readbacks, zero viewport image uploads and zero remaining surfaces. Its
212 publication drops and 23 superseded packets are retained in
`native-world-history-live-normal-20261007.json`; that run still uses the
explicit stateless native background. Cold admission remains synchronous
(1,235 ms worst poll in this run).

## Remaining integration boundary

The current live channel drops publication attempts and can supersede completed
packets; World v1 sample/publication sequence is not an actual every-queue source
sequence. Merely retaining the previous displayed canvas would apply an
incomplete history. The live viewer continues its explicit stateless native
initial-background policy. Full live history needs recovered startup/clear and
buffer ownership, complete earlier producers, and native reconstruction of every
intervening queue, including unpublished requests. No original destination
pixels may be used to fill that history. Whole World replacement remains
unvalidated and original drawing stays active.
