# Canvas startup before the first World draw

Pinned No-CD build: SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred image base `0x400000`. The scope is isolated 800×600 Quick Battle,
map selection 2, movies disabled. Original simulation and drawing remain active.
This extends the [retained-canvas investigation](native-world-canvas-history.md).

## Recovered sequence

The first World consumer uses an existing screen-sized surface. It is not
allocated or cleared at World entry. Creation, a zero-fill request, menu/loading
work, and repeated locks/binds precede it. The last screen copy, lock, bind and
World entry all observe the same nonzero content fingerprint.

| Step | Original site / entry | Static contract and live observation |
| --- | --- | --- |
| Create presentation surface | `0x4e4155` → `0x58ad90` | Initializes wrapper `0x6a4fe0`; descriptor caps `0x200`. Live wrapper has separate interface fields at +4/+8 and 800×600 dimensions at +0x10/+0x14. |
| Create drawing surface | `0x4e41ec` → `0x58ad90` | Initializes wrapper `0x6a49e0`; caps `0x840`. Same dimensions, separate interfaces. Exactly one creation and no release of this wrapper occur in the bounded pre-World trace. |
| Request startup clear | `0x4e41f8` → `0x58bc10` | Full-surface fill with word zero. This is the only observed full-fill request for `0x6a49e0` before World; no selected rectangle-fill request targets it. The initial clear has no prior lock sample, so its pixel completion is not measured. |
| Reuse for menus/loading | `0x58b660`, `0x57dc20`, `0x581ec0`, image/copy wrappers | Hundreds of successful locks of `0x6a49e0` return the same process-local pixel pointer. Menus and loading paths repeatedly bind it. Content becomes nonzero before World. |
| Final screen copy | `0x46b9fc` → `0x58bf40` | Source `0x6a49e0` (ECX), destination `0x6a4fe0` (first stack argument), offsets (0,0). The retained trace sampled source-wrapper fingerprints, matching the later World entry. The direction is corrected by the producer disassembly; it presents existing draw-canvas content. This does not independently prove completion of every driver write. |
| Set clip | `0x4fcfc7` → `0x57dc60` | Before the first consumer, clip globals hold (left,top,right,bottom) = (0,0,800,600). |
| Lock for World | `0x4fd531` → `0x58b660` | Wrapper `0x6a49e0`; returned pixels have word stride 800. Wrapper +0x18 holds byte pitch 1600. |
| Bind for World | `0x4fd54d` → `0x57dc20` | ECX = pixels, EDX = byte pitch / 2, stack argument = height. The setter stores pointer, word stride and height; it neither allocates nor clears. |
| First World consumer | `0x4fd554` → `0x5002a0` | 800×600 canvas already has 365,306 nonzero pixels in the final retained run. Same fingerprint as the preceding copy/lock/bind. |

The final trace has 8,407 records, 58 creation calls across all wrappers,
714 lock calls, 636 binds, 636 clip changes, 21 full-fill calls across all wrappers,
205 opaque-copy calls, 8 JPEG decodes and 1,911 tinted font-coverage calls.
Of the locks, 686 return the first World pixel pointer from wrapper `0x6a49e0`.
Record counts depend on instrumented scheduling and are not engine frequencies.
An earlier retained run has 365,338 nonzero pixels at the same boundary; pixel
counts and process-local pointer values are not assumed stable across launches.

The menu/loading producer chain is confirmed at the selected wrapper level.
For example, `0x4dc5e0` ends loading/progress updates with a copy at `0x4dc69c`;
the final such copy before World already observes the World-entry fingerprint.
The trace does not yet attribute each pixel to a JPEG, text, rectangle, sprite
or driver operation. The native renderer must reconstruct that producer history
or establish a separately validated reset boundary. A fresh zero canvas at first
World entry is an intentional native policy, not the recovered startup state.

## Binding, ownership and unobserved branches

`0x57dc20` writes `0x658174` (pixels), `0x6a2dc8` (stride in words) and
`0x6def50` (height). `0x57dc60` separately writes `0x6e0008` (left),
`0x6cbb6c` (top), `0x6a49b8` (right) and `0x656618` (bottom). Bind height and
bottom clip bound are distinct globals even when both equal 600. These are
runtime/BSS addresses, not claimed file-backed data objects.

`0x58b660` obtains the pixel pointer from the DirectDraw lock descriptor at
`0x6f6904`, records byte pitch from `0x6f68f0` in wrapper +0x18, and returns the
pointer. It includes null-surface, lost-surface restore/retry and fatal-error
branches. The observed successful path does not validate those failures. Its
conditional unlock before returning, and caller-side unlocks, mean a readable
diagnostic pointer alone does not prove a currently held lock or ownership lease.
Fill/copy/JPEG samples use the last observed lock pointer only until a selected
create/release invalidates that wrapper association.

`0x4fd280` also has an alternate branch: when `0x6e1f68` is nonzero it binds that
pointer at `0x4fd47b` using padded dimensions, clips and consumes the World queue
at `0x4fd4ab`, then locks/rebinds `0x6a49e0` at `0x4fd4d4` for subsequent work.
`0x6e1f68` stays zero in this capture. Allocation, clear, generation, upstream
writers and ownership for that branch remain unresolved. Windowed/fullscreen
changes, alternate dimensions, movies, restore failures and later battles need
separate captures. No address is treated as stable across launches except the
pinned build's preferred-base code/global offsets.

Confidence: high within the observed wrapper sequence and static setter scope;
partial for complete canvas lifetime/producer recovery. Matching pointers and
unchanged wrapper/interface fields establish observed reuse within this run,
not a universal allocation-generation contract. Content fingerprints are bounded
diagnostics, not byte-for-byte equivalence proofs.

## Reproduce and retained evidence

```sh
xvfb-run -a -s '-screen 0 1600x1024x24' python3 tools/capture-scene-game.py \
  --canvas-startup --world-frames --world-lifetime --observe-world-refusals \
  --skip-queues 0 --samples 4
python3 tools/inspect-canvas-startup.py --capture-report <report.json> \
  --output <new-analysis.json>
python3 tools/inspect-canvas-startup.py --executable working/game-nocd/Chaos.exe \
  --output <new-static-export.json>
python3 tools/test-canvas-startup.py
```

The capture installs twelve byte-checked observation trampolines before ordinary
startup, records at most 16,384 events, and closes at the first queue entry before
sampling skips. Return slots are matched by post-return stack addresses including
callee argument cleanup; nested calls retain their own original return targets.
Registers, EFLAGS, x87/SSE state and LastError are preserved. The copied JPEG
wrapper's direct call is relocated explicitly. Diagnostic addresses inside that
trampoline are normalized to the original call-return VA. Selected engine-thread
calls are supported; arbitrary concurrent rendering/reentrant driver threads are
not validated. A missing first-World marker, truncation, overflow or unpaired
wrapper timeline refuses analysis. Instrumentation changes timing.

The [diagnostic format](../formats/canvas-startup-diagnostics.md) and retained
`native-world-canvas-startup-20261008.json` bind the selected live records to source
and artifact hashes. `native-world-canvas-startup-static-20261008.json` retains
the static windows/direct callers. Original manifest verification runs before and
after artifact-consuming captures/exports. Native output receives none of these
pixels; World shadow presentation and full live replacement remain separate.

The producer follow-up in [native-canvas-producers.md](native-canvas-producers.md) corrects the initial copy-direction interpretation and the generic sprite-dispatch label for `0x581ec0`. Historical diagnostic reports and their hashes remain unchanged.
