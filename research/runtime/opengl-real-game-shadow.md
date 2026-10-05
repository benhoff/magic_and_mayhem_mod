# Bounded original-game command shadow

2026-10-05. This is original-game startup observation, separate from the
synthetic transport tests and from original pixel equivalence or replacement.
The pinned executable is the no-CD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Confirmed first blocker

The first admitted startup color fill fails the command channel with GAP before
any command bytes are published. The lifecycle log records `fill_ready`, followed
by `blit_initialized` and `blit_propagated`, without `session_started`.
The native consumer refuses the failed channel, presents zero frames and leaves
zero native surfaces. Original drawing continues through subsequent Locks,
Unlocks and blits, including primary owned-frame publication.

The code boundary is explicit in `game_session_blit_begin` in
`runtime/render/owned_session.h`: `p->fill` calls `game_session_gap(6)` before
`session_surface` can start the stream. The live channel records its separate
wire reason GAP (2); this must not be confused with file GAP payload 6.
The existing owned tracker reconstructs successful constant fills, but the
ordered native command vocabulary does not encode one. This is a confirmed
missing command route, not evidence that native copy or GPU presentation is
incorrect. Confidence is high for this startup and code branch only.

The [frozen execution record](opengl-real-game-shadow.json) contains the exact
sources, pinned original hash, staged manifest provenance, channel state,
consumer result, lifecycle diagnostics and retained artifact hashes. Its
`success` means the observation harness completed; `native_consumer.success`
remains false. No usable full-game command presentation was established.

## Reproduce

```sh
cmake -S renderer -B working/build/live-render-channel -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/live-render-channel --target live-render-channel-test
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel
```

The harness verifies all original artifacts before and after execution, creates
fresh frame/command channels, stages a hash-guarded executable import, copies a
Wine prefix and initializes it. It allows up to 90 seconds for original command
activity, then starts the native consumer against the immutable published prefix
and observes original drawing for 20 seconds. It retains an original-window
screenshot and terminates only its process groups and isolated Wine server.
Movies and CD music are disabled only in staged preferences. Nothing replaces
original rendering or gameplay. Consumer startup may follow the first published
commands; this run does not establish per-command live scheduling latency.

Earlier attempts ended without drawing evidence or during harness prefix setup.
Their logs remain under `working/tests/live-render-game/`. They do not establish
an engine defect or the cause of startup delay. The successful procedure changes
both startup timing and display size; it does not isolate which caused the
earlier absence of drawing.

## Next bounded change

Add an explicitly admitted native constant-fill command, or an equally explicit
fill route using existing native commands, covering initial full-surface fills,
later rectangular fills, native pixel width, failure and sequence/resource
limits. Connect the existing owned fill inputs to it after successful original
calls; do not substitute CHECK bytes as rendering inputs. Validate independent
synthetic original storage first, then repeat this real startup experiment.

Further blockers may emerge afterward. A complete native session, independent
original-surface pixel comparisons, gameplay scene coverage, continuous-session
policy and live replacement all remain pending. The existing owned RGBA stream
and CHECK bytes are reconstructed observer state, not independent driver output.
