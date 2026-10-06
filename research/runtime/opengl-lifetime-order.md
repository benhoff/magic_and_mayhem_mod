# Resource lifetime callback admission

Continuous-only opt-in `MNM_RENDER_ORDERED_COPIES=1` now admits application
QueryInterface, CreateSurface, CreatePalette, surface Release and palette Release
through the same existing drawing callback lease. This is intentional native
scheduling policy, not recovered DirectDraw ordering or default enablement.

The lease spans original execution and existing successful identity/alias/create
or verified final-release processing. Failed creation/query calls do not become
producer creations/aliases; nonfinal admitted Release does not retire resources.
Original arguments, signed HRESULTs/unsigned reference counts and LastError are
preserved. Scoped cleanup releases the gate on ordinary failures and success.
No tracker is held while waiting or executing the original. Existing50ms tick
admission,8ms tracker, surface/palette/queue/pixel budgets and same-thread reentry
are retained. Borrowed CPU/DC intervals are still outside callback admission.

A timeout forwards the saved original once without installing/recording returned
interfaces or declaring producer retirement. It invalidates metadata/pixel epochs
and schedules alias-graph reset before and after original forwarding. A missed
final Release can destroy an entire interface component; preserving its alias
links would allow pointer reuse to inherit identity even after pixel invalidation.
The next guarded tracker entry consumes the reset without dereferencing COM
objects. The second reset discards relationships learned during the uncertain
interval. Consumer histories refuse rather than emit guessed DELETE or resource
identity. Returned interfaces from missed creation/query need later independently
admitted observation; complete recovery of such unseen objects is not promised.

The existing verified-retirement implementation still separates producer storage
from consumer resource lifetime and checks pending borrowed ownership. Timed-out
final-release forwarding never inspects the destroyed receiver. A new read-only
SELFTEST-only DLL export inspects opaque alias relationships under the tracker;
it is absent from production builds and supplies no producer pixels or metadata.

## Validation

The [final lifetime matrix](opengl-lifetime-order-native-20261006.json) checks15
cases,72 independent complete native frames, seven valid/eight refused sessions.
Existing consumer zero-storage/readback/upload and exact original call-count,
return/LastError, monotonic ID, CREATE/DELETE and terminal END/GAP checks pass.

- `cross-release` holds primary Unlock admission while a worker waits to perform
  nonfinal and final offscreen alias Release. Original storage is logically dead
  before the hook receives its token. After joining, the same alias address is
  reused for an independent surface: three distinct resource IDs retire exactly.
- `cross-alias` waits before original QI while an independent primary update is
  pending. Subsequent nonfinal/final Release shares the verified component and
  produces exactly two resource identities without duplicate deletion.
- `cross-create` waits before original CreateSurface, including failed creation
  and successful retry. The original poisons input descriptor width afterward;
  producer metadata uses its owned pre-call descriptor. A subsequent independently
  generated checkpoint has a second ID and clean final retirement.
- Each corresponding `*-timeout` forces the outer original to wait for the
  worker. All original calls still complete, but only the initial native frame
  publishes before GAP. Read-only fixture inspection confirms no retained alias
  relationship between the old object and alias tokens after synchronization.
- Existing churn, more-than96MiB publication with finite pixel/storage budgets,
  alias/nonfinal/final release and same-pointer recreation, untracked release,
  borrowed Lock/DC retirement, tracker contention, conflicting aliases and
  bounded-mode refusal retain their original pixel/count/retirement checks.

The separate [shared-palette/drawing matrix](opengl-lifetime-order-mutations-20261006.json)
compares545 complete frames across eight cases and exercises admitted palette factory,
alias and final-release callbacks during40 same-pointer recreation cycles, as
well as palette/copy/DC/flip/CPU regressions. This is same-thread palette lifetime
validation; dedicated cross-thread palette creation/release remains pending.

```sh
xvfb-run -a python3 tools/test-render-resource-lifecycle.py working/build/renderer-live
xvfb-run -a python3 tools/test-render-mutations.py working/build/renderer-live --palette-resources --case indexed --case cross-palette --case palette-timeout --case source-key --case cross-flip --case cross-dc --case mx16 --case dc32
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --ordered-copies --require-world-summon --world-seconds 30
```

Remaining: dedicated palette lifetime races and failed/nested QI/lifetime fixtures;
DirectDrawCreate/startup orchestration admission; surface description, attachment,
Restore/BltBatch callbacks; borrowed intervals/abnormal owner termination; missed
creation/query interception recovery and implicit/unobserved destruction;
legacy archive-timeout replay, repeated scenes/actions and original-driver,
animated/full-frame comparison/default enablement. No gameplay balance or live
replacement is claimed.

Exact committed history `e794fd2..420f919` was reviewed against parent/current
file/behavior receipts with no unresolved entries. The history review preserves
old hashes and does not assert new validation or original semantic coverage.

The RGB565 fixture expectation now follows the independently retained Surface2
DC bit expansion adopted by the intervening420f919 renderer commit; noncanonical
masks retain normalized scaling. Native fixture bytes remain independently
computed. An initial drawing attempt using the historical normalization rejected
its RGB16 frame. A subsequent passing workspace drawing record and original
observation rejected by its final source guard are retained separately. Neither
asserts current committed-renderer validation. Final drawing/live records use a
private committed-source snapshot plus only this lifetime chunk.

The [final private original observation](opengl-lifetime-order-live-20261006.json)
passes with796 native frames,487 additional frames over
40296ms after forced World recovery,17 matched stable regions and zero
terminal resources. Original tutorial summon/count, full source/preference guards
and2,927 immutable-artifact checks before/after pass. This remains bounded
unsynchronized stable-region observation, not original-driver/full-frame equivalence.
