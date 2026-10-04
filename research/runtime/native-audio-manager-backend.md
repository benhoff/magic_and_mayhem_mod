# Native backend for recovered audio manager

Offline native integration, not injection or live replacement. Implementation:
[native_manager_backend.hpp](../../reconstruction/audio/native_manager_backend.hpp)
and [native_manager_backend.cpp](../../reconstruction/audio/native_manager_backend.cpp).
The adapter implements the recovered configuration, lifecycle, primary,
manager-control, source-cache and voice-admission backend interfaces. It uses
native asset storage and PCM services without making either service depend on
reconstruction or application widgets. `mnm-audio-native-manager` is a separate
adapter target; the core mixer/storage and profile reader retain their boundaries.

## Connected path

Qt-backed `AssetStore` opens a bounded owned
[profile snapshot](../formats/profile-native-loading.md), resolving the supplied
Windows path prefix and case differences through existing asset policy. Profile
sections/values/integers feed the recovered catalog and scheduler initialization.
The native backend supplies default-device/level-2 cooperative success as host
session policy; no window is configured and no hardware sound device is opened.
Native PCM capabilities select the recovered 22,050-Hz stereo 16-bit primary
request, with format metadata separate from the fixed mixer output clock.

Per-map source setup reads native classes and QFile-backed file sizes, then opens
WAV files through the existing strict loader. Owned decoded PCM feeds the actual
native static-buffer Lock/copy/Unlock sequence. Duration uses the recovered
wrapping DWORD arithmetic. Source selection, group resolution, admission,
overlapping duplicate creation, schedule assignment and duplicate rotation all
execute existing recovered models. Tests verify committed PCM bytes and output
samples, rather than only adapter call order.

The native `Device` has one primary buffer. Because the recovered controller
retains primary storage at shutdown, the adapter reuses its retained native
primary on restart. This is explicit native singleton policy, not a claim that
original COM creation returns the same pointer or reference count. Primary
metadata/global gain and looping status route to native primary controls;
secondary Play/status/Stop/volume/pan/reset route to individual native voices.
Hardware compaction is a successful no-op. Native sample/count limits and HRESULT
mapping remain native policy, separate from recovered original allocation faults.

Clock and random-word callbacks are required inputs. The adapter does not invent
an original RNG or silently derive a simulation clock from audio output frames.
It is synchronous and thread-confined. No pointers or C++ objects cross a wire.

## Ownership and teardown

Native 64-bit buffer identities map to separate nonzero 32-bit host tokens.
Source roots retain model-local identities 1..65,536; duplicate identities start
at 65,537, remain monotonic and reject exhaustion. The supplied `AudioManager`
must outlive the backend and own valid caller slots while scheduled. Source roots
and optional native duplicate storage are owned by `SourcePool`. Rotation can
place a duplicate into the source ring and the former root into its descendant
chain without invalidating either object. Logical free callbacks never delete a
wrapper during an active traversal. Pool teardown releases descendants/buffers
first, then clears owned host nodes; disposed duplicate annotations can also be
collected at an idle boundary. Collection rejects still-linked wrappers.

The controller's shutdown restores gain, retires scheduled voices, clears slots,
releases all root/duplicate PCM buffers and retains primary/scheduler resources
as documented in [aggregate lifecycle](audio-manager-lifecycle.md). The adapter's
device/session destructor owns final native PCM cleanup; this does not add an
original primary Release call. Logical table frees need no second C++ delete
because the controller already owns and clears its host vectors.

Missing profiles/WAVs preserve structured asset diagnostics and return recovered
failure where the selected contract returns one. Malformed WAVs fail through the
strict native reader before upload. Malformed/unsupported profile syntax fails
through native policy. Unsafe original null-create cleanup and invalid scheduler
allocation domains still raise the existing reconstruction boundary errors; no
safe original rollback is fabricated. Missing/oversized file metadata contributes
no pinned size. QFile metadata access can require permissions different from CRT
stat; that is not established compatibility.

## Evidence and remaining boundaries

Run `./tools/test-audio-native-manager.py`. Evidence:
`working/tests/audio-native-manager/run-xauygdrw/report.json` and `sanitizer.log`.
All 29 current audio/assets CTests pass, with address/undefined/leak sanitizer
checks. Tests generate profile and PCM files in a temporary directory; no game,
installed game asset, sound output device or original artifact is used.

The end-to-end fixture verifies:

- Native profile → catalog/primary/scheduler initialization; original primary
  request metadata at 22,050 Hz with the native output clock at 48,000 Hz.
- Permanent profile classification/stat/preload and exact committed WAV PCM.
- Randomized-group admission, looping overlap and distinct duplicate tokens;
  summed stereo output `{8000,8000}` from two 4,000-amplitude voices.
- Master -2,000 attenuation of that sum to `{800,800}`.
- Stopped-root duplicate rotation into the ring, stable ownership, caller-slot
  clearing, source/duplicate buffer disposal, saved-gain restoration and silence.
- Restart without a second native primary, one-shot completion with exact 128
  stereo frames, missing/malformed WAV diagnostics, and destructor schedule cleanup.
- Profile syntax/capacity/integer/snapshot/file-size boundaries described in the
  separate input document.

Confidence is high for these synthetic native outcomes. Profile API equivalence
needs a PE32 comparison fixture and installed input checks. Actual caller order
for map transition while voices are active remains separate; this fixture changes
maps after full shutdown/startup. Live thread/cadence, original engine field
coherence, audible transitions, external COM ownership, full profile encodings
and adapter injection remain unverified. Next offline work is profile-call
comparison before widening accepted inputs or wiring a live manager replacement.
