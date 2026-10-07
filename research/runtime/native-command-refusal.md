# Native command refusal diagnosis

2026-10-06. `HOST.command-refusal-diagnostics` is an intentional native
presentation policy. The reader reports the producer's session and terminal
reason only after validating the v2 ring identity and FAILED state. GAP means
surface snapshots or drawing history became incomplete; OVERFLOW, CANCELLED,
INTERRUPTED and INVALID retain distinct names. Unknown codes retain their number.
Malformed identities still fail through the wire validator and cannot supply
trusted producer diagnostics. Cancellation and viewport/resource cleanup remain
unchanged. This does not repair missing drawing history or relax ownership.

## Confirmed user-run finding

`working/experiments/opengl-render/run-b_9tbgld` used continuous commands, an
8 ms tracker admission budget and movie playback. All three mapped sessions
ended FAILED with wire reason2 (GAP). The lifecycle log records a tracker timeout
at lines67–68, followed by a pixel miss, session invalidation and queue refusal.
The first recovery selected a checkpoint and then refused. The next selected
fresh observations after checkpoint admission found an incomplete surface;
that stream also refused before its first frame. The sampled requester/holder
tags map, against the captured DLL source fingerprints, to copy commit admission
and pre-Unlock capture. The diagnostic fields are independent atomic samples,
not an exact ownership certificate.

This establishes a producer ownership/history failure independently of Qt
fullscreen. The Mesa and Quartz warnings do not establish the cause of that GAP.
Its underlying hold includes capture work and scheduler pauses; the trace does
not determine which dominated. No stable-address or original-driver equivalence
claim follows from this observation.

The existing `MNM_RENDER_ORDERED_COPIES=1` policy coordinates drawing and CPU
handoff callbacks, including the two operation families above. It remains opt-in
because it intentionally changes drawing-call scheduling; see
[CPU handoff ordering](opengl-cpu-handoff-order.md) and
[copy scheduling](opengl-copy-order.md). Raising the tracker budget or bypassing
snapshot checks is not a validated fix. Software rendering and disabled movies
alone did not complete the early-checkpoint recovery check in
`working/tests/live-render-recovery/run-fqrwxk37`; that failed attempt remains
available, and original manifests verified before and after. A concurrent commit
also changed a fingerprint during that attempt, so it is not current-code
validation.

## Validation boundaries

The source-bound native fixture checks five known producer reasons, an unknown
code and a malformed session identity, alongside complete-frame, fragmentation,
checkpoint-budget and sustained streaming regressions. Actual Qt Launch-button
tests cover windowed/fullscreen/fractional-DPI continuous startup beyond4096
commands and explicit bounded mode. A separate original startup observation
retains original drawing and completes one bounded v2 native PRESENT; it does
not validate continuous movie playback or sustained gameplay recovery.

User hardware/movie playback, repeatable original continuous publication,
synchronized full-frame comparison and native replacement remain separate gaps.

The fresh isolated ordered campaign run in
`native-command-refusal-ordered-route-20261006.json` passes with software
rendering and movies disabled:328 native frames, independent stable menu region
comparisons, forced region and World recovery, and27 further World frames over
3106 ms after recovery. This supports the README's opt-in launch configuration
within that bounded software-rendered run. The policy remains opt-in; it is not
a user-hardware/movie-playback or unrestricted-repeatability result.
