# Native renderer player launcher

The [completion contract](native-rendering-closure.md) defines the supported
player acceptance target, configuration boundaries and remaining scope decisions.
These launch instructions describe supplied behavior; they do not certify that
all completion requirements have passed.

On x86 Linux, launch with:

```sh
./run-x86.sh
# Equivalent direct entry point:
./run-native.sh
```

Click **Launch game** in the Qt shell. Both entry points require the existing
`native-only-rendering-v1` profile: our resident World renderer and native pixel
producers supply the viewport, with original presentation/menu fallback disabled.
Wine still runs the original No-CD executable for simulation and preparation.
This selection does not establish full-game pixel equivalence or complete native
engine replacement. Unsupported rendering terminates the session.

Linux ARM64 launches select independent World delivery by default. Validated
owned snapshots let the original game proceed while a native worker renders,
and rendering runs separately from the Qt input thread. The existing two-snapshot
limit and ordered validation/return rules remain in force. Game speed and
simulation timing are unchanged. Other hosts retain synchronous delivery.

In Linux ARM64 independent mode, extra presentations reuse the completed native
World image on the GPU, then compose the UI. Replaying only the prepared draw
list can omit native surface writes between World subqueues, making parts of
the completed frame disappear on the extra presentations. Each new game update
still runs its full native drawing pass. Explicit `MNM_WORLD_IMAGE_REUSE=0`
retains redraw mode for diagnostics; synchronous mode does not enable image reuse.

ARM independent presentation also waits for each World's own completed UI before
showing that World. Reusing the preceding UI can briefly place moving overlays or
occlusion at their previous positions. `MNM_WORLD_FIRST_SEAL_BASELINE=1` selects
this matching-UI policy; an explicit `0` retains inherited UI for diagnostics.

Select a mode explicitly, before any presentation options after `--`:

```sh
./run-native.sh --world-delivery independent
./run-native.sh --world-delivery synchronous
```

The synchronous selection preserves the previous launch behavior for comparison.
The ARM performance target is 19–20 completed game updates per second; a mode
selection alone does not demonstrate that rate on every map or workload.

Terrain lighting uses the persistent native worker by default, with capture
disabled. The existing 100 ms timeout and reply checks retain original lighting
as a fallback for the rest of a failed session. A worker failure is recorded and
the launcher reports a nonzero exit after gameplay ends, even when fallback lets
the game continue. To select original lighting explicitly:

```sh
MNM_TERRAIN_LIGHTING=original ./run-native.sh
```

`MNM_TERRAIN_LIGHTING=observe` selects original-active lighting observation.
The player validates the lighting mode and capture selection before preparing a
prefix. This default
is a launch policy; combined-master live journeys, hour-long stability and
hardware performance retain their separate acceptance boundaries.
The default and override policy passes 18 launcher controls; Wine, prefix
initialization and owner execution are mocked in those controls.

The first launch initializes a dedicated prefix at
`working/native-player/wineprefix`, using the existing hash-checked supplied
DirectPlay installer. Later launches reuse it. Another launch using the same
player prefix refuses; an active Wine server must finish before reuse. Foreign
prefixes and symlinked prefix paths refuse. An interrupted initialization is
preserved for inspection; it is not silently repaired or overwritten.

Prepare `working/game-nocd` using `tools/prepare-working.sh` before launch.
Dependencies include Wine/wineserver, Xvfb, Qt/CMake and the existing PE32 adapter
build tools; first initialization also needs wineboot and 7z. The installer
requires its documented, hash-pinned Wine builtin DLL version and refuses a
different one. Manifest verification brackets initialization and the existing
owner launch. Original media remain read-only.

Presentation options go after `--`:

```sh
./run-native.sh -- --skip-movies --no-native-music
./run-native.sh -- --software-rendering
```

Movies otherwise use native media. Music retains the Qt shell's prepared-catalog
requirement; use `--no-native-music` if the catalog is unavailable. The strict
profile accepts only its existing supported presentation/media options.

For a different dedicated destination, or a stopped admitted prefix template:

```sh
./run-native.sh --prefix working/native-player/alternate-prefix
./run-native.sh --prefix working/native-player/cloned-prefix \
  --prefix-template working/path/to/stopped-owned-prefix
```

A template is only accepted for a new destination; its verified source remains
unchanged. The persistent player prefix is separate from ordinary `WINEPREFIX`
and from historical test prefixes. Existing save synchronization stays with the
menu/session services; this launcher does not import another prefix's saves.

The entry point reserves 4 GiB for initialization, build and session output via
`tools/working-retention.py`, and monitors the 90 GiB main-drive reserve. A storage
refusal must be resolved before launch. Help needs no storage reservation:

```sh
./run-native.sh --help
```

Select the original renderer explicitly for compatibility/diagnostics:

```sh
./run-x86.sh --original-renderer
./run-x86.sh --original-renderer check
```

`tools/run-game.sh` remains the original backend used internally by adapters;
`tools/run-qt-shell.sh` remains the configurable capture/replay and diagnostic
entry point. Asahi setup/launch is unchanged; strict live native ownership on
ARM is not established by this launcher.

Validation includes 18 launcher controls and actual fresh/reused public-player
journeys. Expanded x86 checks cover both naturally completed intros, native
music/voice retirement, Save/Load and 640/800 display rebuilding, two offered
Quick Battle maps, six two-minute physical-input phases with five Mini returns,
and a separate NVIDIA run. Each accepted journey finishes original normal Quit
with balanced Worlds and zero cursor/graphics owners. The expanded runs check
391 repository inputs from the actual linked Qt compiler closure.

These are bounded, source-specific runs. Earlier source phases, failed attempts
and transport refusals remain recorded; the old crash cause, natural self-copy
and keyed GPU-source branches, arbitrary maps/saves/duration, original-driver
pixels and ARM ownership remain unproved. See
[initial acceptance](../research/runtime/native-player-acceptance-20261026.md) and
[expanded acceptance and remaining boundaries](../research/runtime/native-player-expanded-20261026.md).

The first 15-control attempt had one Bash test-harness failure: sourcing a script
without arguments inherited the harness's script-path argument. The harness now
shifts that argument before sourcing, and all 15 controls pass. That attempt did
not start Wine or exercise original rendering. Its test-source SHA-256 was
`2396a839e447b1932d2c16fa7947043ecccbc4a45ed6f4174ec6b5e74e9b2388`.
