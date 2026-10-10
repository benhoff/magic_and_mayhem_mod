# Bounded original-active minimap observation

This route extends the existing closed producer observer/replay journal with
owned terrain/marker/camera inputs and selected return/object result checks.
`--canvas-producers --minimap-owned` enables the explicit V2 journal; original
rendering remains active. [Wire contract](../formats/canvas-producers-v2.md).

Existing No-CD terrain/cell/creature and view-zero entry hooks are retained.
Three additional signature-checked entry trampolines at `0x552b50`, `0x552f20`
and `0x5532f0` forward the other outline entries. Only six complete prologue bytes
are displaced (`83ec28535556`); the subsequent original `mov esi,ecx`/`push edi`
remain in the original body. Hook assembly saves/restores registers, flags,
x87/SSE state and LastError with the existing entry/return mechanism. Synthetic
forwarding and original-active live comparison are separate evidence stages.

Source centers, rotation extents, palette words, flash/fog gates, per-creature
visibility and camera directions are copied at entry. No simulation visibility or
camera-vector generation is replaced. Creature rows must match positive-stride
placement. Terrain uses native prior auxiliary canvas composition; original
pixels never seed native storage. Native replay checks cell EAX word offsets,
creature dimension refresh, camera origins and terrain invalidation. Borders and
unsupported source/layout cases remain explicit refusal paths. Four-view offline
pixel comparisons remain independently registered with their historical hashes.

```sh
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-scene-game.py --canvas-producers --minimap-owned \
  --samples 16 --magic-items 0 --skip-window-screenshot --claims <claims.json>
python3 tools/test-native-canvas-producers.py --capture-report <capture-report.json> \
  --claims <claims.json>
```

Claims must be declared prospectively for the owned observation contract. A live
run supports only its observed rotations, centers, list/flash/fog configurations;
synthetic four-view adapter checks do not assert live coverage of absent states.
Real original locks remain active; only selected pixel/result behavior is compared.
No general caller ABI, outer-border or complete HUD replacement is claimed.

The original key handler at `0x46c070` calls `MapVirtualKeyA(VK, 0)` through
IAT `0x5c528c`. Its scan-code dispatch routes 51 (comma) to `0x46d348` and
52 (period) to `0x46d378`, setting opposite pending rotation requests at
`0x689b51` and `0x689b55`. The camera update around `0x4fd604` adjusts the
orientation modulo four and calls `0x556330`, which assigns minimap object
`+0xc5` and invalidates `+0x8d`. This is a static trace of the pinned No-CD
build; full keyboard dispatch and camera-vector generation remain outside scope.

The optional `--motion` mode of `tools/test-minimap-live.py` builds a bounded
XTest input fixture and starts it inside the same disposable Xvfb server as the
capture. It resolves a unique mapped 800x600 game input child using the existing
WindowHost discovery, focuses that child, then sends arrow holds and comma/period
presses at completed World-return journal records. It writes no original camera or
simulation memory. Held arrows are released on completion or failure. Fixture
pacing is an explicit native observation policy: strict opt-in
`MNM_MINIMAP_INPUT_FIXTURE=1`, only with owned minimap observation, sleeps 250ms
at each nonfinal sampled World return. Default captures retain their existing
timing. This fixture proves only the states actually observed in its report;
it does not establish general input timing or interactive driver behavior.

The live motion investigation found two additional observation boundaries. The original outline dispatch table at0x555e08 targets0x555c4e,0x555c57,0x555c4e,0x555c60, whose calls select5527a0,552f20,5527a0,552b50. That caller state is independent of minimap+c5; V2 therefore carries the actual selected outline entry separately from the stored orientation. Entry5532f0 remains instrumented and synthetically forwarded, but this table does not call it. Moving-camera captures also exceed the earlier65536-record journal; V2 uses an explicit262144-record/512MiB bound, keeping original completion storage independently bounded. Failed/truncated trial records remain diagnostics under working; they do not support integration claims.

The finite preview reader accepts the larger V2 journal while encoded asset reads retain128MiB limits. The interactive Qt stream consumer retains its existing128MiB tail bound and is not part of this V2 live-observation milestone. A first-render input trial exited before a complete World return; startup queue files describe entry snapshots, so the fixture now follows operation12 World returns and refuses missed scheduled boundaries.

The preserved [motion result](native-minimap-live-motion-20261009.json) passed
all1062 independently compared checkpoints and474236912 pixels with zero
mismatches. It recorded all four stored orientations, seven distinct centers,
same-orientation panning, fog1, flash0/1, RGB565 and60 creature entries. The
actual outline mapping was0→entry0,1→entry2,2→entry0,3→entry1. Entry3 at5532f0
remains covered by synthetic forwarding and isolated pixel fixtures only.
The24 adapter cases also exercise distinct stored orientation/selected entry
values;36 malformed/result/undefined refusals pass. The52-entry synthetic
forwarding fixture preserves registers/flags/x87/SSE/LastError and rejects
five invalid fixture configurations before hook mutation. Source declarations,
actual compiler dependencies, manifest checks and child reports are bound in
the immutable result. This promotes scoped native observation integration,
with original drawing still active and replacement remaining `none`.
