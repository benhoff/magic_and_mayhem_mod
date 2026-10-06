# Mapped ring reader and cooperative producer queue

2026-10-06. `NR.command-ring-adapter` connects the [v2 byte ring](../formats/render-command-ring-v2.md)
to the production native channel reader and PE32 owned-session publisher.
The existing v1 channel is still supported. Both versions retain the bounded
command archive/decoder and original rendering. No original-driver equivalence
or live replacement is claimed.

The native reader negotiates by exact file size and identity, binds one v2 reader,
copies at most64KiB per poll and acknowledges that private copy. The existing
native decoder and GPU consumer keep their surface history and execute complete
commands. The native creation API accepts explicit version2; default remainsv1.
Invalid identity, reserved fields, ownership/counters and terminal state refuse
through the ring primitive. Existing v1 validation remains active.

The producer claims a fresh v2 channel and allocates one32MiB private byte queue.
Admitted command header/fields/pixels are copied completely into owned queue
storage while the existing tracker serializes producers. Published queue bytes
are immutable until consumed. Allocation/admission failure refuses; it cannot
silently fall back to an apparently complete native session. This32MiB transport
storage is a separate native policy budget in addition to the unchanged64MiB
owned pixel storage and1MiB mapped ring, not an increase to those budgets.

After tracker release, the calling thread tries a separate nonblocking drain
guard and publishes at most1MiB in64KiB chunks. FULL returns immediately and
preserves the exact queued bytes for later callbacks. Queue consumption follows
successful ring publication; no original borrowed pixels/pointers survive the
hook. GetLastError is preserved. Concurrent drain attempts return immediately;
producer and consumer cursors use release/acquire ownership. No peer waits or
new worker threads are introduced.

END requests native completion only after every queued byte has published.
Cancellation, queue overflow, invalid wire and allocation failure refuse. The first
queued failure reason is sticky; a later generic failure cannot replace it.
Detach makes one bounded best-effort drain and refuses interrupted pending work;
it never waits for a reader under the loader lock. A busy drain retains storage
until process cleanup rather than freeing memory another drain could be using.
Dynamic manual unloading of an installed hook remains unsupported; this does not
add hook teardown/uninstallation. A stream cannot reopen after completion.

Progress is cooperative: without further hooked callbacks, queued work can stay
pending after the reader makes space. Dedicated idle retry scheduling and an
explicit lifecycle shutdown path outside DllMain remain pending. This increment
proves bounded active-callback transport; it is not sustained idle scheduling.
The archive still stops at64MiB,4062 operational/4096 total records, and the
sequence target/operation ceilings remain unchanged.

## Validation protocol

```sh
cmake --build working/build/render-ring --target live-render-channel-test
xvfb-run -a python3 tools/test-render-session-sequence.py \
  working/build/render-ring --wire-version 2
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/test-live-render-game.py \
  working/build/render-ring --presentations 3 --wire-version 2
```

`python3 tools/test-render-command-queue.py` runs native and ASan/UBSan tests which execute the actual C channel/queue with mapped-file
and heap API fakes, preserving11 legacy v1 cases and testing delayed-reader
2MiB+123-byte completion, owned-copy poisoning, ring FULL retry, queue overflow,
cancellation, interrupted close, drain exclusion and forced allocation failure.
Seven queue cases also transfer35,653,675 bytes across the32MiB private queue
boundary and preserve the first overflow reason after a later invalid request.
The Qt live-channel fixture adds version2 creation and independent complete
native display comparisons under1-byte,7-byte and64KiB fragments. Native reader
and original PE32 hooks run the existing eight sequence cases onv2; independent
fixture frames, original HRESULT/LastError/call counts and cleanup are checked.
The original launch verifies immutable media before/after and observes a bounded
three-PRESENT startup. Since a ring retains only a suffix, the harness compares
that physical retained window with the exact archive and records native reader
completion independently; it does not misinterpret circular storage as a full
append-only stream.

The first original-game attempt used the platform default640x480 Xvfb screen
and stopped on an original DirectDraw “Action not supported” dialog before any
owned drawing diagnostics. The failed attempt is retained under
`working/tests/live-render-game/run-1xdzsjt5/`; immutable manifests verified
before/after. It does not validate the rendering bridge. Original samples require
the explicit1280x1024x24 display configuration above.

## Execution evidence

Fresh final-code records: [native and sanitized queue](opengl-command-ring-adapter-queue.json),
[v2 PE32 sequence](opengl-command-ring-adapter-sequence.json),
[v1 startup regression](opengl-command-ring-adapter-startup.json) and
[original startup observation](opengl-command-ring-adapter-live.json).
The native incremental-consumer and live-channel CTests also passed, including
v2 creation and complete synthetic frames under three fragmentation budgets.
Strict CRT-free PE32 selftest and production builds succeeded.

The original run published39,363,820 bytes through the1MiB ring and ended with
three native PRESENTs before producer exit, zero live native surfaces and zero
ordinary native/RGBA readbacks or viewport uploads. Its retained ring window
matched the owned archive. All three frames share the same early-startup hash;
this establishes transport completion, not changing gameplay or original-driver
pixel equivalence. Both immutable-input checks verified2927 files.
The earlier successful run is preserved in
[initial observation](opengl-real-game-ring-adapter-initial.json) with its earlier
source hashes; the final record follows the sticky failure-reason correction.

Existing historical evidence for other contracts touched by shared source changes
retains its original fingerprints. These new records validate only their stated
queue, bounded fixture and startup scope. Dedicated idle progress, explicit
shutdown outside DllMain, sustained decoding beyond64MiB/4096 records, original
pixel comparison and live replacement remain pending.
