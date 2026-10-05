# Main / Quick Battle engine observation

## Scope and evidence

Build-specific contract for No-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred image base `0x400000`. Addresses below apply only to that build.
Runtime object pointers are captured per call, not assumed stable across launches.

Static evidence: hash-checked disassembly, configuration-string references,
vtable slots, and read-only Ghidra exports from
`working/decompiled/menu-support-yypujxe8/`. Its manifest hashes the artifacts;
Ghidra discovers missing function definitions only within its discarded session.
Confidence is high for the selected callback dispatch and fields; decompiler
names/types elsewhere remain inferred.

Synthetic evidence: `tools/test-menu-observer.py` executes the original callback
bytes and jump tables at their preferred addresses in a freestanding PE32
fixture. Fourteen cases compare unhooked and observed execution, including
invalid actions, `thiscall` stack cleanup, nonvolatile registers, result,
LastError, helper receiver, adjacent field guards and observer install rejection.
Only the common helper and shared tick are stubbed: this does not establish
whole-menu behavior, fade timing, engine service equivalence or live replacement.
The fixture reserves its address range in the executable image before Wine can
map unrelated files there.

Live observation (2026-10-04): the staged original game completed Main →
Quick Battle → Cancel → Main with X11 mouse input in an isolated Xvfb/Wine
session. Evidence is `working/experiments/menu-observer/run-8_8cbhcl/`, with
13 records in `events.bin` and input details in `input-check-retry.json`.
The original game remained alive for the bounded launch. All records use
thread ID 244; pointers below are evidence from this launch only.

| Record | Observed transition |
| --- | --- |
| 1 | Main ID 3, owner/current `0x657ce0`, initialized, depth 2 |
| 2–3 | Main index 2; helper activates fade and sets pending `0x6e0020` |
| 5–7 | Fade clears; Quick becomes current, Main deinitializes, depth 3 |
| 8–9 | Quick ID 22 index 3; fade activates and return flag becomes 1 |
| 11–13 | Fade clears; Quick deinitializes, Main resumes, depth 2 |

This confirms the selected live callbacks and transition ordering with original
logic retained. It does not prove keyboard/focus behavior, other buttons,
long-session equivalence or Qt-driven replacement. The standalone 60-second
idle observation is in `working/experiments/menu-observer/run-7_omi9v1/`.
The successful synthetic report is
`working/tests/menu-observer/run-wqihli7h/report.json`.

## Recovered dispatch

| Screen | ID | Vtable | Initializer | Callback |
| --- | --- | --- | --- | --- |
| Main | 3 | `0x5c63e4` | `0x4a7110` | `0x4a75c0` |
| Quick Battle | 22 | `0x5c641c` | `0x4a7e40` | `0x4a83a0` |

Initializers register callback pointer at callback-object `+4` and owner at
`+8`. Both callbacks receive the owner in ECX and one stack argument (button
index), return EAX zero, and use `ret 4`. Both call helper `0x557510` before
index dispatch. The helper has engine dependencies and starts a fade under
selected conditions; direct state-field writes would bypass those semantics.

| Callback/index | Effect after shared helper |
| --- | --- |
| Main / 2 | Pending screen `0x6e0020` (Quick Battle) |
| Quick / 0 | Pending screen `0x6de750` (Create Multiplayer) |
| Quick / 1 | Pending screen `0x6a5018` (Join Multiplayer) |
| Quick / 2 | Pending screen `0x658970` (Single Player) |
| Quick / 3 | Return flag = 1 |
| Either / out of range | No dispatch effect |

Main indices 0/1/3/4/5 cover campaign start, load, preferences, quit and
command-line handling. They require further bounded validation before adaptation.
Mini Menu now has separate [offline bridge preparation](mini-menu-engine-bridge.md); activation is disabled and runtime behavior remains unvalidated.

Observed object fields are unaligned: vtable `+0`, ID `+4`, initialized `+8`,
fade byte `+0xc`, fade counter dword `+0xd`, pending screen pointer `+0x33`,
return flag dword `+0x43`. Never cast these to a native host object layout.
Shared current-screen pointer is `0x6f34e0`, depth is `0x6f349c`, stack starts
at `0x6f34a0`. Controller push/pop/resume are `0x557040` / `0x557130` /
`0x5571e0`. Both screen vtable slots `+0x10` point to shared tick `0x5595d0`.
That tick gates pending transitions/return on fade completion. Main entry/exit
are `0x4a6b70` / `0x4a6e20`; Quick entry/exit `0x4a7a80` / `0x4a7b80`.
Quick exit also resets the realm string to Celtic.

## Observation hook and wire format

`runtime/menu/observer.c` forwards every original call. It issues no menu
commands and suppresses no original drawing. Installation checks the image
base, callback prologues and both original tick slots before changing memory.
Staging and launching additionally verify full executable/DLL hashes. Main's
seven-byte prologue has no relative instruction; Quick's eight-byte prologue
includes a relative call which the trampoline relocates. All write permissions
are acquired before patching; hooks install during process DLL initialization.
The DLL must remain loaded for the process lifetime.

`MNM_MENU_OBSERVE` opts in and specifies a new log path. Existing files are
refused. Failure to create/write the header leaves original entries unmodified.
Logs allow concurrent readers, retain at most 256 records, serialize writes,
and skip unchanged idle tick states (fade-counter-only changes are ignored).
Original LastError is preserved around instrumentation. Logging failure stops
logging while calls still forward. This bounded log is not a continuous trace.

Header: 16 bytes, ASCII `MNMMENU1`, little-endian version u32 = 1, record size
u32 = 64. Each record contains sixteen little-endian u32 values, in order:

`sequence, event, thread, menu_id, object, vtable, initialized, next_screen,
returning, argument, result, active_screen, depth, last_error, fade_active,
fade_counter`.

Events: 1 tick before; 2 callback before; 3 callback after; 4 tick after.
Sequence starts at 1. Object/current-screen pointers are diagnostic PE32 values,
not pointers that Qt may dereference. The reader rejects invalid headers,
misaligned records, excess records and sequence gaps.

## Reproduction and next boundary

```bash
./tools/export-menu-support.py --project working/decompiled/nocd-nn0_667n/project
./tools/test-menu-observer.py
./tools/prepare-menu-observer.py
./tools/run-menu-observer.py working/experiments/menu-observer/run-EXAMPLE --seconds 60
./tools/read-menu-observation.py working/experiments/menu-observer/run-EXAMPLE/events.bin
```

Use the fresh directory printed by preparation. Preparation copies the working
No-CD installation and adds only a DLL import to that disposable executable;
original callback bytes remain identical on disk. It disables intro movies/CD
music only in the copy. Launch uses `tools/run-game.sh` and a dedicated prefix.
Original manifest verification surrounds export, fixture, staging and launch.

For live validation: capture Main idle, choose Quick Battle, wait for Quick
idle, choose Cancel, and confirm Main resumes. Compare callback indices, owner
and current-screen identity, engine thread, fade completion, stack depth and
transition ordering. Test focus/keyboard as well as mouse separately. Do not
start multiplayer or gameplay merely to gather this first menu trace.

Only after that evidence should an engine-thread command adapter invoke the
original callbacks with validated active-screen ownership. Add a versioned
semantic command channel with stale-screen/duplicate rejection outside widgets;
then connect `MainMenuWidget` and `QuickBattleMenuWidget`. Replacing original
menu drawing and validating complete transitions remain later milestones.

The subsequent opt-in Main/Quick command adapter and Qt integration are described
in [menu-action-bridge.md](menu-action-bridge.md). Observation-only staging remains
the default; `prepare-menu-observer.py --actions` explicitly enables action-capable
staging, and launch requires the separate `--menu-channel` option.
