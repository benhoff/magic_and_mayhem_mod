# Finite consumer surface residency

2026-10-06. Native policy `NR.continuous-resource-working-set` separates the
continuous consumer's 32 resident surfaces from the producer's existing 128-entry,
64 MiB independently owned native checkpoint store. This addresses the 33rd-surface
World loading refusal recorded in [the previous route observation](opengl-render-routes.md).
No original renderer replacement or driver equivalence is claimed.

## Admission and ordered lifetime

A continuous CREATE first looks for an existing alias-resolved wire resource.
If another complete resource needs admission and the 32-slot or 16,777,216 resident
pixel budget would be exceeded, the producer selects the least recently used
eligible offscreen wire resource. Recency uses the existing finite command sequence,
with deterministic slot order for ties. It queues DELETE, decrements resident pixel
accounting and clears only the session slot. CPU pixels, metadata, observed aliases,
application lifetime and attached-buffer relationships remain owned by the tracker.

A later admitted operation recreates the resource from complete owned native input
with a fresh monotonically increasing wire ID, then emits its ordered UPDATE/COPY
or FLIP. DELETE is consumer storage retirement; it does not assert original final
Release or call COM. Final application Release still retires CPU/alias state at its
verified boundary, deleting a wire ID only if it remains resident. Re-admission
never reuses a deleted wire ID. Exhaustion refuses before evicting anything.

Primary surfaces, active borrowed locks, suspended DCs and both operands of the
current copy/flip are excluded from eviction. Operand protection resolves verified
aliases. Eviction/admission is wholly inside the existing tracker guard; no original
call, GPU work or consumer wait is performed there. Pending queue commands already
own their serialized inputs and remain in FIFO order before DELETE. An all-protected
working set refuses rather than dropping updates or inventing pixels. Pixel-pressure
admission can delete several eligible resources before CREATE; if it still cannot
fit, the stream is invalidated explicitly.

Default bounded samples retain their 32-surface refusal policy. Palette residency,
128 CPU resource records, 32 borrowed locks, 32 palettes, 64 MiB owned/pending/scratch
storage, 32 MiB private queue, 1 MiB ring, deadlines and finite sequence/byte/ID
lifetimes retain their existing limits. Diagnostic cache eviction/refusal records
use the existing bounded/deduplicated lifecycle log.

## Recovery checkpoint

Strict CHECKPOINT and automatic prefer-checkpoint recovery still require complete,
current owned native pixels and metadata for **every observed CPU surface**, unique
verified identities, complete indexed palette bindings, exactly one primary,
quiet borrowed ownership and no pending missed operations/alias invalidation.
The surface-count bound now follows the existing 128-entry owned tracker rather
than the 32-slot consumer cache. The existing conservative aggregate admission of
16,777,216 owned pixels and 32 MiB full-state serialization estimate, including
cleanup reserve and palettes, remains enforced.

After admission, the new session preserves the entire verified CPU state and
creates complete palette resources and the complete primary before PRESENT/READY.
It materializes other complete native dependencies when subsequent operations
need them. Thus a consumer receives a complete initial frame and valid resource
references without allocating all retained offscreen assets together. A missing
baseline never becomes an incremental update. Metadata-only, stale, ambiguous,
borrowed and over-budget checkpoints still refuse; claimed failures do not retry
the same candidate file. Direct-export fresh recovery remains a separate policy.

## Validation

The [six-case actual PE32/GPU record](opengl-working-set-native-20261006.json)
checks 142 independent complete fixture frames. Sixty-four small offscreen resources
plus the primary rotate through at most 32 resident slots. Two-operand copies,
partial failed-call retries, evicted attached-buffer flips and primary retention
create 373 fresh wire IDs; archive replay checks live references and every DELETE.
Held-lock and real Wine DIB/DC pressure retain the protected wire ID until successful
original Unlock/ReleaseDC. An entirely borrowed working set refuses. Alias pressure
checks original nonfinal/final Release counts and pointer reuse after cache eviction.

The large RGB565 case retains five complete 2048x2048 offscreen CPU snapshots while
only three fit beside the primary on the consumer. Eight CREATEs/re-admissions keep
peak residency at four surfaces and 12,582,936 pixels, retaining 41,943,136 owned
CPU bytes. Consumer and publication worker/queue/mapping cleanup pass without
ordinary renderer readbacks or CPU viewport uploads. These synthetic fixtures
check original fake-engine results/LastError and poison borrowed rows before the
hook can consume them; expected GPU frames come from separate original fixture
storage, never from the producer's own mirror/checkpoints.

The [strict checkpoint matrix](opengl-working-set-checkpoint-20261006.json)
passes 20 cases and 312 independent complete-frame comparisons. The former
33-surface refusal case now completes three checkpoints with retained CPU pixels
and lazy dependency reconstruction. Mixed formats/operations, shared indexed
palettes, retirement/recreation, overflow/stall/stale ACK, held resources,
incomplete metadata/pixels and allocation/mapping/worker faults remain covered.
The [administrative recovery regression](opengl-working-set-host-20261006.json)
retains separate actual-worker and same-context control/refusal coverage.

Reproduce the new matrix with:

```sh
xvfb-run -a python3 tools/test-render-working-set.py working/build/renderer
xvfb-run -a python3 tools/test-render-checkpoint.py working/build/renderer
xvfb-run -a python3 tools/test-render-host-recovery.py working/build/renderer
xvfb-run -a -s '-screen 0 1800x1000x24' \
  python3 tools/test-live-render-routes.py working/build/renderer \
  --mode campaign --require-world-active
```

The route's required World assertion checks Active complete frames before and after
an injected failed reader, a single session advance, and at least 20 further frames
spanning two seconds after recovery. This is bounded gameplay ingress/recovery;
long scenes, action/effect/HUD/movie coverage, all original callers, hardware-driver
pixels, external host takeover and live replacement remain pending. CPU tracker,
palette, snapshot and queue saturation can still refuse beyond this scope. A
resident cache does not remove those finite budgets or make incomplete state safe.

The [required original-game run](opengl-working-set-game-20261006.json) stays Active
through New Game/Enter/three original World ticks. It reaches 32 resident surfaces
without a capacity GAP, restores complete state after the injected World reader
failure, and publishes 29 additional frames over 3.02 seconds in the same recovered
session. Four independently captured stable menu regions match within one RGB
channel value, including after Region Entry recovery. Original World drawing stays
active, native terminal resources/readbacks/uploads are zero, source preferences
stay unchanged, and 2,927 original files verify before/after. These are menu-region
pixel observations plus bounded World publication/recovery; World pixels and movie
playback are not independently compared. The earlier exploratory World run remains
under working; only the final required-assertion source snapshot is registered.

Fresh continuity and final-Release regressions pass nine cases each, with 422 and 63
complete independent GPU frames respectively. The former continuity resource-cap
case now succeeds through cache eviction; default bounded retirement, held work,
ambiguous/missed aliases and invalid/missing transport remain refusals. Together
with the six-case cache, 20-case checkpoint and 16-case host matrices, the recorded
synthetic checks compare 1,128 complete fixture frames. Source fingerprints remain
unchanged across all five final synthetic reports. No historical evidence hash was
refreshed. Retrospective receipts cover two compressed fixture payloads omitted
from exact file accounting in 558b1fc/c84f26e; this asserts neither a past gate pass
nor new original routine execution.
