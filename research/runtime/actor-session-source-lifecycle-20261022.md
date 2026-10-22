# Actor session and source ownership

The native `ActorSession` API composes actor pool construction, preloaded actor
activation, actual source registration/grouping, actor release and cleanup,
source pruning, slot reuse, shutdown and a second session. Actors, map occupancy,
source nodes, three pending queues per source family and allocations share one
PE32 value-address store. There is no separately synchronized logical graph.
The original owner/planner schedule and concrete task/resource providers remain
explicit inputs. This closes the bounded owned lifecycle composition; it does
not implement the full original game session.

## Recovered control and ownership

`actor_source_services.hpp` implements fourteen source/collection entries:
`407920`, `4079f0`, `416590`, `416fe0`, `514810`, `5183f0`, `51e790`,
`42afd0`, `429900`, `407780`, `407820`, `407650`, `417e60`, `417580`.
The graph retains the original eight recursive traversal slots, signed mode and
activity gates, round-robin fallback, child-vector cursor relocation, owner and
network gates, terminal roots and recipient handoff. Registration deduplicates
and pushes to the general, wizard or other pending queues. Queue capacity work
includes initial backing, a spare map slot, single-block map rotation,
multi-block map growth and block crossing. Pruning removes all queue/work-vector
occurrences, clears actor links, detaches children and destroys retired sources.

The primary manager is `690600`, secondary `644550`; family-specific actor roots
are `+678/+67c`. Source objects occupy `8d` bytes: recipient `+5`, child vector
`+29/+2d/+31`, actor `+35`, parent `+39`, rotation `+3d`, free-slot bytes `+41`,
cursors `+49`, nested traversal `+69`, activity `+89`. Protected manager roots are
`+11daf/+11fc8`; work vectors are `+11db3/+11de5/+11e05`. A source recipient's
`+8` backlink moves to a newly formed group and the old leaf link is cleared.
Recipients on ancestors protect retired descendants from collection.

Source grouping uses R250 receiver `6a29d8` (instruction `4166e2`), independently
of particle `6574c8` and action `6e1ac8` streams. All250 state words and the pinned
successor table `5eb940` are retained in the fixture. Original object byte `25`
comes from the caller/traversal/coordinate stack. Its machine provenance is
checked separately at allocation. The native opaque tag is deterministic; only
that byte is excluded from allocated-byte equality. This is a representation
boundary, not recovered tag equivalence.

`actor_transition_services.hpp` recovers `50aa30` and its callback selection:
health/motion gates, old marker retirement, behavior/pending state and the
64-bit callback field are published before the selected handler runs. Handler
bodies remain named supplied endpoints. `actor_session.hpp` adapts the existing
pool constructor, record constructor/reset/destructor and pool destructor to the
same allocator/dispatch, including the selected typed deleting callback.
The router now extends the prior149 routes with22 routes.

## Intentional native orchestration

`NP.actor-session-source-ownership` records the public session policy separately
from `RS.actor-session-source-lifecycle`. The API checks phase and owned live
slots, refuses reuse while either source family retains a slot, and refuses pool
destruction while sources/queues/work vectors remain. Reusing a record first
destroys and reconstructs it, then resets it; reset alone would overwrite owned
histogram backing. After recovered pruning reaches quiescence, native close frees
empty source vector backing and destroys the pool. Close can be retried after a
provider releases protection and the API supports another begin/close cycle.
These choices are native policy, not the original higher-level shutdown cadence.

## Validation and reproducibility

Pinned original NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Read-only decompilation, instruction/ABI exports and raw runs are retained under
`working/tests/actor-session-registration-20261022/` in the main and owned task
worktrees. Original-consuming producers verify the immutable manifest before
and after execution. Selected original function bodies and ABI bytes are guarded
before redirection and throughout checkpoints. Only named external ends are
redirected; native outputs are never seeded from original outputs.

The final matrix uses64 independently seeded two-session journeys with16
checkpoints each, compared across original32-bit execution, native64-bit execution
and native ASan/UBSan execution. Checkpoints serialize owned allocation contents
(including retired blocks), source/actor/map state, selected globals and ordered
callback arguments with callback-time state. Every admitted journey must return
normally with zero live owned allocations. Forked child `_exit` means host LSan
teardown is not claimed; the fixture's allocation ledger checks complete owned
retirement. Authored owner availability, secondary-family registration, task
recipient inputs and32 imported fragmented deque layouts are explicit inputs,
not a recovered original planner. Recipient tests check two-way group transfer,
ancestor retention, recipient-only shutdown refusal and successful release.
The two mandatory policy refusals per journey retain the pool and prevent slot
reuse without allocating.

The shared fixture include was made safe for nested use. All eleven pre-existing
service matrices are rerun after that change, preserving historical fingerprints.
Generated reports and prospective unique source changes are SQLite payloads with
Git references under `research/runtime/actor-session-source-results-20261022/`.
Earlier attempts and failures remain retained, including the fixture allocator
64KiB-boundary overlap fixed before the final matrix. Old results are not rehashed.

Run with a new output identity:

```sh
python3 tools/working-retention.py run --reserve-gib .7 -- \
  python3 tools/test-actor-session.py --output working/tests/actor-session-new
```

`tools/reconcile-actor-session.py` checks exact routing and records each historical
semantic lead as a concrete route, reviewed embedded helper, supplied provider or
unresolved composition. A link alone does not complete a caller or transitive
frontier. Static accounting grants no extra execution or indirect-dispatch credit.

## Remaining boundaries

The Critical downstream umbrella remains partial. Arbitrary supported installed
profiles, original owner/planner cadence, concrete attack/task handlers, font and
resource decoding/raster/acceptance, scenario evaluator/action subtypes and other
indirect providers need their own recovery and integration. External platform
acceptance and failure/lifetime behavior, original-world/live replacement,
physical legacy driver equivalence, malformed CRT partial-copy faults, Windows
SEH and throwing cleanup remain separate. Source registration in this session is
a concrete recovered implementation; the older service matrix's source endpoint
remains an intentional historical boundary. No gameplay balance changes are made.

The accepted final execution is `final-1.json`:393,604,952 compared bytes,
1,024 checkpoints,16recipient transfers,16recipient-protected reuse refusals,
32recipient-only shutdown refusals and128mandatory protected-pool/reuse refusals.
The earlier matrix6 execution matched but its result aggregation had a filename
error and retained an inconsistent success flag; it is diagnostic history only.
The final producer corrects the aggregation and failure status. Routing review
accounts for the43historical leads as12concrete routes,19embedded helpers in
their stated valid-input scope and12remaining supplied providers. These are
static dispositions, not43newly validated full functions.
