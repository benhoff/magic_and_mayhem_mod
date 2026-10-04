# Version 1 publication and lifecycle contracts

These are toolkit-designed IPC contracts, not recovered original game formats.
The supported baseline is little-endian PE32 x86 and native x86-64 sharing mapped
files. Fixed-width wire fields do not establish portable atomic IPC semantics
on other architectures. Header identity is immutable after initialization.
Qt creates an exclusive, zeroed file per launch; peers validate magic, version
and exact file length before using it. No process pointers are transmitted.

Schema owners `producer` and `consumer` mean the roles below, not necessarily
Qt versus injected code. `creator` initializes identity and reserved bytes.
Schema `header_size` describes the initialization prefix, not the declared-size
field interpretation or the end of every control region (media has responses
at offset 512).

## Frame: injected producer, native consumer

The payload is opaque RGBA8888, top row first, tightly packed. Dimensions are
1..2048 and stride is exactly width * 4. The maximum payload allocation remains
2048 * 2048 * 4; only the current frame's bytes are meaningful.

Capture writers serialize access. An aligned 32-bit sequence is made odd before
modification and even with release ordering after pixel and frame metadata
publication. Readers acquire the sequence, reject odd values, validate bounds,
copy the payload and accept only if the acquired sequence remains unchanged.
Repeated frame counts are ignored. Failed capture does not increment the frame
count; old frames are dropped rather than queued.

Status and startup diagnostic words can change outside the frame sequence.
Do not infer that a status/HRESULT/count snapshot is jointly consistent just
because the frame sequence matches. Preserve the existing per-field ordering
in adapters. Startup status numbers are diagnostics, not command replies.

## Input: native producer, injected consumer

The native producer writes aligned words under an odd/even sequence guard,
using an ordered odd publication and release even publication. Readers acquire,
snapshot and recheck the sequence. Active state requires the matching target,
visible viewport and viewport keyboard focus. Valid coordinates are inside the
1..2048 game image.

The 256 virtual-key words use bit 31 for held state and bits 0..30 for a rising
edge generation modulo 2^31. Auto-repeat does not increment the generation.
Release/focus loss clears held state but retains generations. Each injected
process tracks consumed generations for GetAsyncKeyState independently;
GetKeyState reads held state. Key translation and which keys fall back to
Windows remain adapter responsibilities.

The producer heartbeats nominally every 50 ms. Readers measure sequence changes
with their own monotonic tick counter and reject state after 500 ms without a
change. Initial observation can receive a 500 ms grace period, even for a stale
file. This is neither a durable event log nor a synchronized timestamp lease.
Unavailable, inactive, malformed or stale input falls back to original APIs.
Normal shutdown publishes inactive state.

## Media: injected request producer, native response consumer

Only one producer call may submit/wait at a time. Concurrent calls use legacy
fallback. Request IDs are nonzero and wrap skipping zero. Request metadata/path
use the producer's odd/even request sequence. Response ID/status/accepted ID use
the consumer's separate odd/even response sequence. Each reader acquires and
rechecks the corresponding sequence. The 25 ms heartbeat is independent.

The producer's cancellation ID and sound-cancellation generation are separately
published atomic words, outside the request sequence. They are not guaranteed
to be a consistent snapshot with the request. The consumer must not start a
request it observes as cancelled; active matching playback is stopped.

Paths contain 0..259 declared ASCII bytes in a 260-byte field. Existing producers
reject empty paths for movie/file-sound. Stop-file-sound has zero flags and zero
path length. Readers use the declared bytes; no trailing terminator is required.
Movie flags are zero. File sounds require SND_FILENAME; allowed flags are
0x2001b and looping requires asynchronous playback. Memory/resource/alias sounds
remain legacy operations. Actual asset resolution is adapter policy: trusted
FMV AVI / Sounds WAV roots, case-insensitive unambiguous names, no traversal or
escape through symlinks. Contract adoption must retain those checks.

Responses progress through loading, acceptance and terminal status. The accepted
ID is cleared on loading, set on acceptance, and retained through the terminal
response. It preserves evidence of acceptance if the producer misses a rapid
accepted-to-error transition. Asynchronous file-sound calls return on acceptance;
playback may continue after the submitting call finishes.

The producer polls every 5 ms. More than 5 seconds of unchanged heartbeat,
10 seconds without acceptance, or 10 minutes total waiting publishes cancellation
and ends the wait. Before acceptance, decoder/unsupported errors or timeout
permit legacy fallback. After acceptance those failures must not restart the
same movie through the legacy renderer. Complete returns success;
skipped/stopped/cancelled and sound-busy return failure without replay.
Unsupported legacy sound calls cancel a previous native sound; accepted native
sounds retire legacy asynchronous sounds. Keep these lifecycle decisions in
adapters; shared constants do not implement them.
