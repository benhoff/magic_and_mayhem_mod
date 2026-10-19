# Native startup history to World drawing

The opt-in producer viewer hands fully defined reconstructed native canvas
contents to the existing GPU World SceneRenderer at each contiguous queue entry.
Its source data, bindings and per-request clips come from the closed producer
stream and independently decoded installed SPR assets. Source identities remain
native logical generations. World GPU output is checked against independent
native CPU producer composition and committed back to native producer storage
before later copies and HUD work. It never initializes from an original
destination image or an invented zero clear.

Selected black/distortion primitives reject horizontal boundary cases before
World draw admission. Geometry outside the effective clip emits no drawing.
Unknown inside-World writes, undefined native history, destination/extent changes,
missing queues and incomplete drawing refuse the session. The default independent
Worldv2 zero-history viewer retains its existing policy.

`--world-producer-handoff` enables this experimental path in
`tools/capture-scene-game.py --canvas-producers --producer-live <executable>`.
Build the executable through `compat/legacy/canvas-producers/CMakeLists.txt`.
`tools/test-world-producer-handoff.py --capture-report <report.json>` freezes
sources, builds the native viewer/tests, compares every completion externally
and executes original World raster requests from reconstructed native entry
state in a separate private process. Original destination files are comparison
outputs only.

After a positive comparison, `--world-producer-bypass 8` selects at most eight
outer indexed-copy generic raster entries in the last captured queue. The
signature-checked original entries are5947b2/59521a. The adapter publishes the
owned source request before skipping its original raster body, waits for a
correlated native GPU completion and writes native tight RGB565 rows back to the
already locked original destination. The cdecl caller stack and incoming
GPR/EFLAGS/x87/SSE/LastError are preserved. Header identity, exact byte extent,
payload checksum and writable destination pages are checked before writeback;
malformed replies/timeouts terminate the bounded experiment without a destination
seed or fallback. Other rendering, simulation, menus and presentation stay
original. The fixed request/reply protocol is in
`protocols/include/mnm/world_producer_bypass_v1.h`.

Post-bypass destination observations contain native contributions. They cannot
alone prove equivalence. The independent driver compares every selected reply
against both original prefix replay and the precise suppressed original generic
body executed on the saved native before-canvas. Only those checked entries are
eligible for a live replacement claim. Wider producer takeover, driver/error
behavior, full GPU menu/HUD composition and native simulation remain separate
boundaries.

Initial original-active [live handoff](native-world-producer-handoff-live-20261008.json)
matches all16 World returns and1,062 sampled completed canvases:
474,236,912 RGB565 pixels, zero differences. Native World submits29,265 captured
source requests on this fixture; rejected/no-output requests remain explicit.
Startup contents and intervening HUD work enter every queue from reconstructed
native storage. Source fingerprints and2927 original files remain unchanged.

The initial [bounded bypass](native-world-producer-handoff-bypass-20261008.json)
skips eight actual59521a bodies in queue16. All eight native replies match
independent original prefix replay and the precise original entry executed on
saved native before-state:3,840,000 pixels, zero differences. The complete
[frozen comparison](native-world-producer-handoff-comparison-20261008.json)
also matches all16 independent original World completions /7,680,000 pixels and
all1,064 observed mixed destination completions /474,262,832 pixels.
Different runs sample different source requests; these are not timing comparisons.
5947b2 has synthetic bypass ABI coverage only; its live pixel replacement is
unobserved. The full World consumer and remaining original raster bodies stay active.

[Forwarding/bypass ABI checks](native-world-producer-handoff-forwarding-20261008.json)
pass all49 producer sites, synthetic native-body refusal/execution-counter checks,
GPR/EFLAGS/x87/SSE/LastError and two malformed reply refusals. Native handoff/HUD
retention, existing scene renderer/history and strict producer tests pass.
The adapter permits future-queue notifications while native processing catches
up, requires the exact producer sequence before replying and times out each
original-thread wait after120seconds. Observed completion does not establish
real-time throughput or a hard GUI latency guarantee.

[Historical failures](native-world-producer-handoff-negatives-20261008.json)
retain allocation/lifetime, horizontal distortion rejection, wrong synthetic
hook selection, future-queue refusal and source-drift evidence. Earlier
producer/startup/World evidence fingerprints remain unchanged; shared-source
changes may leave those older contracts stale without their own scope-bound reruns.

The renderer dependency refresh includes the shared sprite atlas and all119
reviewed compiler, build, runtime and validation dependencies. The new
[current GPU replay](native-world-producer-handoff-gpu-replay-20261008.json)
revalidates the prior eight captured bypass replies, all16 independent original
World completions and1,064 mixed completion observations with zero differences.
Historical result hashes above remain unchanged.

The verified frozen executable then completed a new
[current live run](native-world-producer-handoff-gpu-live-20261008.json): all16
queues,27,591 World source requests and1,062 completed canvases /474,236,912 pixels,
zero differences and no remaining native surfaces. One actual59521a body at
producer sequence45634 in queue16 was skipped. Its native before-state and reply
are retained. The new
[exact original comparison](native-world-producer-handoff-gpu-current-comparison-20261008.json)
executes that precise original entry and its preceding original raster prefix;
both match the native reply over480,000 pixels. All16 independent original World
completions also match. The
[current scoped replacement](native-world-producer-handoff-gpu-current-bypass-20261008.json)
binds this live writeback, original comparison, ABI evidence and verified native
binary provenance. This refresh validates one live draw; the prior eight-draw
live result remains historical, alongside its current offline replay.

Two attempted eight-call refresh captures contained only three eligible draws
in the final queue. The capture driver requires the full configured count, so
these are retained as
[incomplete-limit diagnostics](native-world-producer-handoff-gpu-limit-diagnostics-20261008.json),
not positive replacement evidence. One also used an interrupted older build with
two stale GPU dependencies; the verified current build was used for the successful
one-call run. An initial static diagnostic index omitted source provenance and
failed audit; it and that audit are retained unchanged. A new static index records
the capture driver fingerprint. These findings establish neither real-time
throughput nor wider draw replacement. Older shared behavior contracts need their
own fresh, scope-bound evidence before current equivalence can be claimed.

## Tranche 2 generic caller boundary (2026-10-19)

The finite generic path now admits only tag13/14 at exact byte-checked
continuations500512/500a4e/57e940. Unknown or mutated callers fail43 before a
request or destination writeback. The selected nonempty fully-inside rectangle
domain returns EAX0, matching private unchanged5947b2/59521a execution.
Empty/clipped requests forward original work. Each continuation restores20
argument bytes and consumes AX after overwriting flags; preserving every
incoming scratch value was not a substitute for comparing consumed return
semantics. Other original scratch-global/unsampled caller effects remain open.

Original controls compare36 cases, six caller/tag pairs and27,648 words. Actual
PE32 forwarding/refusal controls retain flags/x87/SSE/LastError, prescribed zero
EAX, unknown/mutated caller refusal, malformed replies and timeout behavior.

After a separately bounded untraced ownership pilot and positive source-only
GPU handoff, a fresh16-queue capture replaces exactly one59521a body in queue16.
Independent original prefix and exact-body execution both match all480,000
reply words. All1061 checkpoints and16 World completions also match externally.
The corrected opaque-run renderer has its own fresh capture/comparison.
No live5947b2 reachability, all-producer replacement or complete native ownership
is inferred from this single selected body. Earlier failed ownership journeys
and all pre-correction artifacts remain retained separately.

### Tranche 2 bounded acceptance completion — 2026-10-19

The eleven declared bounded tasks now pass. The subsequent static acceptance review links the independently compared single `59521a` replacement to the supplied source-stable 275-World normal campaign and same-snapshot isolation/input controls. All gameplay engine and harness fingerprints match the current checkout; differences are coverage tooling/tests and the approved 90 GiB reserve policy. See [the execution record](rendering-tranche2-execution-20261019.json) and `rendering-tranche2-results-20261019/bounded-acceptance-review.json`. This supersedes the earlier blocked task verdict, preserves all negative attempts and old source hashes, and promotes no broad engine status. Earlier stage-13/camera failures remain historical risks; no `5947b2`, complete/default/native-only/restart, hardware/FPS or arbitrary-scheduling acceptance follows. No evidence was deleted or archived.
