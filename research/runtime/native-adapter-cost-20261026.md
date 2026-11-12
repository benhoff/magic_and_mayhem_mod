# Native adapter cost, 2026-10-26

The target is approximately 20 **fresh original World updates per second**
(50 ms per update). Repeated native redraws of an old World are not fresh engine
FPS. These are bounded tracing-enabled measurements on a shared host, using the
retained native-latency Qt diagnostic build and isolated original No-CD engine.
They do not certify monitor latency, sustained performance, whole-game pixels or
all current application sources.

## Changes and ownership

- Defer lifetime collection until retirement can make progress. Nested guards
  owned by the same PE32 thread use the TEB thread token and integer depth;
  outer guards retain critical-section, LastError and x87/SIMD preservation.
- Borrow an unchanged bridge-owned copy source only after the original call and
  epoch/generation checks succeed. Reuse a distinct initialized destination only
  while the tracker guard remains held, after before-records own their bytes.
  Bootstrap/self-copy still require independent storage; failures invalidate or
  checkpoint without changing the original result.
- Find each dirty row's first and last changed pixel. Skip the interior once its
  exact envelope is known; native storage bytes and pending-byte masks still
  define dirtiness, including unused RGB32 bits.
- Cache exact-address history namespace lookups only under the serialized
  provider lifetime mode. Allocation publication, final release and physical
  Flip invalidate the cache; every authority/lease decision remains current.
- Retain at most four exact-size private bridge buffers, each 64 KiB–1 MiB.
  Idle storage participates in the existing 64 MiB budget and is evicted before
  it can prevent an admissible active allocation. Pending Unlock snapshots and
  masks are counted together. Contended returns publish idle ownership before
  subtracting active accounting. Recovery drains idle buffers; terminal stop
  drains and disables the pool. No pixel authority or surface identity is cached.
- The checked No-CD image-copy row at `0x533aec` previously requested current
  CPU history for both source and destination. A small write to an 800×600
  destination could therefore trigger a 960,000-byte GPU-to-CPU transfer. Require
  current source bytes, but use the existing value-only private-alias store for
  an eligible stale protected owner-Lock destination. Its public pages remain
  inaccessible, CPU authority remains stale and exact dirty bytes remain pending.
  Same-allocation source overlap restores history and preserves original forward
  DWORD/residual-byte order. Existing unsupported-lease restoration and exact
  executable-byte checks remain in place.

This last change is an intentional native admission policy, not a recovered
claim that the original DirectDraw engine supported deferred GPU history.

## Measurements

Warm adjacent original World-entry intervals, excluding hidden/pause/cold rows:

| Run | Change | Mean ms | Fresh FPS |
| --- | --- | ---: | ---: |
| baseline01 | Prior collector/source borrow | 85.60 | 11.68 |
| candidate01 | Dirty row edges | 84.02 | 11.90 |
| candidate03 | Destination reuse/history cache | 74.89 | 13.35 |
| candidate04 | Recursive guard fast path | 72.47 | 13.80 |
| candidate05 | Repeat with fault sampling | 69.97 | 14.29 |
| candidate06 | Private storage pool | 59.20 | 16.89 |
| candidate11 | Protected image-copy destination | 50.26 | 19.90 |
| candidate12 | Confirmation | 50.19 | 19.92 |

These sequential observations have different host activity and some different
input windows; they are not independent causal speedup estimates. Candidates09+
add 20-second quiet windows before each unchanged stress phase. Earlier totals
must not be compared as identical workloads.

The pre-fix candidate10 and fixed candidate11 phase analysis uses the same input
sequence and excludes one second at each boundary:

| Phase | Before, fresh FPS | Fixed, fresh FPS |
| --- | ---: | ---: |
| Initial quiet | 20.00 | Not retained in bounded trace tail |
| Initial stress | 18.57 | 19.77 |
| Resumed quiet | 14.26 | 19.98 |
| Resumed stress | 15.72 | 19.91 |

Candidate11 completed 1,877 Worlds and normal menu/resume/quit lifecycle.
Candidate12 repeated the lifecycle over1,833 Worlds: initial retained stress19.87,
resumed quiet20.00 and resumed stress19.92FPS, with35 CPU transfers. Both fixed
runs lose initial quiet rows from the bounded trace tail; phase results cover
retained adjacent entries, not a complete initial-window census.
Command-channel full-surface CPU reply tickets fell from 802 to 31 (96.1%).
These transfers are **separate from** the World producer's zero readback counter;
zero World readbacks never established zero command-consumer materializations.
The four sampled materialization logs in the pre-fix run name `0x533aec`.
Candidate09 independently recorded829 transfers and a similar resumed slowdown.
The remaining transfers are retained; this is not a zero-readback claim.

The storage pool reduced process minor faults from207,500 over10.272 seconds to
3,474 over10.230 seconds in candidate05/06. Both had zero major faults.
Dirty-region differential controls compare6,000 exact partitions; the dense
640×480 microbenchmark fell from2.357 to0.0417 ms. These isolated costs are not
whole-frame latency.

Rejected and reverted experiments remain preserved: Sleep(1) in the handoff
(candidate02,90.98 ms), a16 ms worker-status cadence (candidate07,64.14 ms), and
128 PAUSE polls before yielding (candidate08,64.71 ms). None changed the adopted
pacing policy. Original target50 ms and gameplay rules are unchanged.

## Validation and evidence

SQLite-backed reports and raw binaries/logs are under
[native-adapter-cost-results-20261026](native-adapter-cost-results-20261026/) and
[native-engine-latency-results-20261026](native-engine-latency-results-20261026/).
Old reports retain their original hashes, including failures and superseded
source versions. The current controls are registered independently:

- `dirty-controls01`: native and ASan/UBSan dirty-region controls, plus exact
  old/new partition comparisons and isolated costs.
- `storage-controls03`: native and ASan/UBSan exact-size reuse, zeroing,
  allocation failure, eviction, tight budget, concurrent return and terminal
  stop. All17 allocated test blocks are reclaimed.
- `provider-controls03`:32 cases in each plain/protected profile,199,031
  assertions/profile,1,616 full planes,1,600 concurrent copies,16 alias pairs,
  eight writer cases,512 pins and zero surviving surfaces/devices/DCs. Nested
  and outer guard checks preserve FP/SIMD state and LastError.
- `image-controls03`: actual PE32 destination-only writes with no reader
  callback, exact dirty masks, stale authority and NOACCESS; subsequent source
  restoration; stale same-allocation DWORD/residual overlap; span and executable
  prefix refusals;528 original clipped-copy cases and existing transforms and
  continuation controls. The original row-body tail is replaced with traps in
  the fixture. The initial harness link failure is retained asimage-controls01;
  its missing CreateThread import was fixed in the runner.

- `history-controls04`:33 real-provider checks in each compatibility/serialized
  profile, plus native/sanitized cache/store checks and three terminal reader
  refusals per profile.
- `copy-controls06`:26 actual PE32 copy cases and independent CPU/OpenGL
  replay, eight fills, three continuous publication policies and seven Unlock
  cases (including retry, budget, missing base and invalidation). The two
  preceding replay-process failures are retained ascopy-controls04/05; a private
  network namespace isolates X server sockets. The runner now checks process
  failure before parsing JSON so diagnostics are not hidden by a decoder error.

The live harness verifies private hardware/native-only presentation, no loaded
DirectDraw module/forwarding, functional Mini/resume and Quit behavior, normal
engine exit and retirement of graphics resources. Production PE32 symbol maps
are accepted only after an exact byte-identical relink. The Qt binary is retained
from a prior diagnostic context: five application source files and two glyph scene sources differ from this
worktree; the diagnostic input helper also intentionally adds quiet windows.
Additional retained legacy adapter, glyph backend, harness and test differences
are listed inperformance01.json; no current-source credit is inferred for them.
The complete current render/native-draw source closures match the fixed runs.
This is not validation of every current application/scene source.

## Boundaries and accounting

Behavior stages remain independent and unchanged. Bounded controls do not promote
original-driver comparison, whole-session pixel equivalence, all writer/caller
coverage or complete replacement. Trace-disabled sustained timing and an
all-current application build remain separate validation work.

`committed-history01` reviews the six first-parent commits from6cdbbacd through
8c0c4ea4:3,902 file transitions have matching exact receipts, with no unresolved
receipt match. Three historical large payloads were unavailable locally; their
pinned pointer identities matched receipts, but their bytes were not verified.
This is a historical availability limitation, not a past gate-pass claim.
The source/evidence batch has its own exact hash-bound review. This commit
prepares the tested adapter changes for integration into master; combined-source
validation remains separate from the retained diagnostic measurements.

The task remains isolated in the native-adapter-cost worktree. Original inputs,
other contributors' worktrees, historical evidence and shared prefixes are
preserved. Temporary builds and prefix copies run in private RAM namespaces;
new durable results and review bookkeeping use SQLite with retained local
verified backups. No evidence is published or uploaded by this work.
