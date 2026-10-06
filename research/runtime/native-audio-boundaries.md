# Native PCM format, lifetime and failure boundaries

Reviewed 2026-10-05. This completes a bounded native PCM core contract, separate
from the recovered DirectSound manager, physical Qt output and live replacement.
Evidence ID `NA.core-boundaries-20261005` records synthetic execution; original
comparison and replacement remain `none` for these native policies.

## Supported inputs and output

| Boundary | Supported native policy |
| --- | --- |
| Encoding | PCM tag 1, unsigned 8-bit or signed little-endian 16-bit |
| Channels | One or two, interleaved whole frames |
| Rate | Nonzero u32 with `rate * alignment <= UINT32_MAX`; alignment and byte rate must agree |
| Format extension | Zero extra size; WAV accepts 16-byte or 18+-byte `fmt ` payloads, ignoring trailing payload bytes |
| RIFF | Declared container bounds, even chunk padding, unknown chunks skipped, `fmt `/`data` in either order; bytes after the declared container ignored |
| Rejections | Compressed/float/extensible tags, 24/32-bit or multichannel PCM, nonzero extra size, zero/inconsistent rates, missing/duplicate chunks, empty/partial frames and truncated chunks/padding |
| Mixing | Signed stereo 16-bit at the immutable Device clock; linear interpolation, whole-buffer loops, final-sum clipping and deterministic ID order |
| Bounds | Default 16 MiB per secondary buffer and 128 total identities including primary/duplicates; 65,536 output frames per mix; AssetFile loading separately caps input at 32 MiB |

This scope does not add compressed or streaming decoding. Primary format metadata
is validated separately and does not change the mixer clock. A fresh secondary
buffer now contains encoding-correct silence: 128 for 8-bit and zero for 16-bit.
The earlier zero-filled 8-bit storage decoded to negative full scale before upload.

## Ownership and lifetime

Device is thread confined and owns all identities. Duplicates retain shared
committed PCM and revision but independent controls, cursor and playback state.
Release removes an identity immediately; surviving duplicates retain the samples.
No explicit retired ID is reused. ID/ticket wrap is refused with `limit` after the
last representable value; exhaustion is reviewed code, not executed evidence.

One shared storage permits one writer. Lock returns staging pointers valid until
successful Unlock, writer Release or Device destruction. Invalid Unlock leaves
the lock outstanding for correction. Successful Unlock publishes only the supplied
prefixes of each region and increments revision only when bytes were written.
Mixing sees committed bytes while a write is pending. Releasing a different
alias preserves the lock; releasing the writer cancels unpublished staging.
Stale/copied tickets cannot publish a later transaction. Byte-granular writes
remain intentional, including partial samples; allocation size must contain whole
frames. Destruction retires playing voices and outstanding writes through RAII.
Device must outlive borrowers such as QtOutput/PcmQueue; concurrent access and
using lock pointers after invalidation are outside the supported contract.

## Failure guarantees

Returned `invalid`, `unsupported`, `badFormat`, `busy` or `limit` leaves caller
output arguments and accepted state unchanged. Creation and duplication publish
an ID only after successful map insertion. Failed creation does not consume an
ID. Lock allocates staging before publishing its transaction. Mixing completes
all allocations before advancing any voice and swaps the output only on success.

Heap allocation failures propagate `std::bad_alloc` (and allocator/container
length errors may propagate); they are not converted into a playback HRESULT or
`limit`. The synthetic fixture overrides allocation to fail every allocation in
five operations: secondary creation, primary creation, duplication, lock and mix.
It checks unchanged output/count/cursor/revision and recovery after each failure.
This establishes these core exception boundaries, not process-wide OS exhaustion,
Qt/backend allocation guarantees or manager/COM exception containment.
Parser structural errors throw `std::runtime_error`; AssetFile errors retain
`AssetInputError` diagnostics. Failed invalid sample queries throw rather than
returning a fabricated buffer. Physical output/recovery contracts remain in
[native output](native-audio-output.md) and [recovery](audio-output-recovery.md).

## Scoped execution

Reproduce with `python3 tools/test-native-audio-boundaries.py`. The runner retains
unique build/execution logs, binary and source fingerprints and JSON under
`working/tests/native-audio-boundaries/`; the registered immutable result is
[native-audio-boundaries-20261005.json](native-audio-boundaries-20261005.json).
It executes boundary, buffer, voice-state and mixer fixtures normally and with
ASan/UBSan plus leak detection. The new fixture covers 24 PCM combinations,
format/chunk failures, silence/completion, shared committed mixing, alias/writer
retirement, forged/stale tickets, empty commits, destruction while locked/playing,
limits, zero output clock and five allocation sweeps. Existing independent
frame-step and absolute-time PCM oracles retain their bounded scopes.
No original files, game assets, physical audio device or original code execute in
this runner. Broader standalone audio/asset CTests are recorded separately in the
result. Physical audibility, live transitions/cadence, compressed codecs, threading,
long sessions and original equivalence remain unverified here. Historical evidence
hashes remain unchanged; shared-source changes can leave old results stale.
