# Native terrain and creature scene (NS07)

Reviewed 2026-10-05. This is an offline presentation milestone, separate from
original mixed-scene behavior equivalence and live replacement.

`apps/world-scene` composes the validated NS06 MovementSession, frozen ordinary
terrain heights, owned ANI display cursors, existing ANI body offsets and SPR
origins, and the recovered terrain producer/signed queue. Qt remains in the
application/presentation layer; the native world has no widget, renderer or
build-specific dependency. The scene reads controller state and never runs an
independent animation clock. Current display takes precedence over the previous
completed segment display; pending/unstarted actors have no displayed sprite.

The visual terrain is explicitly diagnostic: a caller-selected TTD body repeated
at frozen-grid coordinates, with a caller-selected standing layer's ordinary
fine height. It does not reinterpret installed MAP bytes as navigation inputs.
The scene's cell-centred isometric projection, body centring, camera origin,
viewport, and body-only terrain submission are native inspection policies.
Terrain and creature body are sorted together using the recovered signed queue;
creature priority bias is the previously recovered body bias 6. Fine terrain
heights replace whole-layer depth annotations for this visual fixture. Four
views rotate the diagnostic projection; no camera-relative ANI selection is
inferred. Embedded palettes are unshaded and the visibility pass is omitted.

Validation commands and runnable examples are in
[the app README](../../apps/world-scene/README.md). The standalone C++ scene test
compares 72 complete masked/clipped RGB565 compositions with an independent CPU
oracle, checks all four diagnostic projections, completed-segment display,
invalid controller display refusal, no rendering-side controller advancement,
and zero retained GPU surfaces after each frame.

The installed-asset validation script `tools/test-world-scene.py` checks three
synthetic frozen profiles (terrace, slope, consecutive vertical), each in four
views. It compares exported positions against headless simulation traces, queue
key ordering, complete SPR pixel compositions from an independent decoder, and
fresh-process split/resume PNG, RGB565, queue and final checkpoint equality.
The deterministic ANI fixture uses existing movement-test programs and explicit
sprite IDs; installed TTD/SPR assets provide actual pixels.

Confidence is
high for these bounded native integration checks; no original mixed-scene pixel
agreement, installed MAP navigation, live agreement or replacement is claimed.
Remaining work includes real world geometry/navigation admission, camera/action
mapping, attachments, shaded creature palettes, visibility, multiple-creature
occupancy and scheduling. Gameplay balance remains unchanged.

## Recorded validation

The normal standalone build has 97 CTests passing across the full initial run
and local-display retries. The first sandboxed run passed 91 and could not start
its six Xvfb/OpenGL tests. Local retries passed those six; one concurrent Xvfb
cleanup collision required a final serial retry, which passed. This was a display
execution boundary rather than a pixel mismatch. The new scene test also passes
under ASan/UBSan (leak detection disabled for Qt/OpenGL process caches).

Both builds pass 144 complete installed-SPR pixel comparisons and 12
fresh-process frame/queue/checkpoint continuations. The sanitizer run also checks
creature projection anchors and depth keys independently from the submitted
queue. Before/after manifests each verify all 2,927 original files.

- Normal frame/persistence report: `working/tests/world-scene/run-ivaacvnw/report.json`.
- Sanitizer report and build/test/display-retry logs: `working/tests/world-scene/run-4t0plrh8/`.
- Installed `Creatures/redcap.ani` base 8 and `Creatures/RedCap.spr` smoke export:
  `working/tests/world-scene/run-ivaacvnw/installed-demo-000.png`, with its JSON,
  checkpoint and before/after manifest logs. Manually inspected; this adds no
  claim of original mixed-scene agreement.
- Source/artifact hashes and exact scope: [machine-readable evidence](native-world-scene.json).

Builds use `working/build/world-scene` and `working/build/world-scene-sanitized`.
Sanitizer flags are `-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`
and linker `-fsanitize=address,undefined -no-pie`, with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. Tests/scripts are the commands in the app README;
Xvfb requires a local display socket outside the restricted sandbox.
