# CPU handoff ordering and tracker provenance

The previous strict summon repetition refused after a pixel-tracker miss with
an8ms admission budget. Its log did not identify the holding operation or the
requesting operation. This work adds bounded source-bound provenance and extends
the existing intentional opt-in copy scheduling policy to CPU Lock/Unlock calls.
No resource, queue, wait or recovery limit increases; original media remain immutable.

## Observed hold evidence

The [instrumented original observation](opengl-tracker-holds-observation-20261006.json)
keeps the prior copy-only scheduling scope. It passes the strict summon and30-second
post-action stable-region observation, finishing Active at783 frames. The bounded
`tracker_hold_origin` entries map tag `0xc73d012a` to copy commit admission in
`runtime/render/lock_surfaces.h:298`; four holds measure14,8,9 and15ms. This confirms
that copy commit can exceed the8ms tracker budget in that isolated execution.
Elapsed durations include scheduling pauses and work performed under the guard;
they do not identify a CPU or disk bottleneck. This run does not reproduce the
old failed request, so its exact requester remains unknown. The initial diagnostic
source hashes are preserved without asserting validation of the later gate extension.

`tracker_hold_origin` records owner thread, source tag and duration at guarded
release. `tracker_wait_origin` records waiting thread, sampled current owner,
request tag, sampled hold tag, wait duration and sampled hold age. Remote owner,
tag and time fields are independent atomic snapshots and may race ownership
turnover; do not treat them as a linearizable ownership certificate. Existing
`tracker_wait_timeout` diagnostics and conservative invalidation are retained.
All diagnostic arrays remain19 words; existing128-entry/four-per-reason limits apply.

Tags combine low16 basename FNV-1a bits in the upper16 bits with the source line
in the lower16 bits. The route report stores a collision-checked `tracker_sites`
map and exact source fingerprints. Helper macros preserve the requesting call
site through metadata/pixel entry wrappers. Tags are version-bound research data,
not stable behavior IDs, original addresses or a wire contract. Do not resolve
an old tag against a modified source file. LastError is preserved at tracker release.

## Native scheduling extension

In continuous mode, `MNM_RENDER_ORDERED_COPIES=1` now leases the same existing
process-local owner across application-facing Lock and Unlock callbacks as well
as Blt/BltFast. Lock admission includes the original Lock and descriptor recording;
Unlock includes pre-call owned capture, the original Unlock and successful commit.
It holds no tracker across an original call or a wait. Default scheduling is
unchanged. This intentional timing policy is not recovered DirectDraw ordering.

A CPU handoff cannot compete for the tracker while another thread owns a gated
copy transaction. The lease is released when each callback returns, including
original failure. It never spans application execution between Lock and Unlock;
existing borrowed-lock ownership, generations, epochs and complete pixel guards
still govern that interval. No new references or worker lifetime are introduced.

The existing50ms tick-based gate budget remains. Reentrant callbacks never wait;
uncertain nested writes retain refusal. On gate timeout, the original Lock/Unlock
is forwarded exactly once, target misses and pixel epochs invalidate both sides
of that unobserved call, and active legacy history is marked incomplete. An
unobserved successful Lock never manufactures a producer descriptor or authorizes
reading its pointer during a later Unlock. Other metadata, palette, DC, flip and
lifetime callbacks remain outside this gate and can still expose tracker contention.
Abnormal termination of a thread owning the gate remains outside the contract.

## Independent execution checks

The [final eight-case fixture matrix](opengl-cpu-handoff-native-20261006.json)
compares411 complete frames. `cross-lock` starts a real Wine worker's writable
Lock while an original fake primary copy is pending: the worker original must
enter only after the copy commit, then its independent native Unlock updates the
offscreen source. Primary output, original counts/arguments/results/LastError,
DELETE/END and zero final storage are checked independently.

`lock-timeout` makes the original copy wait for that worker. Both CPU handoffs
forward after bounded gate refusal, preserving original borrowed input and
poisoning it only in the independent fake engine after original Unlock. The
interrupted producer ends in GAP and publishes only its initial frame. Existing
cross-copy/BltFast, copy timeout, same-thread source-write refusal, identical
metadata and mixed RGB16 cases retain their independent results. The source
guard confirms no fingerprint changed during execution.

Reproduce with the existing optimized probe, Wine/Xvfb and Tesseract prerequisites:

```sh
xvfb-run -a python3 tools/test-render-mutations.py working/build/renderer-live --case cross-lock --case lock-timeout --case cross-fast --case cross-copy --case copy-timeout --case source-write --case meta16 --case mx16
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --ordered-copies --require-world-summon --world-seconds 30
```

Pending: classify any remaining tracker misses using version-bound request/holder
samples, other mutation-family concurrency and borrowed intervals, repeated scenes,
synchronized animated actors/effects/full-frame comparison, original hardware
Windows drivers and default enablement. The exact requester in the historical
failed summon run remains unproven; the gate extension is justified separately
by the independent CPU-handoff fixture. No replacement or balance change is claimed.

## Final source-stable original result

The [fresh strict record](opengl-cpu-handoff-live-20261006.json) confirms the
original summon and independent native instruction/count. Native publication
remains Active after forced World reader recovery and through30-second post-summon
sampling: 500 more frames over42435ms after recovery,
800 total at finish. All17 named stable-region comparisons pass;
terminal resources are zero, with no ordinary native readbacks or viewport uploads.
Source fingerprints and preferences are unchanged; all2,927 immutable original
files pass verification before and after. This establishes this finite run,
not unrestricted repeatability or animated/full-frame equivalence.

Committed history `89ab1db..4c01686` was reviewed separately against exact
parent/current file and behavior receipts, with no unresolved accounting gaps.
