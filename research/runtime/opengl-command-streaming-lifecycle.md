# Idle command transport lifecycle and bounded-memory native streaming

2026-10-06. Native policies `NR.command-idle-lifecycle` and
`NR.command-streaming` extend the [mapped adapter](opengl-command-ring-adapter.md).
These policies do not recover original-driver equivalence or replace original
rendering. Version1 and complete offline replay retain their64MiB/4096 limits.

## Retry and shutdown contract

`runtime/render/command_scheduler.h` starts one PE32 retry thread from the first
normal DirectDrawCreate callback (or the isolated test entry point), outside
DllMain. The worker calls the same guarded nonblocking queue pump, with at most
1MiB per attempt and10ms between retries. It owns no tracker, COM object, borrowed
pixel pointer or native GPU state. Ring FULL retains bytes without peer waits;
reader ACK frees transport slots only after a private copy. Startup and explicit
shutdown share a nonblocking guard, including worker handle publication. Worker creation failure
refuses the channel. A terminal ring ends the retry loop. Callback pumping remains
available, and all drain attempts use the same guard. The pump checks queue storage
under that guard, including after explicit shutdown releases it.

The exported stdcall `RenderShutdown(milliseconds)` is an explicit application
lifecycle entry point. It acquires the tracker, finishes/refuses the owned sample
and stops capture admission, releases the tracker, then drains queued publication
for at most min(milliseconds,5000)ms. If a reader never makes space, pending work
refuses with INTERRUPTED; existing first failure remains sticky. It requests worker
stop and joins outside the tracker and loader lock with a1000ms join bound. A join
timeout retains worker-visible storage and reports failure; a later call can retry.
After successful join, mapping/queue/thread handle are released. Repeated completed
shutdown returns the same outcome. Success means clean producer END/publication,
not renderer/GPU completion; the native consumer must still drain and finish.
GetLastError is preserved. Concurrent shutdown attempts fail without waiting.

Calling this export is the orchestrator's responsibility before unload/process
exit; the legacy game is not patched to call it automatically. Normal completed
samples can finish through idle retries without calling it. Consumer cancellation
propagates to the worker and refuses pending work. DllMain remains a bounded
best-effort fallback: it requests worker stop and refuses interrupted pending
publication without joining, retaining worker-visible resources for process cleanup.
Manual unloading with installed callbacks remains unsupported; this export is not
hook uninstallation, and force-killed processes cannot promise orderly shutdown.

## Sustained native decoding contract

Live channels select streaming mode automatically for wirev2. `CommandDecoder`
and `CommandConsumer` retain bounded mode by default, including v1 and offline
replay. Streaming removes cumulative64MiB/4096 caps while retaining:

- At most64KiB input per append and a maximum record of16,777,256 bytes, derived
  from2048x2048x32-bit CREATE plus its fields/header. Framing retains at most one
  incomplete record plus one fragment/header allowance; completed byte prefixes
  are removed. Decoded commands are consumed in32-command Qt batches before the
  next channel read.
- At most64 live surfaces and16,777,216 live pixels, with normal format, geometry,
  ordering, alias and resource checks unchanged. CREATE IDs must strictly increase;
  only the high-water ID is retained instead of a growing set of destroyed IDs.
  Bounded offline mode retains its existing unique-ID policy.
- Finite32-bit byte publication and command sequence lifetimes; neither cursor nor
  sequence wraps. Streaming decoder refuses beyond UINT32_MAX input bytes. A new
  session is needed before exhausting that lifetime.
- Explicit surface destruction and END before completion. Trailing bytes, missing
  END, malformed record lengths, sequence gaps, surface misuse and cancellation
  refuse rather than resynchronize or silently drop work.

Streaming memory is bounded by current fragment/record/decoded batch and live
surfaces rather than cumulative session traffic. GPU leases retain their existing
ownership rules. Ordinary execution performs no native/RGBA readbacks or viewport
image uploads; framebuffer grabs in tests are explicit validation only.

This increment does not change the PE32 owned sample's64MiB archive,4096 records,
32 PRESENT target or256-operation ceiling. Those are independent observation
limits, not limits of the v2 native decoder. Sustained original gameplay capture,
archive rotation/optional capture, session rotation before4GiB, application-wide
shutdown invocation, original pixel comparison and replacement remain outstanding.

## Reproducible validation

```sh
python3 tools/test-render-command-queue.py
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/test-render-command-idle.py
cmake --build working/build/render-ring --target live-render-channel-test \
  render-command-consumer-test mnm-render-commands
xvfb-run -a python3 tools/test-render-command-streaming.py working/build/render-ring
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/test-render-session-sequence.py \
  working/build/render-ring --wire-version 2
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/test-live-render-game.py \
  working/build/render-ring --presentations 3 --wire-version 2
```

The74.6MB synthetic run does not execute the decoder at the full4GiB
lifetime boundary; join-timeout and OS thread-creation failure remain reviewed
refusal branches rather than injected execution cases.

Fresh final-code execution: [PE32 idle lifecycle](opengl-command-streaming-idle.json),
[native sustained GPU](opengl-command-streaming-native.json),
[native/sanitized C queue](opengl-command-streaming-queue.json),
[v2 sequence](opengl-command-streaming-sequence.json),
[v1 startup](opengl-command-streaming-startup.json) and
[original startup](opengl-command-streaming-live.json). Strict CRT-free PE32
selftest/production builds pass; eight sequence cases compare44 full fixture
frames and eight v1 startup cases comparefour. Both2927-file original manifests
verify. Original three frames remain identical early startup, not gameplay.
Prior adapter
records retain their old fingerprints; shared-source changes do not refresh unrelated
historical contracts or assert original equivalence.

The [ASan/UBSan native GPU repeat](opengl-command-streaming-sanitized.json)
also passes the74,631,284-byte/5,077-command fixture and bounded regressions.
As in the existing Qt/Mesa harness, integration uses detect_leaks=0; the C queue
ASan/UBSan runner enables leak checking. Native surface cleanup is asserted
separately. The initial v1 regression attempt was invalidated by a source change
while running; its final fresh rerun above passes without hash changes.


## Subsequent producer increment

[Continuous owned production](opengl-continuous-producer.md) now has its own
opt-in `MNM_RENDER_CONTINUOUS=1` policy, independent of the historical bounded
sample described above. The v2 consumer/queue budgets and finite counter
lifetimes remain; optional command archives retain their old bounded limits.
Default bounded sessions and this increment's historical evidence are preserved.
