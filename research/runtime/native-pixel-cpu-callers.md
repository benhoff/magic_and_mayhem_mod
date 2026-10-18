# Optional raster callers under an authored CPU context

The retained physical-host journey executes 31/43 optional producer routes.
Seven absent routes (`raster/splice0`, `1`, `4`, `5`, `11`, `12`, `14`) belong
to the raw base-family-other-than-6 dispatch selected by original `0x484470`.
The host reports base family 6. Isolated original selection and individual
pixel comparisons already exist; neither proves execution by game callers.

`tools/test-raster-cpu-game.py` separates debugger controls from a live game
acquisition. A test-owned PE32 debugger starts the already hash-checked staged
game through the existing runner. Three instruction hardware breakpoints observe
the original initializer entry, `0x484604` immediately after its helper's
CPUID.1, and the initializer return at `0x48458a`. The sole authored CPU change
is EAX bits 8..11 (raw base family), with other CPUID registers unchanged.
The original helper and dispatcher compute their own object fields and targets.
The debugger has no `WriteProcessMemory` import and does not patch instructions,
dispatch pointers, drawing inputs or counters. It clears each hardware breakpoint
after observation, checks all 561 pinned initializer/sibling/helper bytes at all
three stops, and retains a bounded 256-byte proof with runtime process/thread,
caller/object identities and original selected targets. Those identities are
observed per process, not assumed stable across launches.

This is an authored CPU platform experiment. Successful game callers establish
reachability under that context; they do not establish execution on a physical
alternate CPU, unchanged original driver/pixel output, or full-session parity.
The physical-host 31/43 result remains historical and independent. The other
five unresolved entries are outside this experiment and remain in the denominator.

Before live acquisition, the debugger must pass PE32 controls for base families
6 and 15, changed-code rejection, skipped-initializer rejection and stale-output
refusal. The fixture maps and executes the unchanged original initializer with
owned data pages; it cannot add game-route credit. Acquisitions use prospective
claims, new report paths, original-manifest bookends, a stopped cloned private
Wine prefix and the main storage guard.

## Observed game callers, 2026-10-17

The [scoped actual-game result](native-pixel-cpu-callers-20261017.json) observes
all seven alternate bodies with zero forwarding/history refusal. The original
initializer is called from return PC `0x4e8fbf`, reads the authored raw family
15 and computes its own four targets. All 561 pinned bytes match at its entry,
post-CPUID and return observations; the actual game child exits with code zero.

| Original target | Native routes | Calls in final default-policy journey |
| --- | --- | ---: |
| `0x5947b2` | splice0 / splice1 | 3,108 / 346 |
| `0x595b47` | splice4 / splice5 | 325,932 / 124,973 |
| `0x597086` | splice11 / splice12 | 3,419 / 713 |
| `0x5968a4` | splice14 | 1,139 |

The nonresident Apprentice journey completes both ten-second physical input
phases, Mini Cancel/resume, normal Quit and private Wine shutdown, retaining the
5 FPS software floor. It records 914,079 native calls and 31/43 exercised routes.
Together with the separate physical-host 31/43 result, **38 distinct routes**
have now executed. This is a union across CPU contexts. The physical-host result
remains 31/43; the 43-route denominator is unchanged.

The five unresolved natural callers are `pixel/pcx_words`, `pixel/brighten`,
`pixel/row_copy`, `raster/splice15` and `minimap/camera3`. Existing entry/pointer
searches find no confirmed callers for the first four. The reviewed camera
dispatcher selects camera0/2/0/1, leaving camera3 unselected there. Indirect or
outside callers remain unknown. No dispatch slot or camera table was forced.

[Final-source PE32 controls](native-pixel-cpu-controls-20261017.json) pass both
CPU branches and changed-code/skipped-initializer/stale-output rejection. Ten
Python tests additionally check proof corruption, collateral CPUID changes,
direct/batch launch admission, preserved original batches, and explicit RGB
policy selection. The compatibility launcher uses `start /wait`; the adapter
retains its original batch and creates a separately fingerprinted CPU-debugger
batch. The debugger reports success only after its actual game child exits
normally and all three original-code observations complete.

## Scope and retained failures

The producer/game witness is frozen at `ac848056` with the declared observer and
RGB gate changes. Current main-checkout CPU-guard and merged-native-AI work is
separate. The scoped report names source differences instead of refreshing its
historical hashes. Original game callers are observed in that checkout; complete
current-renderer integration, original pixels and physical alternate hardware
remain unclaimed. Counters do not identify every higher-level indirect caller.

The first correctly instrumented journey reaches all seven bodies but fails on
two RGB-add forwards. An enabled bounded geometry diagnostic replay does not
reproduce them and passes; their exact inputs/cause remain unassigned. Independent
original execution confirms a rejected empty-horizontal RGB branch. The
[candidate repair](rgb-add-empty.md) passes protected-memory/caller comparisons,
but two subsequent ungated acquisitions stop in presentation with producer
reason 2 before completion. Their cause is unassigned. Empty native admission is
was therefore explicit default-off (`MNM_PIXEL_EMPTY_RGB_NATIVE=1`), and that historical final
default-policy journey above passes without it.

The [immutable acquisition catalog](native-pixel-cpu-callers-provenance-20261017.json)
retains setup/build refusals, the initially bypassed batch-launch attempt,
concurrent-source exclusion, forwarding and presentation failures, original
manifest bookends, exact raw reports and source fingerprints. No failed
acquisition is converted into a successful full campaign. The corrected
[incoming history](coverage/producer-cpu-callers-incoming-history-final-20261017.json)
and [merge receipt resolution](coverage/producer-cpu-callers-merge-history-resolution-20261017.json)
are accounting reviews, not new engine validation or historical gate passes.

## Reproduction boundaries

Use an isolated checkout with the registered helper/test sources, a verified
supported `working/game-nocd`, and a stopped admitted prefix template. Build the
native shell before preparing the game declaration so its compiler dependency
closure can be included. Select `NP.raster-cpu-game-callers`, its campaign
scenario, and the normal nonresident campaign's menu/startup, counter/history,
route and prefix claims. Add `test-native-campaign.py`'s `SOURCES` and
`linked_compiler_sources()` to that prospective declaration before execution;
the ordinary campaign refuses missing compiler dependencies. Do not refresh a
declaration after its acquisition.

Run `tools/test-raster-cpu-game.py --mode controls` first, then `--mode game
--family 15`, with separate new `--claims`/`--report` paths and
`--prefix-template`. The game tool retains the ordinary campaign acceptance:
Apprentice, two ten-second physical input phases, Mini Cancel/resume, normal
Quit, 5 FPS software floor, zero optional forwards/history refusal and explicit
execution requirements for all seven alternate raster splices. It does not
enable resident World batching, which bypasses the CPU interiors being tested.
Use `--rgb-diagnostics` only for a separate bounded geometry observation and
`--empty-rgb` only to opt into the candidate no-pixel policy. Neither changes
the campaign's acceptance checks. Default runs remove ambient RGB diagnostic
or candidate-policy selectors before starting their owned game.

If shared `/tmp` is over its user quota, run the acquisition in an unprivileged
user/mount namespace with a new checkout-backed directory bound to private
`/tmp`; keep the main checkout's retention guard outside that namespace. This
also keeps Xvfb/Wine temporary state task-owned. The host mount and existing
shared temporary data must remain unchanged. The original public display is
never used for this experiment.

## Accounting boundary

Later shared campaign-runner edits make the acquired CPU controls historical.
The helper's implementation status therefore stays partial, with current-source
validation pending. The actual frozen-game caller witness remains independent.
The RGB candidate retains its fresh isolated original comparison through the
executed fixture's exact source closure; unexecuted build-only dependencies in
the broader acquisition remain historical.

The full checkout audit/gate is blocked by eight empty-evidence scenarios belonging
to ongoing active-lock-reader and protected-CPU-lease work. A copied, read-only
task accounting view excludes precisely those eight rows from validation and
reindexes only that stable copy. The actual shared register preserves those
rows. This view is used for exact task receipt preparation, never as a full
checkout, staged-tree or CI gate pass. Concurrent source-census drift remains
explicit in the full report.

## Current-code refresh, 2026-10-18

The [all27 follow-up](native-pixel-route-27-followup.md) supersedes the preceding current-source pending limitation for this policy with a new prospectively declared authored-family15 original-game witness. It executes917,672 native calls, all seven alternate requirements and31/43 routes, zero forwards/history refusal and normal Quit. The accepted physical-host refresh executes1,297,960 calls and31/43 routes; their union remains38/43, with five unresolved natural callers. All27 formerly absent cohort routes additionally pass fresh original/native controls in both formats. This promotes only the CPU caller policy to bounded live_observation; physical alternate CPU, original pixels, resident history and complete replacement remain unclaimed. Earlier immutable results and the historical accounting-view boundary above are preserved. The eight empty scenarios mentioned above were a historical checkout condition, not the state asserted by this refresh.

## Current-source observation, 2026-10-18

Fresh six-case PE32 controls include a fourth hardware-breakpoint RGB observer
and stale RGB-proof rejection; seventeen decoder/selection tests pass. The
256-byte CPU proof is unchanged. The optional 512-byte RGB proof is a separate
versioned record. Source-bound default-policy journeys pass normal Quit, both
ten-second phases, the5FPS floor and zero forwarding/history refusal:
763,089 native calls under authored raw-family15, and876,981 on the physical
family6 host. Each still executes31/43 routes; their union remains38/43.
Physical CPUID registers before/after observation are identical. Authored-family
results remain a platform experiment, not physical alternate-hardware evidence.

The [current alternate](rgb-empty-default-alternate-accepted-20261018.json) and
[physical](rgb-empty-default-physical-accepted-20261018.json) records include
matching read-only RGB epochs, with zero empty calls. The [default no-op policy](rgb-add-empty.md)
passes512 isolated cases plus8 opt-outs; live empty-branch integration remains
open. The [PE/module boundary review](pixel-route-entry-boundaries.md) provisionally
classifies the five missing callers as unused candidates, retaining all43
routes, unresolved computed/external flows and the implementations. The separate
[camera-stress boundary](raster-displacement-physical-boundary.md) failed
acceptance and is not folded into these successful standard journeys.

## Current finite-raster renewal (2026-10-18)

[Fresh PE32 controls](finite-cpu-displacement-renewed-debug-20261018.json) and the [fresh original-game witness](finite-cpu-displacement-renewed-game-20261018.json) prospectively renew the unchanged CPU/RGB caller contracts after the shared raster source change. The latter passes1,347,324 native calls,zero forwards/history refusal,all seven alternate requirements and normal Quit, with1,776 exact RGB observations and zero empty calls. Its finite-edge diagnostic observes6 calls/17 retained WORDs without changing dispatch, game code, drawing inputs or counters. Separate finite-policy stage credit requires its own prospective scenario. Physical alternate CPU, original pixels and five unused-candidate callers remain unclaimed.
