# Voice admission and duplicate start

Reviewed 2026-10-04. Selected manager start decisions and duplicate handling
reconstructed and tested offline. The source loader, RNG implementation and
live ownership remain separate.

## Evidence

Pinned No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Saved export: `working/decompiled/audio-support-jhnwov_f/manifest.json`,
`manager_voice_paths.asm` and `voice_duplicate.asm`. Their artifact hashes
were checked against the manifest before reconstruction. This reuses previously
guarded static evidence; no executable or original artifact was consumed,
modified or launched in this chunk.

The manager block is `0x0056f000..0x0056f3f9`; duplicate creation/start is
`0x00572180..0x005722a6`. Confidence is high for their inspected comparisons,
call ordering and field/link writes, within the decoded valid-input domain.
Synthetic and native fixture agreement is not live call coverage.

## Admission decisions

1. Manager +0x1c and +0x18 must both be nonzero. Otherwise return zero and
   write manager +0x254 to a supplied output slot. The model supplies this
   disabled-wrapper identity explicitly.
2. Call the recovered scheduler selector with requested volume. No slot means
   return zero without resolving/loading a sound or changing the requested
   output. Retirement/eviction can already have changed existing voices and
   slots before subsequent operations fail; there is no transaction rollback.
3. Search the sorted group-ID table (+0x240/+0x244). A match selects a member
   using `(randomWord & 0x7fffffff) % memberCount`. Search the sorted source-ID
   table (+0x238/+0x23c); a hit retains the resolved sound ID; a miss selects ID `0x19a`. The decoded catalog
   preserves these operations while excluding raw table pointers and bsearch
   implementation details. Groups require a positive member count.
4. Walk the source ring from manager +0x250 through wrapper +0x1c, comparing
   sound ID +4. For the first matching wrapper, GetStatus success without
   flag `0x2` allows reuse. A successful `0x2` (buffer-lost) query writes wrapper +0x0c = 2 and
   triggers the loader; any query error also triggers the loader. Playing flag
   `0x1` alone does not trigger loading. No matching source also calls the loader.
5. After successful loading/reuse, a nonnull buffer is queried again. Playing
   flag `0x1` or a query error selects duplication. Otherwise start the source itself.
   A null buffer bypasses this second query, then enters the original control
   sequence; it is not silently treated as a successful no-op.

`AdmissionBackend::loadSource` delegates the much larger `0x0056f400` routine.
Its selected source-cache decisions and replacement ownership now have separate
[AU14 evidence](audio-source-cache.md); Windows profile and WinMM internals,
source-pool initialization and live integration remain separate. Fixtures
provide an existing ring member; a future loader adapter must establish any
new member's ownership and topology before returning success.

## Direct starts and duplicate ownership

Direct start calls volume, pan and Play in order, stopping at the first error.
An identical cached volume suppresses the volume call and its field writes.
Otherwise requested and cached fields are written **before** the volume call;
they remain written if it fails. Play receives reserved 0/0 and loop flag 0/1.
Only successful start publishes the wrapper to the scheduler and output.
Assignment preserves cached volume, coordinates, caller-slot address and
wrapping clock + duration, or the loop deadline `0xffffffff` without a clock
call. There is no position reset before Play in this inspected start path.

The selected duplicate helper calls DuplicateSoundBuffer first, then allocates
a fresh default-initialized 40-byte wrapper and starts it. Buffer duplication
failure returns immediately without a wrapper, chain change or output write.
After successful duplication, volume/pan/Play failure **does not undo** the
new wrapper: sound ID, +8, +0x0c and duration are copied from the source,
the child is appended to the duplicate chain, source requested volume is
written, and the helper's output is published even on control failure. The
source's cached volume is unchanged. The outer manager returns immediately
on that error, leaving its caller output and scheduler record unpublished and
its source-ring head unpromoted. Lifetime teardown remains responsible for
the retained duplicate. This is original behavior, not a new native leak policy.

After a successful **direct** start with duplicates, the original wrapper
rotates to the end of the duplicate chain. Its first duplicate takes over the
source-ring links and gets the requested-volume field. The scheduler and
caller output still identify the original wrapper that was just played.
Do not substitute the new source-ring head for that playback identity.

The final source-head promotion also runs on direct-start control failure.
For a selected source differing from the current head's sound ID, detach
it and insert it immediately before the old head, then make it head. With
equal indices, only rotate the head pointer. Duplicate and loader failures
return before this step. Host source links represent +0x18/+0x1c separately
from the +0x20 duplicate chain; resetWrapper continues to preserve source links
as the original reset block does.

## Model and boundaries

`reconstruction/audio/voice_admission.*` implements these selected decisions
using `VoiceWrapper`, the existing scheduler and a fakeable backend. Native
audio services remain independent of build-specific reconstruction. Valid
nonempty circular source/schedule rings, acyclic uniquely owned duplicate
chains, sorted unique catalog IDs, representable table indices and coherent
host/output-address annotations are preconditions. Backend callbacks must not
mutate topology, except for an eventual loader's explicitly validated contract.

The backend owns allocated host wrappers and supplies unique session-local
identities. Original scratch globals, raw heap pointers and initialization
state are not shared as wire layouts. The duplicate helper is modeled under
active manager gates. Original malloc failure is followed by an unchecked
null-wrapper write; emulating that fault or inventing an unwind is excluded.
Native allocation errors are outside this recovered valid domain. Live cache
ownership, source allocation and cross-thread/cadence compatibility remain
unverified. No runtime hook logic or mixer gain/selection policy changed.

Admission exposed a native status compatibility defect: looping secondary and
explicitly playing primary buffers returned `0x3`, setting DirectSound's
buffer-lost bit. They now return `0x5` (playing `0x1` plus looping `0x4`).
The constants are confirmed in local `/usr/include/wine/windows/dsound.h`;
[Microsoft's GetStatus contract](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708929%28v%3Dvs.85%29)
distinguishes playing, looping and buffer loss. The PE32 proxy forwards native
status unchanged, so this correction prevents healthy looping voices from
entering the manager's buffer-lost reload path. Host and PE32 status fixtures
now assert `0x5`. This fixes an existing contract rather than adding buffer-loss
simulation; no wire-layout or protocol-version change was needed.

## Validation

Run `python3 tools/test-audio-admission.py`. Evidence:
`working/tests/audio-admission/run-ho_cgq0m/report.json` and logs. All 19
audio/asset CTests passed. Tests cover disabled/no-slot gates, masked group
selection, fallback index, first and second status-query decisions, loader
errors, eviction before failed load, duplicate API and each control failure,
16 manager busy/loop/control-result combinations, cached-volume suppression,
optional output, two/three-member source topology, head versus middle duplicate
rotation, metadata copies, slot identity and wrapping/loop deadlines.

The native Device fixture exercises both one-shot and looping playback. It
admits the original and an overlapping duplicate
against two scheduler records. Synthetic mono 4000 samples produce exact
stereo `[8000,8000]`; stopping only the duplicate leaves `[4000,4000]`.
Committed sample content remains equal and both native buffer identities are
retained. `working/build/audio-output/audio-admission-sanitize` passes address,
undefined-behavior and leak sanitizers outside sandbox tracing. These fixtures
use no game assets, Wine, audio device or live game. The isolated PE32 COM fixture also passed with the corrected primary and
secondary status flags: `working/tests/audio-bridge/run-rphs_f0n/report.json`,
including stale-host retirement. That fixture launches only its synthetic
selftest under Wine, never Chaos.exe. Source-cache loading at `0x0056f400` now has separate
[reconstruction and integration evidence](audio-source-cache.md).

Correction reviewed with AU14 on 2026-10-04: the initial fixture/model returned
source-table ordinals after ID lookup. `admissionSoundId` now preserves the
resolved ID, matching the caller register and wrapper +4; class metadata uses
the separate ordinal. Nonordinal ID and full loader-to-admission fixtures pass
in the AU14 evidence report. The older AU13 report predates this correction.
