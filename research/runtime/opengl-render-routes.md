# Original route observation and independent menu pixels

2026-10-06. The hash-pinned no-CD game remains responsible for drawing and
simulation. The continuous v2 native consumer polls every 16 ms, matching the Qt
shell, beside the original 800x600 Wine client on an isolated 1800x1000 Xvfb display.
This extends observation beyond startup; it does not replace original work.

`tools/test-live-render-routes.py` stages the existing rendering bridge, verifies
its executable/DLL hashes, and adds the existing observation-only MenuAnchor import
by script. No action channel or native campaign dispatcher is enabled. The menu
observer forwards original callbacks/ticks; three after-World-tick records verify
receiver `0x6cbb78`, screen 2 and initialization after New Game -> Region Entry ->
Enter. These addresses are specific to source SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

The new Qt probe waits for an explicit attachment marker. The controller cancels
the initial unread ring, waits for producer failure acknowledgment, then attaches
through automatic RECOVER. It injects a second failed reader in Region Entry.
Snapshots are explicit diagnostic framebuffer reads, individually requested and
bounded to 32 total ordered controller requests. Ordinary frame publication still
asserts zero renderer readbacks and viewport image uploads. Each probe has a
180-second deadline; the controller has separate bounded phase waits and terminates
its isolated processes/prefix on exit. The original installation is copied into a
real disposable directory. Source preferences stay byte-identical; immutable
manifests verify 2,927 files before and after.

## Fresh observations

The [retained execution record](opengl-render-routes-live-20261006.json) fingerprints
the current harness, producer, consumer, menu observer and rendering wire sources,
and the staged binaries, logs and PNG artifacts. The committed-history report
reviews `2013cae..63863bc` separately from these workspace changes. The earlier
campaign exploration remains under `working/tests/live-render-routes/run-vpcxmm_j`;
it is not the final source-matching result.

| Route / boundary | Observed result |
| --- | --- |
| Late attachment after original Main readiness | Fresh session 124 restores complete owned state and presents native frames. |
| Region Entry failed reader | Session 125 restores complete owned state, returns Active, and resumes native presentation. |
| Original New Game -> Enter -> World | Three forwarded original World ticks; 238 total native frames before final fallback. Original process and World drawing remain active. |
| World resource admission | Session gap reason 2; checkpoint refusal reports 33 observed surfaces, exceeding the current 32-surface budget. The refused resource has complete 48x48 RGB565 pixels; this particular refusal is not evidence of a missing pixel baseline. |
| Fresh-observation recovery after World loading | Session 126 starts without a complete checkpoint, then logs untracked/invalidated copies and session gap reason 3 before a complete native frame. Consumer falls back and frees all resources. |
| Movie-enabled startup | Source PlayFMV/PlayFMVOut remain TRUE in the staged preferences. Main becomes ready; session 124 presents 217 frames over the approximately 11-second native observation and remains Active until test cancellation. |
| Movie sample activity | No return PC within `0x469830..0x469a00` appears in the bounded 2,048-record application draw archive. Actual movie playback, wrapper result and movie pixels remain unverified; preference enablement and arrival at Main are insufficient proof. |

The World refusal is a finite resource-admission boundary, not a throughput
benchmark. The first failure occurs before the controller's intended World
failure injection, so that injection is explicitly skipped. The record retains
session IDs, ring headers, exact diagnostic values and original route events.
Neither unknown source pixels nor a resource over-budget condition is silently
accepted. No queue, surface, pixel, archive, retry or timeout budget was increased.
Diagnostic logs are bounded and deduplicated; they do not provide every original
caller or establish the exact API return PC responsible for each World gap.

## Independent pixel comparison

The controller locates the named original Magic & Mayhem X11 client through window
geometry/translation, independently of the producer's owned buffers and frame
mirror. PIL captures that client from X11. The Qt probe explicitly captures its
native framebuffer. Pointer movement outside the client avoids cursor contamination.
RGB comparisons use a predeclared per-channel tolerance of 8 and at most three
attempts per region; every final comparison passed on its first attempt with a
maximum difference of **1**, and no pixel exceeded the tolerance:

- Main title `(100,30)-(700,180)` in both campaign and movie-enabled runs.
- Region Entry title `(45,40)-(220,80)` before and after forced recovery.
- Region Entry difficulty text `(45,92)-(620,114)` before recovery.

These are five independently captured **stable menu regions**. Native/X11 images
are unsynchronized and omit the cursor, animation, full screen, terrain, sprites,
effects, HUD and movie output. RGB565 expansion/rounding can produce the recorded
one-value differences. No whole-operation equivalence, original driver equivalence,
gameplay pixel comparison, or live replacement status is promoted. Both consumer
finishes report zero native surfaces/palettes; retained artifact hashes were checked
again after process termination.

Reproduce with:

```sh
cmake -S renderer -B working/build/renderer
cmake --build working/build/renderer --target live-render-route-probe -j4
xvfb-run -a -s '-screen 0 1800x1000x24' \
  python3 tools/test-live-render-routes.py working/build/renderer
```

Use `--mode campaign` or `--mode movies-enabled` for a smaller isolated run.
The next producer milestone is a finite resource working-set policy suitable for
World loading, with explicit dependency admission/retirement and complete recovery
checkpoints. Capture the rejected original call sites before classifying remaining
copy branches. Movie validation also needs bounded wrapper/sample evidence and
paired decoded output; merely retaining the movie-enabled case does not complete it.
