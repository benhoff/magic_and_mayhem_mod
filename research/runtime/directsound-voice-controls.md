# DirectSound voice control contracts

## Evidence and scope

Pinned PE32 i386 no-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
image base `0x00400000`. Addresses identify this build only. Object, buffer,
list and output-slot pointers are launch-dependent.

Reproduce selected assembly, direct callers, indirect-call contexts and offline
tests with `python3 tools/test-audio-voices.py`. The exporter verifies the whole
input hash before and after. Method order is cross-checked against installed
`/usr/include/wine/windows/dsound.h`. No game is launched or binary patched.

`reconstruction/audio/voice_contract.*` models selected engine control blocks
through a fakeable backend. It does not implement native playback advancement,
mixing, COM emulation, hooks or output. The Qt asset/WAV pipeline is unchanged.

## Confirmed control paths

| Entry or call | Contract | Confidence |
|---|---|---|
| `0x0049c530`, call `0x0049c54c` | GetStatus +0x24: playing bit 1 or nonzero HRESULT counts as busy | High: branches and SDK order |
| `0x004ff640`, call `0x004ff679` | SetVolume +0x3c: cache before call, optional duplicate propagation | High |
| `0x0056f000`, block `0x0056f24d` | Volume, pan, Play in order; any nonzero result aborts the sequence | High for block; selection/list handling not modeled |
| `0x0056f291`, `0x00572237` | Play +0x30: reserved 0/0, loop boolean becomes flags 0/1 | High |
| `0x0056f870`, call `0x0056f8df` | SetPan +0x40 by sample identifier, then propagate to duplicates | High |
| `0x00571460`, calls `0x00571557`/`0x00571596` | Existing positional voice receives computed volume/pan | High for calls; preceding geometry not reconstructed |
| `0x005722b0`, call `0x005722d1` | SetPan with optional wrapper +0x20 duplicate-chain propagation | High |
| `0x00571c30`, `0x00571ec0`, `0x0056fd90` | Retirement query, conditional Stop/zero reset, scheduler clearing | High for blocks; list reorder not modeled |
| `0x00571be0` | One-shot deadline = now + duration; loops use 0xffffffff | High |
| `0x0058ffb0` | WAV duration = unsigned (dataBytes * 1000) / byte rate; product wraps at 32 bits | High for valid arithmetic, not malformed mmio behavior |

Manager +0x1c and +0x18 gate operations; `enabled` means both are nonzero.
Precise semantic names remain unspecified. Primary looping Play at
`0x0056fd73` sets +0x18 after success; global stop clears it before stopping
the primary at `0x0056fe62`. Primary GetVolume at `0x0056fd27` uses +0x1c only;
it must not be mistaken for a frequency query.

### Busy, stop and reset

Busy returns false immediately for a null source buffer, even when duplicate
checks are requested. Otherwise a failed status query returns true, as does
playing bit 1. Children are queried only after the source is idle; each child
check excludes recursion and the first busy result stops traversal.

The stop block zeroes status scratch before GetStatus. Successful idle status
skips Stop/reset. Query failure or playing bit attempts Stop; only result zero
leads to SetCurrentPosition(0). Positive nonzero HRESULTs fail too. No arbitrary
position was observed at the identified reset sites. Retirement clears records
and external output slots even if Stop/reset fails. `stopAndReset` represents
only the backend block; `clearSchedule` represents the record reset and returns
the external slot address to zero without dereferencing a game pointer.

### Volume and pan

Wrapper +0x10 is requested volume and +0x14 cached volume. Equality with +0x14
suppresses all work, including duplicate updates and scheduler notification.
Both caches are written BEFORE manager gates/SetVolume. A failed SetVolume thus
leaves the new cache: repeating the value succeeds without retrying the call.

After success, an optional callback to `0x00571d80` updates scheduler volume
ordering. Propagated children are called with notification enabled regardless
of the parent's flag. If the parent's notification flag is also set, it calls
the child's notification AGAIN after success. An equal child cache suppresses
its inner work but not the parent's extra notification. Tests preserve this
unusual order. The sorted pointer-list implementation is outside the model.

Pan has no equivalent cache. Repeating a value still calls SetPan; propagation
stops at the first nonzero result. Annotation duplicate pointers must form an
acyclic host chain; they are not stable game addresses or native voice state.

### Positional pan and scheduler

The final pan stage `0x0057189f..0x00571905` truncates signed
`lateral * 3333 / width` toward zero inside the width, and saturates to ±3333
at/beyond it. The annotation requires positive width and rejects interior
products outside signed 32-bit range rather than inventing overflow behavior.
Preceding camera rotation, map lookup, distance and terrain attenuation in
`0x005715b0` are not reconstructed; an adapter can consume engine-computed values.

| Scheduler offset | Field |
|---|---|
| +0x00/+0x04 | Next/previous list pointers |
| +0x08 | Unsigned millisecond deadline, loop sentinel 0xffffffff |
| +0x0c | Cached volume/order value |
| +0x10 | Voice wrapper pointer |
| +0x14 | Optional external output-slot pointer |
| +0x18/+0x1c | Stored positional coordinates |

The 32-byte annotation uses explicit 32-bit address fields on a 64-bit host.
Clearing sets deadline/voice/output to zero, volume to -5000, coordinates to -1;
links are retained for a separate list operation. Expiration compares unsigned
`now >= deadline`, excluding 0xffffffff. It is not wrap-safe: early expiration
near clock wrap and one-shot sentinel collisions are preserved, not corrected.
Scheduler deadlines are not actual device playback cursors.

## Frequency/cursor audit and validation

`voice-call-audit.json` retains all indirect calls in `0x0056df60..0x00572320`
and the external busy/volume wrappers. No register-plus-0x44 call (buffer
SetFrequency slot) occurs there. The only register-plus-0x10 call is
`0x0056fefb`, device GetCaps, not buffer GetCurrentPosition. Register-only/import
calls remain in the evidence for review. Slot offsets alone do not identify an
interface. This is bounded static evidence, NOT whole-program absence proof,
runtime coverage or resolution of every indirect target.

Initial native playback therefore requires play/stop, loops, zero reset,
busy/status, volume and pan. Variable pitch and arbitrary cursor queries remain
conditional. Creation flags allowing frequency changes do not prove their use.

Tests cover status errors/bits, disabled gates, call order, positive/negative
failures, cache-before-failure and suppressed retries, duplicate propagation and
notifications, loop flags, deadlines/equality/wrap, duration truncation/wrap,
scheduler reset and signed pan truncation/saturation. Existing audio buffer and
Qt asset-input CTests run alongside them. Confidence is high for these selected
assembly-backed contracts; live timing, audible parity, full COM references,
selection/eviction lists and complete positional computation remain unvalidated.

Successful evidence: `working/tests/audio-voices/run-jcmi0vl9/report.json` and
`working/decompiled/audio-support-jpj7j8ub/manifest.json`. All six audio/asset
CTest checks passed; original-manifest verification passed before and after
(2927 files). The audit contains 82 selected indirect-call sites. A standalone
voice fixture built with `-fsanitize=address,undefined -fno-omit-frame-pointer`
also passed with leak checking enabled. Its output is
`working/build/audio-voices/audio-voices-sanitize`; it needs no Qt event loop,
game process or audio device. LeakSanitizer was run outside sandbox tracing.

[Independent native voice state](native-audio-voice-state.md) now has separate
fixture evidence. Next: stereo mixing/sample-rate conversion and QAudioSink
output. Recovered-contract tests alone do not establish those native milestones.

Primary controls and selected manager startup/disable/shutdown ordering are now
modeled separately. See [primary audio manager](primary-audio-manager.md).

Selected scheduler selection, eviction, assignment and old-volume ordering now
have [offline models and evidence](audio-voice-scheduler.md). Full manager start
and wrapper-duplicate admission remain outside these models.

Wrapped positional arithmetic, orientation pan, signed map-byte adjustment and
existing voice updates now have [separate offline reconstruction](positional-audio.md).
Camera projection and map lookup ownership remain separate inputs.
