# Native Main / Quick Battle action bridge

## Scope

This records the original V1 integration and exit extension. Normal live menu
sessions now use the separately versioned
[Single Player / map V2 extension](single-player-menu-bridge.md); the V1 wire and
fixture remain available as regression coverage.

The first opt-in live integration connects native Main's Quick Battle button
and Quit buttons, and native Quick Battle's Cancel/Escape intent, to the original engine callbacks.
Other native actions are disabled. Original initialization, fades, screen stack,
callbacks, drawing and exit behavior remain in the game. This is a bounded
presentation/action integration, not a replacement of original menu logic.

`./tools/run-qt-shell.sh --live-menus` opens the shell; Launch game stages a fresh
hash-checked copy, launches through `tools/run-game.sh`, and uses the Wine
viewport for fallback. No OpenGL capture is required. Use original menus retires
the channel for the session and exposes the original viewport. Window close from Main or Quick Battle uses original Main Quit, returning
from Quick first. After fallback, exit through the original game menu. Relaunch creates a new channel/experiment.

`LiveMenuSession` owns asynchronous staging/launch, heartbeat, acknowledgement,
transition timeout and fallback. `MenuBridge` owns the file mapping. Widgets
emit their existing semantic signals; they contain no executable addresses or
process controls. The shell selects the visible screen from engine snapshots.
It disables actions while a request or transition is outstanding. A callback
acknowledgement alone does not authorize the next screen.

## Recovered contract and adapter

The executable hash, selected callbacks, tick and object fields are the pinned
No-CD contracts in [menu-engine-observation.md](menu-engine-observation.md).
`runtime/menu/channel.h` runs at the forwarding tick hook, before the original
tick, on the first observed engine thread. It validates current-screen identity,
vtable, initialized state, absence of fade/pending/return state and the expected
state generation. It calls the original callback trampoline (Main index 2/4 or
Quick index 3), including the original shared helper. It never writes the
engine's transition fields directly and never passes engine pointers to Qt.
LastError and the original tick return value remain preserved.

Observation hooks still require successful bounded log initialization and
verify original callback bytes/table entries before installation. The launcher
checks complete executable/DLL hashes. Code staging remains scripted and confined
to a disposable installation. `original/` is never written.

## Wire protocol

`protocols/include/mnm/menu_v1.h` defines a 128-byte little-endian file mapping.
Header bytes 0–7 are `MNMMCMD1`; u32 at 8 is version 1; u32 at 12 is size 128.
No C++ object layouts or cross-process pointers appear in the channel.

| Offset | Writer | u32 fields in order |
| --- | --- | --- |
| 16 | Qt host | sequence, alive, heartbeat counter, request ID, semantic action, expected generation |
| 64 | Engine | sequence, generation, screen ID, ready, acknowledged ID, status, engine thread ID |

Each lane has one writer and an independent even/odd seqlock; readers reject
odd/changing snapshots. Unlisted bytes are reserved and initially zero. Screen
IDs are 0 (unavailable), 3 (Main), 22 (Quick Battle). Actions are 1 (Open Quick)
2 (Back), and 3 (Quit on Main). Status is 0 before requests, 1 dispatched, 2 stale generation,
3 unavailable, 4 unsupported action/screen, 5 retired.

Only one request may be outstanding. IDs strictly increase; the adapter consumes
rejected IDs too, so duplicates cannot run a callback again. Generations change
when current object, supported screen or readiness changes. Requests are never
automatically retried. Engine ownership checks remain authoritative if state
changes between a host snapshot and command consumption.

The host updates heartbeat every 50 ms. The adapter permanently retires on
alive=0 or 2000 ms without a heartbeat change. A new heartbeat cannot revive a
retired session. Qt waits up to 120 seconds for initial engine updates after staging, allowing
launcher verification/prefix startup, then uses a 2000 ms update timeout and
a 10-second command/transition timeout. Window discovery starts after staging
and has its own startup allowance. No failure issues an alternate original click.
Header mismatch rejects initialization; runtime corruption disables dispatch.

## Validation

Synthetic and live evidence are recorded separately in the coverage ledger.
Reproduction:

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --target mnm-qt-shell menu-bridge-test --parallel 4
ctest --test-dir working/build/qt-shell -R 'qt-(menu-bridge|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
./tools/test-menu-observer.py --channel
./tools/test-live-menus.py
```

The PE32 fixture executes the original callbacks with the existing helper/tick
stubs. It checks selected dispatch, duplicate/stale/unsupported/unavailable
requests, wrong owner, an unstable host lane, lease expiry and permanent
retirement. Those checks do not establish the real shared helper or fade timing.
The native channel test checks snapshots, outstanding requests, heartbeat field
preservation, stale caller snapshots, malformed headers/state and retirement.

The live test creates an isolated Xvfb display, activates the actual Qt buttons,
waits for engine-confirmed Main/Quick/Main readiness and records the two
acknowledgements and engine thread. The launcher runs for a bounded interval.
Screenshots, report, shell log and artifact hashes stay under
`working/tests/live-menus/`; original observer records and staged hashes stay
under `working/experiments/menu-observer/`. Original-manifest verification
surrounds fixture, staging, launch and live test.

Confirmed validation (2026-10-04 local): all five targeted Qt checks passed.
The channel fixture passed at `working/tests/menu-observer/run-vi6kb2lu/`;
the observer-only regression fixture passed at
`working/tests/menu-observer/run-fitu9p5x/`. The completed live run is
`working/tests/live-menus/run-phq_roev/`, staged from
`working/experiments/menu-observer/run-00ic5r8c/`. It recorded Main generation 1
(ack 0), Quick generation 4 (ack 1), and Main generation 7 (ack 2), all on
engine thread 412 in that launch. The observation log contains exactly Main
index 2 and Quick index 3 callback completions. After Use original menus, the
host alive flag was 0 and the adapter status was permanently retired (5),
with acknowledgement 2. The original game stayed alive for the 45-second smoke.
All surrounding original-manifest checks verified 2927 files.

Confidence is high for this selected live callback/presentation path and
synthetic guards. Initial integration runs exposed startup timeout and window
discovery allowances that incorrectly counted staging/preflight; these were
corrected before the completed run. The initial timeout evidence remains in
`run-ujd9kpxu`; the native round trip with failed viewport discovery remains
in `run-3x8mrmht`. Neither failed run is counted as a complete acceptance pass.
Qt QWidget captures omit foreign Wine pixels. A further live run at
`working/tests/live-menus/run-vqk_xyt5/`, staged from
`working/experiments/menu-observer/run-l4ma6u4d/`, uses direct X11 capture for
fallback. Its `qt-original-fallback.png` was inspected and shows the original
Main menu inside the shell, with its original controls available. Native Quick
and Main captures from the preceding run were also inspected.

Remaining boundaries include hardware focus/key behavior, shutdown during preparation/fallback and longer
sessions, all other actions and screens, native data binding and OpenGL viewport
integration. Original drawing suppression requires a later separate validation.

Startup feedback correction: a user launch appeared idle while repeated original
manifest checks and preflight ran. The shell now immediately displays verification
and staging status in its viewport, then launch/Wine startup status. Failures before
viewport attachment also appear there. This is presentation feedback only; the
required verification and launch sequence are unchanged.

## Native exit extension

Main Quit uses original `0x4a75c0` index 4, including the shared fade helper.
On the non-demo build the callback sets owner+0x43 to 1 to return out of Main;
the existing demo flag branch is retained. No process kill or direct engine
state write is used. Closing the shell from Quick Battle queues original Cancel,
waits for engine-confirmed ready Main, then requests Quit. Once Quit is
acknowledged, the host retires the channel and waits for normal launcher
completion before closing. Repeated close requests do not dispatch again.
Unsupported/fallback sessions expose the original viewport and explain where
to quit; shutdown during preparation remains outside this bounded extension.

The original-bytecode fixture adds Main Quit before and after hook installation
and command guards for Quit on Main versus rejection on Quick. Live reproduction:
`./tools/test-live-menus.py --exit-from main` (native button) and
`./tools/test-live-menus.py --exit-from quick` (window close).

Exit evidence: the 16-case original-bytecode/channel fixture passed at
`working/tests/menu-observer/run-zxzadgn0/`. Window close from Quick passed at
`working/tests/live-menus/run-2yv2yd9t/`, using disposable experiment
`working/experiments/menu-observer/run-ek5awdc1/`: native Main/Quick/Cancel
snapshots acknowledged requests 1/2 on one observed engine thread, followed by
original Main index 4, acknowledged request 3, retired host lane, and normal
launcher status 0. All 2927 immutable input files verified before and after.
The first exit fixture run executed successfully but its Python expected trace
still omitted the added Quit case; it failed validation at
`working/tests/menu-observer/run-a03deogp/`. The corrected validator passed the
fresh run above. A sandboxed attempt could not bind Wine's local socket and
provided no callback validation.

The native Main Quit button also passed at
`working/tests/live-menus/run-ocq72p31/`, experiment
`working/experiments/menu-observer/run-ev6nd6__/`, with the same callback indices
2 → 3 → 4 and launcher status 0. The five targeted Qt checks passed after the
exit changes. Final shell closure waits for launcher's post-run manifest
verification, so it can lag behind the game window disappearing.
