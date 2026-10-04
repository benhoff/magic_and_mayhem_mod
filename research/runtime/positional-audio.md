# Positional audio arithmetic and voice updates

Reviewed 2026-10-04. Selected static arithmetic and update contracts reconstructed;
synthetic native fixtures only, no injected geometry or live playback claim.

## Evidence

`python3 tools/export-positional-audio.py` exports hash-guarded No-CD assembly,
orientation jump tables and floating constants. Original-manifest verification
passed before and after for all 2927 original files. Evidence:
`working/decompiled/audio-support-s5lce_d9/`, including positional controls,
voice update, wrapped differences, approximate distance, float-to-integer helper
and `positional-constants.json`. Source SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
No executable was modified or launched.

| Block | Confirmed contract |
| --- | --- |
| `0x005715b0`, orientation table `0x00571920` | Listener offsets for orientations 0..3: (-6,-6), (+6,-6), (+6,+6), (-6,+6). |
| `0x0040e4e0`, `0x0040eb70` | Source minus biased listener, modulo map extent, choose shortest signed delta; exact half-extent tie stays positive. |
| `0x004eac10`, called with third axis zero | Weighted absolute distance 2*max(abs(dx),abs(dy)) + min(abs(dx),abs(dy)); positional caller halves it with truncation. |
| `0x00571782` onward | Above range, volume=-5000 and pan output untouched. Otherwise volume=((range-distance)*5000)/range -5000 using integer division. |
| `0x005717ae` onward | Optional signed map-byte attenuation, constants 1.0 and 5000.0. |
| `0x0057183d` through `0x00571888` | Lateral projection: (dx-dy)/2, (dx+dy)/2, -(dx-dy)/2, -(dx+dy)/2 for orientations 0..3. Halves truncate toward zero. |
| `0x0057189f` onward | Pan=3333*lateral/width within width, saturating to +/-3333 at/beyond width. |
| `0x00571460` | Existing positional voice update; cutoff retirement, volume-cache writes and scheduler notification, then pan. |

The signed byte is loaded from map-backed storage indexed by coordinates and
an additional argument shifted right once. Its physical meaning is not proved
here; the model calls it mapByte, not terrain height or occlusion. If enabled
and byte < descriptor threshold (+0x18), adjusted volume is:

```
trunc((1 - (byte-threshold)/(-127-threshold)) * (volume+5000) - 5000)
```

Division in that branch is floating-point. `0x0059bee0` temporarily sets x87
rounding to truncation toward zero, converts to a 64-bit integer and restores
the control word. The recovered model uses long double arithmetic and bounded
Int32 conversion; ambient x87 precision/rounding and exact floating boundary
compatibility remain unverified. Tests cover signed byte bypass, partial
attenuation, -127 cutoff and invalid zero divisor.

## Update behavior

If the world-present gate is zero, return zero without touching the voice slot.
Otherwise a valid active slot is required. At volume <= -5000, query status only
when manager initialized/active gates and buffer are present. Failed status or
playing bit attempts Stop, then clears the scheduler entry even if Stop failed.
Only successful Stop attempts zero reset. Clear caller slot regardless of gates
or control errors; return Stop/reset error when present. Idle status skips
scheduler clearing but still clears caller slot. Samples are not released.

For audible volume changes, requested and cached volume fields are written
before calling SetVolume. Only successful SetVolume notifies scheduler ordering.
Pan is attempted whenever manager gates are enabled, including after a failed
volume call. Equal cached volume suppresses volume/notification, not pan.
The audible update returns zero even if SetVolume/SetPan failed. The original
writes separate scratch globals for these results; the model does not emulate
shared scratch storage or reentrant callback effects.

## Implementation boundaries

`reconstruction/audio/positional_audio.*` receives decoded source coordinates,
map extents, descriptor range/width, orientation and optional signed map byte.
The listener input is the map-coordinate output of camera projection
`0x004f7d00`, before the +/-6 offset. Full screen-to-map camera projection and
its helper dependencies, static lazy wrapper allocation, map-byte lookup and
ownership are not reconstructed here. An adapter must supply those inputs.

Valid orientations 0..3, positive extents/range/pan width and nonoverflow signed
arithmetic are explicit preconditions. Invalid inputs throw; this does not claim
to reproduce original divide faults, overflow, or stale global state for invalid
orientations. PanWritten records the original untouched-output boundary for
inaudible returns. Native services do not depend on this model; recovered
contracts consume native backend interfaces. No balance constants were changed.
Confidence is high for the listed static branches and valid integer arithmetic,
conditional for full camera/map integration and floating edge equivalence.

## Reproduce and validation

```bash
python3 tools/test-positional-audio.py
python3 tools/export-positional-audio.py # consumes guarded static executable
```

The fixture runner reads no game assets, opens no audio device and launches no
Wine/game. Evidence: `working/tests/positional-audio/run-ywwv4jpi/report.json`.
All 16 combined audio/asset CTests passed. Fixtures check 2312 coordinate/range/
orientation cases against independent integer expectations, 7665 wrap cases,
432 gate/cache/failure update combinations, idle/failed status, signed map bytes
and cutoff boundaries. Native PCM verifies attenuation -1000 and pan +3333
produce stereo samples {68,3162} from input 10000, then out-of-range retirement
produces silence, clears caller slot and retains the sample buffer.

`working/build/audio-output/audio-positional-sanitize` passed AddressSanitizer,
UndefinedBehaviorSanitizer and LeakSanitizer outside sandbox tracing. Runtime
bridge code was not changed. Live audible/timing behavior and geometry input
mapping still require separate evidence.
