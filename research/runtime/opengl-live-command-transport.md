# Live bounded native command transport

Native policy, 2026-10-05. `NR.live-command-transport` connects the existing
owned-session hooks to `LiveCommandRenderer`, without replacing original calls.
The [wire contract](../formats/render-command-channel.md) has separate identity,
publication, terminal state and cancellation; frame/input/media v1 remain intact.

`runtime/render/command_channel.h` attaches only when
`MNM_RENDER_COMMAND_CHANNEL` names a fresh channel. `owned_session.h` mirrors its
existing ordered records into both the evidence file and the live mapping.
CREATE/UPDATE/COPY/PALETTE/SWAP pixels come from owned original snapshots, not
CHECK expectations. Existing tracker, palette, geometry and session admission
still apply. The producer remains bounded to 16 observed session operations;
unsupported fill/identity/ownership branches can GAP before a usable preview.
Mirror failure terminates the session and never changes original HRESULTs,
LastError preservation, pixels or original drawing dispatch.

`CommandDecoder` retains incomplete wire headers/payloads and independently
validates session sequence/resource/geometry contracts as complete records arrive.
`CommandChannel` copies bounded acquired immutable prefixes. `LiveCommandRenderer`
queues decoded commands and executes at most 32 per Qt poll with the persistent
consumer and shared GPU textures. It drains already-published terminal bytes
before acknowledging END. Cancellation, gap, malformed framing, regressions,
unknown state, identity changes and incomplete producer exit abort resources and
clear the native viewport. Default CHECKs are structurally admitted but skipped;
Verify is explicit on the adapter. CPU diagnostic framebuffer reads in tests are
separate from ordinary execution.

## Usage

```sh
./tools/run-qt-shell.sh --software-rendering --native-commands
```

Launch game normally from the shell. This opts into `--capture-locks` and a new
owned command session per launch; the staging launcher validates the new file
before original preparation, records its path in the experiment manifest and
passes its Wine path to the DLL. Qt handles no channel offsets in its widgets:
launch orchestration delegates to the adapter. Movies may use their existing
separate media route. The original Wine game window remains available. Failed
native sessions instruct the user to use that window; complete bounded sessions
retain their last GPU frame until launch lifecycle cleanup. There is no continuous
session restart or automatic renderer replacement.

## Validation and remaining boundary

```sh
cmake -S renderer -B working/build/live-render-channel -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/live-render-channel --parallel 4
ctest --test-dir working/build/live-render-channel --output-on-failure
xvfb-run -a python3 tools/test-live-render-channel.py working/build/live-render-channel \
  --sanitized-build working/build/live-render-channel-sanitized
```

The sanitizer build uses `-fsanitize=address,undefined -fno-omit-frame-pointer
-fno-pie` and executable linker `-fsanitize=address,undefined -no-pie`; Qt/driver
cache leak detection is disabled. Synthetic x86 writer admission/publication and
refusal tests exercise the exact C publisher. Native tests check one-byte and
seven-byte fragments, delayed producer terminal, early terminal draining, a
32-command execution budget, ownership cleanup and invalid/incomplete sessions.
Independent complete framebuffer checks cover small native fixture prefixes.

Wine integration invokes the actual intercepted COM hooks over independent
synthetic engine storage. RGB mixed/failed-call/alias and indexed 800x600 sessions
are compared at every PRESENT against hashes of all independently sampled native
pixels, including palette changes and flips. Each valid session observes at
least one presentation while the Wine producer PID is still running; deliberate
fixture delays provide scheduling opportunity and make no throughput claim.
The mirror file and acquired mapping bytes must agree exactly. Restore, held-lock
and pre-launch cancellation fail explicitly and release owned surfaces.

No original artifacts are consumed by these synthetic tests. Original full-game
shadow output, startup/drawing branch coverage, indefinite sessions/resynchronizing
checkpoints, physical GPU performance/context loss and live replacement remain
pending. Hook-origin native integration is recorded separately from observation
or equivalence of the actual game. Historical records keep their original hashes;
changes to shared command/hook sources leave earlier results stale.

The frozen [native execution record](opengl-live-command-transport.json) passes
all seven Wine cases, normal/sanitizer fragment and cleanup tests, and eleven C
publisher cases. Four complete sessions agree at every PRESENT and three invalid
sessions refuse; ordinary native readbacks and viewport image uploads are zero.
All eight renderer CTests and existing protocol binding checks also pass.

The owned hook's existing RGBA frame publication remains active for observation;
Qt's command route ignores that channel. Zero readback/upload counters describe
the native GPU consumer and viewport, not removal of producer snapshot copies,
legacy RGBA conversion or original rendering work. A completed bounded native
preview stops Qt input forwarding and asks the user to continue in the original
window until continuous session ownership is implemented.
