# Record once, replay the native World renderer

Use this to compare renderer versions against exactly the same moving scene.
The capture preserves engine-produced drawing instructions and resource/history
updates. Replay executes those instructions through the current native renderer
without running Wine, the original engine, AI or game input.

Record a few named cases: walking crowd, melee, fire/spells, and camera movement.
Keep the same case and asset files when comparing optimizations. A recorded
creature count or trajectory cannot be changed by issuing new game orders during
replay; that needs another recording or an explicitly authored control.

## Record a case

From the repository root:

```bash
./tools/render-replay.py record working/render-replays/crowd.sqlite3 --label walking-crowd
```

The normal native player opens. Load a prepared battle or perform the desired
actions once, then save and quit normally. The first gameplay session becomes
the case. The default capture stops accepting data after 3,600 World updates
(about three minutes at 20 Hz); gameplay continues. Use `--frames 600` for a
shorter recording. This limits diagnostic I/O, not simulation or rendering.

Recording starts at World generation 1, including the setup needed to interpret
later frames. Recording uses the existing bounded background capture queue and
prefix compression. It can perturb live FPS and is not a performance benchmark.
The compressed acquisition remains under the printed `recording-*` directory.
If recording fails, it stays available for diagnosis; a case is not silently
promoted from an interrupted run. Use a fresh output name for each case.

Existing resident captures in formats 1–4 can also be imported:

```bash
./tools/render-replay.py import working/render-replays/crowd.sqlite3 \
  --capture /path/to/resident-source.bin --label walking-crowd
```

The importer validates the entire capture, including an excluded tail if
`--prefix N` selects a completed prefix. It preserves source-history chunks;
flattening or inventing missing initial state is refused.

## Watch and inspect the same frames

```bash
./tools/render-replay.py inspect working/render-replays/crowd.sqlite3
./tools/render-replay.py view working/render-replays/crowd.sqlite3 --start 1800 --end 1920
```

The separate **World renderer replay** window provides Play/pause, Previous,
Next, Restart and a frame number. Space pauses; Left/Right step; Home restarts.
Pause before entering a frame number. The default requested playback rate is
20 Hz; `--fps 10` slows the same captured positions and actions.

Seeking rebuilds renderer and producer history from frame 1. It can take time
for long captures. Earlier frames initialize state but are excluded from a
selected benchmark window. Restart, backward seek and benchmark repetitions
create fresh renderer state, preventing shadows/displacement from accumulating
over a previous playback. No keyboard or mouse events are sent to a game window.

Only the World image is replayed. HUD/menu composition and audio are separate.
Capture ordinals do not contain the original wall-clock intervals, so viewing
uses the requested fixed rate. IPC delays, original pacing, the dedicated live
render-owner loop, compositor timing and input response are not reproduced.

## Compare pixels and renderer cost

```bash
./tools/render-replay.py verify working/render-replays/crowd.sqlite3 --start 1800 --end 1920
./tools/render-replay.py benchmark working/render-replays/crowd.sqlite3 --start 1800 --end 1920
```

`verify` compares every selected frame's native pixel hash across two fresh
replays. It prints a run ID. To compare a later renderer build against that
retained run, use the same case, range and results database:

```bash
./tools/render-replay.py verify working/render-replays/crowd.sqlite3 \
  --start 1800 --end 1920 --against 1
```

Replace `1` with the earlier verification run ID. Different pixels fail the run
and identify the first differing frame. A repeatable hash proves repeatability;
the earlier native version is a regression reference, not an original-game
pixel oracle.

`benchmark` reports native completion milliseconds per World: CPU production,
scene preparation and GPU drawing with a serial completion wait. Capture
decompression, source/request decoding and handoff, file I/O, diagnostic pixel
reads and hashing are outside those frame timings.
It performs one final diagnostic read; verification reads every selected frame.
Ordinary World delivery performs no readbacks. Compare the distribution against
the 50 ms budget for 20 updates/second. These are isolated renderer service times,
not live-game FPS, GPU busy time, Qt swap pacing or physical scanout.

Use `--backend software` for an isolated llvmpipe control. Hardware mode uses
the normal local OpenGL driver and reports its renderer name. Avoid competing
game/GPU workloads when measuring performance.

For native spike attribution, export the existing opt-in owner stage counters:

```bash
MNM_WORLD_OWNER_PROFILE=1 ./tools/render-replay.py benchmark working/render-replays/crowd.sqlite3
```

Each selected frame's `owner_profile` in the results SQLite payload separates
producer operations, binding/preparation, scene admission, drawing and
presentation. Export occurs outside frame timing; collecting the counters itself
adds diagnostic overhead. These are wall times, not CPU self-time or GPU busy
time. Use ordinary benchmarks for performance comparisons.

Native World rendering batches consecutive additive particles using bounded
owned source contributions, preserving exact RGB565 saturation and draw barriers.
`MNM_WORLD_ADDITIVE_BATCH_BASELINE=1` selects the older serial path for diagnostic
comparisons. This changes native drawing only; simulation and game pacing stay
the same.

## Storage and validation

Cases are SQLite databases with ordered, hashed, prefix-compressed source bytes,
explicit event kinds, provenance and pinned asset hashes. Case output is never
overwritten. Missing generations, damaged compression, trailing data and asset
changes refuse replay. The default asset root is `working/game-nocd`; specify
`--assets PATH` for another byte-identical installation. Assets are shared
read-only rather than copied into every case.

Results append to `working/render-replays/results.sqlite3`, or `--results PATH`.
It retains per-frame timings/hashes, errors, commands, exact replay executable
bytes and tool source identities. Temporary compressed interchange files are
reconstructed from SQLite and removed after successful replay; failures retain
their interchange and diagnostics. New cases allow at most 8,192 events, 16 GiB
decoded input and 512 MiB compressed payload. Source-history records count toward
the event bound. Existing repository disk reserves remain in force for recording.

Focused controls:

```bash
python3 tests/test-render-replay.py
python3 tests/test-render-replay.py --renderer working/build/render-replay/mnm-world-replay
```

The native controls use authored 32-instance moving SPR scenes with independent
asset bindings, destination-dependent rectangles, repeat hashes, selected-range
history restoration, source-only initialization, actual viewer rewind and a
deliberately damaged baseline that must fail. They do not simulate original AI
or prove original pixel equivalence. The existing capture controls additionally
check the opt-in recording limit and native/Python prefix-codec compatibility.
