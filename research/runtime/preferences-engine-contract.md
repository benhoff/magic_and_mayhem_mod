# Preferences engine contract recovery

Recovered 2026-10-05 for the working no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone is read-only static recovery plus isolated original-bytecode
execution. It does not enable live Qt Preferences or establish device/display,
file persistence, pause or live caller equivalence. Subsequent Main integration is
tracked separately in the [V6 bridge](preferences-engine-bridge.md).

## Identity, entry and return

The constructor `0x004a8560` installs vtable `0x005c648c` and screen ID 10.
Startup `0x004bf370` constructs the singleton at `0x006a4948`.
The settings receiver is a separate object at `0x006de6c8`, constructed by
`0x004c1b70`; its filename pointer is `CFG\prefs.cfg` (string `0x005e194c`).
Addresses are specific to this pinned executable. Heap control addresses must
be read afresh; do not publish them or assume they survive screen teardown.

| Preferences vtable slot | Function | Recovered role |
| --- | --- | --- |
| +0x00 | `0x004a8af0` | Enter: capture current device audio levels, then initialize |
| +0x04 | `0x004a8b60` | Leave: destroy screen resources, rebuild display if requested |
| +0x08 | `0x004a8be0` | Suspend through leave |
| +0x0c | `0x004a8bd0` | Resume: initialize if not initialized |
| +0x10 | `0x005595d0` | Original common screen tick and stack transitions |
| +0x14 | `0x00559390` | Original common input handling |
| +0x18 | `0x004a9b00` | Update button/slider transition state |
| +0x1c | `0x004a8bf0` | Load layout |
| +0x20 | `0x004a8d00` | Release initialized controls/resources |
| +0x24 | `0x004a8e40` | Build controls, seed values, establish availability |
| +0x28 | `0x004a9670` | Original drawing |
| +0x2c | `0x00475c40` | Empty original per-tick method |

Main callback `0x004a75c0`, local index 3, stores Preferences in parent +0x33
(at `0x004a76d5`). Battle Mini callback `0x004b23f0`, local index 0, also
stores Preferences in +0x33 (at `0x004b2461`) and sets its return flag.
The latter must not be treated as proof of a Qt Mini-to-Preferences round trip.
Realm-view caller behavior and all campaign Mini variants need separate recovery.

The common tick pushes pending +0x33 only outside a fade, suspends the previous
owner, and invokes the child's enter method. On a return flag +0x43 outside a
fade it calls leave, pops the stack and resumes the actual previous owner.
The stack uses depth `0x006f349c`, entries `0x006f34a0` and current owner
`0x006f34e0`. Valid push depth is below 15; return requires depth at least 1.
Do not infer a fixed destination from a button acknowledgement.

## Object and control state

Fields are packed and frequently unaligned; future adapters need bounded reads
and `memcpy`-style scalar access, not host C++ object-layout casts.

| Screen offset | Meaning |
| --- | --- |
| +0x04 / +0x08 | Screen ID / initialized state |
| +0x0c / +0x0d | Fade active byte / fade counter |
| +0x11 | Fade enabled byte |
| +0x13 | Original background surface |
| +0x33 / +0x37 / +0x3b / +0x43 | Pending push / replacement / modal / return |
| +0x47 | Background reload callback; `0x004a97d0` |
| +0x4b | Slider callback object: fn `0x004a9700`, receiver at callback +8 |
| +0x4f | Eleven text controls, stride 0x29 |
| +0x53 | Two sliders, stride 0x86; current value at slider +0x61 |
| +0x57 | Twelve radio controls, stride 0x7e; local index at +0x2d |
| +0x5b | OK/Cancel text controls, stride 0x59 |
| +0x5f | Button callback object: fn `0x004a9840`, receiver at callback +8 |
| +0x63 | Five groups, stride 0x10: pointer list, selected ordinal, count, capacity |
| +0x67 | Resolution changed byte, applied on leave |
| +0x68 / +0x6c | Entry snapshots of effects attenuation / music level |

Callback objects use vtable `0x005c64bc`, function at +4 and receiver at +8.
The original radio-group membership table at `0x005c63a4` is
`0,0,1,1,2,2,2,3,3,3,4,4`; capacities at `0x005c6390` are `2,2,3,3,2`.
Group selection follows the list's selected ordinal, then reads the chosen
control's local index. Empty or malformed groups are unsafe to call in the
original; a future adapter must reject them before dispatch.

## Settings mapping and availability

| Choice | Original local indices | Receiver field / absolute global | OK mapping |
| --- | --- | --- | --- |
| High / Low resolution | 0 / 1 | +0x0d / `0x006de6d5` | 1 / 0; change sets screen +0x67 |
| Full / Cut animation | 2 / 3 | +0x29 / `0x006de6f1` | 0 / 1 |
| Fast / Medium / Slow dialogue | 4 / 5 / 6 | +0x2d / `0x006de6f5` | 0 / 1 / 2 |
| Fast / Medium / Slow game play | 7 / 8 / 9 | +0x35 / `0x006de6fd` | 20 / 17 / 14 |
| Border picture On / Off | 10 / 11 | +0x11 / `0x006de6d9` | 1 / 0 |
| Music slider | callback index 0 | +0x41 / `0x006de709` | Slider value directly, 0..15 |
| Effects slider | callback index 1 | +0x45 / `0x006de70d` | Slider value minus 5000 |

The installed layout says -5000..0 for both sliders, but original initialization
**overrides** music to 0..15, step 1, and effects to 2500..5000, step 50.
Thus the original effects UI exposes attenuation -2500..0. These override calls
are at `0x004a9337..0x004a9373`; `0x004cdff0` takes maximum then minimum.
The existing Qt preview's two attenuation sliders cannot be bound directly.
The broader sound backend's accepted attenuation range is a separate contract.

Initialization selects dialogue after clamping its stored value to 0..2.
Game speed uses thresholds: >=20 selects Fast, >=17 selects Medium, otherwise
Slow. OK normalizes that choice to one of 20/17/14 and writes both preference
and timing globals (`0x006dbe99`, `0x006dbe95`), including integer 1000/rate.
This establishes stored timing values, not measured live frame rate or balance.

Music is disabled when `0x00657c74` is zero; effects when `0x006b01b4` is zero.
High resolution is disabled if `0x005e12e0` is zero or `0x006e1ed0==2`;
Low is disabled if `0x005e12dc` is zero. A nonzero byte `0x006e2030` disables
music, High and Full animation at the end of initialization. The meaning of
that mode byte is not established here; publish observed availability instead
of assigning it a guessed mode name. Final OK audio application checks the two
device globals; the raw slider callbacks themselves do not enforce those gates.

## Editing, OK, Cancel and persistence

Entry `0x004a8af0` runs only while +8 is zero. It queries effects through
`0x0056fd10` on receiver `0x006b0198`, music through `0x004818b0` on
`0x00657948`, and captures globals in screen +0x68/+0x6c before initialization.
A second enter while initialized does not replace those snapshots.

Slider callback `0x004a9700` applies audio immediately. Music calls
`0x00482190`; positive levels inspect MCI state through `mciSendCommandA`
and may restart music through `0x004819b0(1)`. Effects calls
`0x0056fd30` and requests preview sample 509 through `0x0056f000`.
The callback returns zero and consumes one stack argument (`ret 4`).

Button callback `0x004a9840`, index 0 (OK), reads radio groups, applies settings,
updates the border descriptor through `0x004f7970`, applies available audio
levels, then calls the original settings writer `0x0054c890`. Index 1 (Cancel)
restores both entry audio snapshots, including device service calls, and skips
the writer and radio/timing changes. Unknown button/slider indices are inert.
Both accepted buttons invoke `0x00557510`, then set return +0x43 to 1;
that helper starts the original transition, so completion is asynchronous.

Resolution rebuilding occurs on leave through `0x004a88f0`, with gameplay
resource work conditional on gameplay initialization at `0x006cbb80`.
It recreates display resources, window dimensions and fonts; this has static
evidence only and must remain original work during the next integration.

The writer sets normal file attributes, builds an absolute path from the current
directory and receiver filename, and calls `WritePrivateProfileStringA` for
multiple sections. It writes more settings than the visible menu. Existing
unexposed values must be preserved. Notably it writes `SoundEnabled=TRUE` and
`CDMusicEnabled=TRUE` unconditionally. Its API return values are ignored; OK
can return normally even when persistence fails. Do not advertise a successful
save merely because the original callback was acknowledged. See
[on-disk key mappings](../formats/preferences-config.md).

## Evidence, targeted tests and next boundary

Selected read-only disassembly, Ghidra and artifact hashes:
`working/decompiled/preferences-support-ahcm7xw8`. The earlier export
`preferences-support-yrw3b18n` also established the callback/state findings.
Both use the pinned executable and a read-only analyzed project, with changes
discarded. Reproduce with `python3 tools/export-preferences-support.py`.

`python3 tools/test-preferences-contract.py` compiles a private 32-bit Linux
oracle and executes the original enter, slider, button and writer instructions.
Device, MCI, formatting, transition and filesystem dependencies are replaced
only in the private mapping after expected-byte checks. The original settings
writer runs against an API recorder; no real preference file is written.
Initialization is stubbed: these tests do not execute the full control builder.

Final evidence: `working/tests/preferences-contract/run-6qar_1u_/report.json`.
The matrix covers 216 valid radio/audio OK combinations, 12 preview-and-Cancel
round trips (including unavailable devices), four final audio-availability
combinations, eight MCI status/device branches, zero music, unknown indices and
profile-write failure. Snapshot preservation on repeated enter is checked in
the OK matrix. `qt-preferences` also passes with synthetic inputs.

The first sandboxed attempt `run-aqnh9u6_` was blocked by SIGSYS on 32-bit
execution; the same oracle passed outside that restriction. `run-_vvi66u9`
passed the initial matrix; the final run adds failure and Cancel/device cases.
Immutable verification checks all 2927 original files before/after exports and
oracle runs. No game or manual testing was required for this milestone.

Confidence is high for pinned static fields and isolated callback effects.
Availability/control initialization and actual display rebuild remain static;
actual audio, filesystem durability and complete caller transitions remain
unvalidated by this recovery milestone. The subsequent guarded Main-to-Preferences
state/action publication, semantic conversion and automated caller checks are
recorded independently in the [V6 bridge](preferences-engine-bridge.md).
