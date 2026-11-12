# Campaign repeatability follow-up — 2026-11-02/03

This owned batch starts at master merge `8e7b87843`, which already contains
campaign completion `9f8475790`. It fixes one reproduced application startup
stall and retains bounded progression, victory and defeat attempts. No original
simulation, health, mana, AI, script, balance or menu lease is changed. Reliable
nominal victory/progression and supported-player acceptance remain pending.

The subsequent [approach correction](campaign-approach-20261103.md) starts from
this committed source phase. Results below retain their original source scopes.

## Confirmed startup path and application fix

The [focused failure](rendering-progression-results-20261101/repeatability-startup-dwell-software-low-02.json)
uses skipped intros, software640 and 20-second Main dwells. It acknowledges two
pre-World source chunks, then stops after Preferences. The [corrected diagnosis](rendering-progression-results-20261102/repeatability-startup02-persistence-diagnosis-02.json)
records 4,358.833ms synchronous GUI persistence: 1,080.898ms obtaining the store
lock and 2,125.359ms in atomic commit. State age reaches 4,360ms against the
unchanged 2,000ms menu lease. The final source slot is collecting, with no pending
chunk ordinal. The superseded narrative and its raw fields remain immutable.
This confirms this fresh failure path, not the cause of all historical startup
variants or the separate glyph/source-history refusals.

`EnginePreferencesPersistence` captures a bounded immutable engine-written
profile, verifies the same seven accepted values, and queues durable commits on
one worker. FIFO order, revision conflicts, file locking, atomic commit and
Cancel semantics remain intact. Queued, in-flight and unconsumed results share a
16-job budget. Results return on the GUI thread; accepted work drains before a
new session and during shutdown. The snapshot read itself stays on the owner
thread and remains subject to filesystem latency.

[Lifecycle controls](rendering-progression-results-20261102/repeatability-phase04-controls.json)
cover the store, mapped-menu heartbeat, startup/resources, session destruction,
hard restart and Preferences controller. A writer blocked for 2,233.199ms leaves
the GUI/menu heartbeat live beyond the unchanged lease. Production builds keep
`BUILD_TESTING=OFF`; fixtures use a separate ON tree.

The [same focused rerun](rendering-progression-results-20261101/repeatability-startup-dwell-software-low-03.json)
completes natural defeat and normal Quit in 355.700s, including four source-only
chunks and 1,816 original World completions. Its [independent comparison](rendering-progression-results-20261101/repeatability-startup-dwell-software-low-comparison-03.json)
matches 7,129 leased swaps / 6,234,310,500 pixels with zero differences. These are
exact phase03 diagnostic results; historical fingerprints are not refreshed.
The earlier [phase01 comparison](rendering-progression-results-20261101/repeatability-startup-software-low-comparison-01.json)
also passes, but contains no source-only chunks.

## Completed frozen v23 nominal matrix

The [complete matrix](rendering-progression-results-20261102/repeatability-matrix-v23r2-results.json)
contains all 12 enabled-intro attempts on frozen phase04/v23 source. All enter
first World. Two pass the full requested defeat route and independent pixels;
ten fail strict assertions. The [case review](rendering-progression-results-20261102/repeatability-v23-matrix-review-01.json)
retains each acquisition, helper refusal, natural report and source closure.

| Configuration | Progression | Victory | Defeat |
| --- | --- | --- | --- |
| Default 800×600 | Natural defeat rejected | World producer cancellation | Helper refusal |
| Default 640×480 | Natural defeat rejected | Natural defeat rejected | Normal Quit, capture truncated |
| Software 800×600 | Natural defeat rejected | Helper refusal | Route and pixels pass |
| Software 640×480 | Natural defeat rejected | Natural defeat rejected | Route and pixels pass |

Software640 [pixels](rendering-progression-results-20261101/repeatability-matrix-v23-software-low-defeat-comparison.json)
match 6,777 leased swaps / 5,926,486,500 pixels over 1,886 original Worlds.
Software800 [pixels](rendering-progression-results-20261101/repeatability-matrix-v23r2-software-high-defeat-comparison-r2.json)
match 6,121 leased swaps / 5,352,814,500 pixels over 1,792 original Worlds.
Movie/unleased paints remain outside comparison. This matrix is not a current
v29 matrix or statistical proof of startup reliability.

The separate skipped-intro software640 [v23 diagnostic victory](rendering-progression-results-20261101/repeatability-victory-software-low-04.json)
completes original 50XP victory, all 21 report texts, original OK, Realm travel to
region2, a fresh World and cleanup through normal Quit. Its [comparison](rendering-progression-results-20261101/repeatability-victory-software-low-comparison-04.json)
matches 11,897 leased swaps / 10,403,926,500 pixels over 3,304 original Worlds.
The nominal failures remain independent negative results.

The initial shared-prefix-family lock refusals are retained. [Four verified
physical seed copies](rendering-progression-results-20261102/repeatability-distinct-prefix-seeds-v23.json)
provide distinct families without widening pool limits. A [scheduler handoff](rendering-progression-results-20261102/repeatability-coordinator-handoff-v23.json)
retired only schedulers after all current children completed; no game/oracle was
suspended or killed. The replacement retries infrastructure refusals and
preserves admitted failures. Its outer retention allowance changes no 900s
scenario, 540s helper or 900s original-oracle bound.

## Current bounded input policies and remaining failures

The [closeout review](rendering-progression-results-20261102/repeatability-closeout-review-01.json)
links candidate source preimages, controls and every original negative result.
V24 misses its 60s rearm-travel deadline. V25 moves atomic cumulative helper
snapshot writes to one worker, with one replaceable pending image and an 8MiB
immutable-byte limit. Blocked-writer tests verify coalescing, final drain and
failure propagation; original passive observations remain authoritative after
cancellation. A real 1.253s write no longer controls the paired W-key cadence,
but this does not prove original key consumption. V25 still times out navigating.

V26 requires exact cleared-bridge coordinates after resupply and reaches them,
but its six Camulos-targeted casts all damage guard5; the wizard later loses.
The [corrected damage review](rendering-progression-results-20261102/repeatability-v26-guard-hit-review-02.json)
finds the sampled guard beyond the target (ray parameter1.3), with no sampled
segment blocker. Interception, retargeting, trajectory and input causes remain
unresolved. Conservative living-actor line checks are authored policy, not a
recovered collision model. V27 has controls only. V28 times out chasing a close
firing posture and never issues the support order.

Current v29 removes that mandatory posture, attempts an ordinary Zombie attack
when the target is visible, requires an original actor-target witness, and
retreats/waits with empty mana after a confirmed order. Exact bridge staging,
fresh role/visibility checks, a single bounded Space pair and cast budgets remain.
Its [nominal software640 attempt](rendering-progression-results-20261101/repeatability-candidate-v29-software-low-nominal-01.json)
reaches the exact bridge at 277.300s with141HP, then naturally loses before any
support order or pause. The support tactic remains unvalidated. Cast admission,
actor death or an order witness never substitute for the natural victory report.
These helper policies do not implement the outstanding native commander feature.

[Current controls](rendering-progression-results-20261102/repeatability-final-controls-02.json)
pass 127 Python, seven Qt startup and 30 Region cases:164 total. This corrected
review distinguishes these fresh guards from the six unchanged-C++ lifecycle
fixtures executed in phase04. Neither controls nor source adoption promote an
engine status or renew historical live evidence.

## Ready bounded scenarios

[Final preparation](rendering-progression-results-20261101/repeatability-final-inputs-01.json)
hashes the selected Celtic map/configuration inputs and pinned No-CD executable.
The unchanged save is a control candidate only; compatibility, position and
save persistence remain unverified. Primary cases always start a fresh campaign.
In the owned prepared checkout, choose `progression`, `victory` or `defeat`, a
fresh report name, `--size 800x600` or `640x480`, and optional `--software`:

```bash
python3 tools/run-rendering-progression.py --evidence-mode full \
  --case victory --size 640x480 --software \
  --prefix-template working/fixtures/prefix-seed \
  --preparation research/runtime/rendering-progression-results-20261101/repeatability-final-inputs-01.json \
  --report research/runtime/rendering-progression-results-20261101/NEXT-UNIQUE-victory.json
```

Run this through the repository storage guard with an explicit task TMPDIR and
6GiB reservation. A different checkout must prepare its own private game,
launch manifest, prefix and build before creating a new preparation report.
Successful acquisition still requires `tools/check-rendering-progression.py`
with the exact acquisition and a new comparison report; semantic success alone
never grants pixels. Timing observation, Main dwell and skipped intros remain
explicit diagnostics and ineligible for nominal acceptance.

Current complete nominal victory/progression, assembled-map/RNG identity,
precise first-frame boundaries, populated object lifetime, simulation equivalence,
rewards/save persistence and supported-player acceptance remain open. All failures,
source phases, corrected narratives and verified local SQLite payloads are retained.
