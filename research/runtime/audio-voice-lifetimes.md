# Voice retirement and wrapper ownership

Reviewed 2026-10-04. Static No-CD reconstruction and synthetic native validation;
not an injected manager replacement or a live playback claim.

## Static evidence and contracts

`python3 tools/export-audio-retirement.py` verifies the original manifest before
and after, checks executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
and exports instruction ranges, direct callers and artifact hashes. Evidence:
`working/decompiled/audio-support-jhnwov_f/manifest.json`. Both manifest checks
passed for all 2927 original files. No executable was changed or launched.

| Address | Recovered behavior |
| --- | --- |
| `0x004de090` through `0x004de1b9` | Retire source and optionally duplicates, gated by manager +0x1c and +0x18. Stop/reset and optional scheduler clearing; no COM Release or heap free. |
| `0x00571d20` through `0x00571d7a` | Retire a scheduler record into the reusable tail of its circular ring, clear external output and record payload. No deallocation or reduction of ring membership. |
| `0x0056df00` through `0x0056df5b` | Recursively destroy descendants first, release own buffer and reset its fields, optionally free own wrapper when flags bit zero is set. |
| `0x005700e0` through `0x00570134` | Recursively release descendants and free them, then release/reset own contents; caller retains the source allocation. |
| `0x00572150` through `0x00572176` | Reset buffer=0, source index=-1, +8=0, +0xc=2, both volume fields=99, duplicate=0, duration=0. +0x18/+0x1c links untouched. |

Retirement queries a nonnull source buffer. Query failure or playing bit attempts
Stop. If scheduler clearing is requested, it occurs immediately after Stop,
even when Stop fails. Only zero Stop result proceeds to zero-position reset.
Any nonzero Stop/reset result returns before visiting duplicates. Idle or absent
source skips these operations but can still visit duplicates.

Each duplicate is called with include-duplicates false and scheduler clearing
true, regardless of the outer clear flag. When the outer flag is true, the
caller also clears that duplicate's scheduler entry after the child returns,
even on failure. This second notification is preserved. Failure prevents later
duplicates from being visited. Successful idle source receives no scheduler
notification; idle children can receive the outer notification. Retirement never
releases sample ownership or disconnects the duplicate chain.

The scheduler helper advances the head when retiring the head. A middle record
is detached and inserted before the head (the tail); retiring the current tail
leaves its position unchanged. A one-node ring remains circular. It clears the
output slot and resets deadline/voice/output to zero, volume to -5000, x/y to -1.
Raw engine links are not host addresses: `ScheduleNode` uses explicit host links
as authoritative topology; `Schedule32` contains the payload snapshot. No game
pointer is dereferenced by the reconstruction.

Destruction is a different operation. Descendants release/free in deepest-first
order, before the source buffer is released. COM Release's returned reference
count is ignored by these blocks. Reset occurs only if the wrapper had a buffer;
a bufferless wrapper can retain other metadata even after children are removed.
The deleting destructor uses only bit zero of its argument. Repeated contents
cleanup of a retained, cleared source performs no additional releases. Calling
the original destructor again on a freed allocation is invalid.

## Implementation and boundaries

`reconstruction/audio/voice_lifetime.*` models these blocks with host annotations
and a LifetimeBackend for COM controls, scheduler notifications, buffer Release
and logical wrapper free. Acyclic uniquely owned duplicate chains and valid ring
membership are preconditions. Fixture objects remain readable after logical
free for assertions; this is not the original allocator's lifetime behavior.

The original scheduler lookup passes null to `0x00571d20` when no matching
record is found, and the helper dereferences it. The model does not invent
missing-record recovery. A backend must supply valid mappings for engine paths
that request scheduler clearing. Duplicate notifications and shared original
scratch globals make broader reentrancy/ownership assumptions worth further
audit; synthetic callbacks do not prove those assumptions in live execution.

Confidence is high for the exported branch ordering, reset fields and ring
transformations. Full source-list ownership (+0x18/+0x1c), manager allocation,
configuration/failure unwind, primary COM lifetime, full shutdown integration
and live thread/timing behavior remain separate. The manager contract still
accepts an explicit release inventory; its retirement callback can now use this
reconstruction with an appropriate wrapper/scheduler mapping.

## Reproduce and validation

```bash
python3 tools/test-audio-lifetimes.py
python3 tools/export-audio-retirement.py # static game input; manifest guarded
```

The first command builds the native reconstruction and runs all audio/asset
CTests using fixtures; it never reads game assets or launches the game.
Evidence: `working/tests/audio-lifetimes/run-04bmjqjl/report.json` and its logs.
All 13 CTests passed. The lifetime fixture covers 432 enabled/buffer/status/
failure/clear combinations, 204 ring size/head/target cases, duplicate failure
short-circuiting, repeated retained-source cleanup, postorder Release/free,
deleting flags and preservation of metadata for bufferless sources.

A real native Device fixture verifies that retiring a source leaves its
independent duplicate playing, source Release leaves the duplicate sample data
usable with exact 10000-valued PCM, and recursive contents cleanup releases the
last owner. `working/build/audio-output/audio-lifetime-sanitize` passed
AddressSanitizer, UndefinedBehaviorSanitizer and LeakSanitizer outside sandbox
tracing. These tests use synthetic samples, no audio device and no Wine game.
No x86 adapter code was changed in this chunk.
