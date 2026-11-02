# First Main to Quick transition timeline — 2026-11-01

This bounded diagnostic batch starts at d93dcd510. It targets only
RW03.G.intro-main-transition: the retained old failure has Main callback entry
and return, terminal ACK1 and no World, but no ordering of ACK versus retirement.
RN31.G.first-world-watchdog stays a separate investigation.

The host writes an optional versioned fixed-size sideband before producer launch,
then appends64-byte records to mapped memory without a synchronous live GUI log or
database write.512 committed records are immutable; overflow refuses evidence.
The collector writes on-disk SQLite with committed FULL-synchronous WAL transactions
and closes/checkpoints after capture. No in-memory backup is used. Wire observations
are double-read through the existing seqlocks; file replacement or multiple public
generations refuses capture. Full V5 payloads are bounded to128 snapshots around
the first transition, to preserve possible decoder-refusal evidence.

C++ host records and Python observations explicitly use the same Linux
CLOCK_MONOTONIC namespace. Existing original callback records have sequence
ordinals, **not execution timestamps**. Their visibility is bounded by successive
read intervals; the first interval can have an unknown lower bound. Original
logging can drop contended rows and caps at256 records. Missing records do not
prove the original callback was absent.

The controlled case suppresses only Qt menu-state reads after its first accepted
Main request. Heartbeats and the existing2000ms state lease and10000ms action
bound remain unchanged. This deliberate blind spot should retain the original
entry/return, published ACK, timeout and retirement ordering. It calibrates the
recorder; it is not a claimed reproduction of the unforced historical cause.

A persistent SQLite budget permits one controlled launch and at most two
unforced enabled-intro launches. Successful runs do not erase failures or grant
whole-player/renderer acceptance. Current controls and actual outcomes are pending.

The shared LiveMenuSession edit leaves other complete-source historical evidence
pending. Only NP.main-quick-timeline can gain scoped current diagnostic support;
original comparison and replacement statuses remain independent.

## Bounded result

The [joined batch](main-quick-timeline-results-20261101/joined-01.json) retains
current optimized/sanitized witness controls, eight decoder/persistence controls
and ordinary startup/movie deferral regression. The one live control retains
Main callback entry/return and ACK1 before client retirement; Qt read suppression
causes the unchanged state-freshness guard to expire. The two unforced attempts
complete both movies, the first Main/Quick transition and normal original Quit.

| Case | ACK visible by, ms after publication | Qt ACK read, ms | Quick ready, ms |
| --- | ---: | ---: | ---: |
| control | 94.677 | suppressed | absent |
| unforced-1 | 50.977 | 34.347 | 589.550 |
| unforced-2 | 21.016 | 29.704 | 545.035 |

ACK visibility is an external observation bound, not the exact original publish
time. Original callback intervals remain receipt bounds rather than execution
timestamps. Full payload snapshots are retained for possible Qt decode analysis.

**Exit: not reproduced in two unforced attempts.** RW03.G.intro-main-transition
and RN31.G.first-world-watchdog remain unresolved. No product repair, deadline
extension, retry, nominal player acceptance or original-recovery promotion is made.
The recorder is available for the next naturally occurring failure; these finite
passes do not establish absence of stalls or retire old source-phase negatives.

The controlled native run was retained successfully before a post-run handoff
queue rejected missing nested prefix-return metadata. Its [failure](main-quick-timeline-results-20261101/control-01-handoff-failure.json)
and exact old producer remain immutable. The corrected producer changes only
post-capture prefix-return reporting. The control was not rerun; this result
does not claim fresh control execution at the new producer hash. Both unforced
runs bind the corrected current producer separately.

The diagnostic is scoped to one fresh public generation per process. Reusing the
same LiveMenuSession for a diagnostic restart is not validated; the collector
refuses multiple admissions. Each bounded attempt used a fresh process.
