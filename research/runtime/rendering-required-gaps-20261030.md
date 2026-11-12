# Required rendering gaps — 2026-10-30

Artifact 3 is the `required_gap_register` in the authored
[closure map](../../docs/native-rendering-closure.json). It binds the immutable
[reviewed baseline](rendering-reviewed-baseline-20261030.md) and the exact canonical
fingerprint of [completion contract revision 1](../../docs/native-rendering-closure.md).
Its source subject remains product commit `648ce219`; this batch starts at
contract commit `4dfa2d30`. It changes accounting/tooling, not renderer behavior.
Main-checkout pending edits remain outside this owned batch.

The checked register covers all 14 journeys and their 47 subrequirements,
21 operations and ten completion gates. Each has an explicit remaining obligation
and closing brief. All 78 mandatory leaf/gate assessments are **missing proof**:
the baseline does not establish adequate current complete acceptance. This does
not assert that every component lacks current tests, that every requirement
needs a new implementation, or that 78 independent executions are needed.
There are no newly established mandatory missing-code or missing-integration
findings. Required testing and scope decisions may expose those later.

## What exists and what remains

The table is a reading guide. Exact subrequirements, configurations, behavior and
evidence IDs, bounded interpretations and closing briefs are in the authored
register. Shared evidence is a lead, not whole-journey acceptance. Two target
configurations remain: default OpenGL and the software variant, with exact
provider identity and explicit applicability of reused component proof.

| Journey | Existing implementation/evidence leads | Remaining acceptance work |
| --- | --- | --- |
| Enabled movies | Native movies and enabled-intro player runs | Both intros, skip/end callbacks, output and return/focus/resource retirement |
| Main/setup menus | Native owned menus and setup acknowledgements | Complete current main/setup/cancel route, state and input/output checks |
| Campaign entry | Captured campaign/player World handoffs | Actual selection/loading and independently checked first composed World/history |
| Quick Battle entry | Real player entry on two maps | Actual setup acknowledgement, loading ownership and first World output |
| Active World | Sustained bounded software/hardware player runs | Current progress, animation, visibility/output and resource conservation |
| Camera | Native drawing with retained camera/minimap preparation | Pan, four views, limits and picking after mode changes |
| Creatures/effects | Native sprite/palette/light mechanisms and comparisons | Actual animation, attachments, combat, default lighting and blended order/history |
| HUD/minimap/tooltips | Native HUD/glyph/minimap routes and bounded comparisons | Actual changing HUD, markers/camera, tooltips and font/DC lifetimes |
| Cursor/interaction | Native cursor provider, coordinate/rebuild/lifetime controls | Current motion/hotspot/picking/selection and refused-call diagnosis |
| Pause/preferences | Owned player screens and resumed World runs | Actual pause, both display modes, cancel semantics and fresh resumed output/history |
| Save/Load | Actual saves, Main Load, rebuilt Worlds and emitted-COPY checks | Complete current save/load/restored state and output/history, beyond successful COPYs |
| Map transitions | World ownership/storage and scene-frame mechanisms | Actual later-map progression, retiring old state and presenting the next map |
| Battle results | Report routing and retained bounded battle runs | Actual victory and defeat, plus return to menu/setup with correct ownership |
| Shutdown | Normal Quit and zero cursor/graphics owners in bounded runs | Current World/menu exits, no pending work and conservation across declared configurations |

The 21 operation requirements retain the existing native copy/write/history,
text/image/sprite/palette, caller-state and loss/recovery/failure mechanisms.
Their gaps concern required real callers, independent output, side effects,
returns and lifetime under the target contract. In particular, CPU/GPU COPY
comparisons do not certify fills/UPDATEs or all earlier destination history.
Native recovery and fresh-generation restart already have bounded implementations
and evidence; normal shared presentation, isolated recovery and physical reset
remain separate claims. No blanket recovery rewrite is prescribed.

The ten gates join this evidence with exact source/configuration identity,
contiguous presented output/history, input/media, transitions, normal/fault
retirement and justified supplier boundaries. A static inventory cannot pass
these gates. A separate full native scene/simulation/actor port is not required
merely to close native pixel ownership.

## Scope decisions that can change required work

All eight remain unresolved. Each keeps its original conditional blocking rule
and exact close-when clause from the completion contract; none becomes a blanket
blocker or an automatic exclusion.

| Decision | Required determination |
| --- | --- |
| `SC.alternate-ownership` | Required natural alternate allocation/publication/writer/lifetime chain, or precise justified boundary |
| `SC.padded-fullscreen` | Whether the claim includes padding/fullscreen and which actual output/transition/lifetime proof it needs |
| `SC.natural-callers` | Required raw readers, self/keyed GPU copies, aliases/writers and large fills; zero observations do not prove absence |
| `SC.prior-history` | Required preceding pixel history and its relevance to current resident output |
| `SC.map-save-domain` | Explicit supported campaign/map/save/duration domain, retaining later progression and both results |
| `SC.screen-provider-lifetime` | Required real Main/Realm/World/Report routes, resources, callbacks and failure/retirement semantics |
| `SC.rgb555-live` | Whether a required live path emits RGB555, then validate it or justify the exact format boundary |
| `SC.physical-compatibility` | Exact Wine/native-provider versus physical/legacy-platform claim and appropriate evidence |

## Retained negative evidence and optional diagnostics

`OBS.cursor-refusals` preserves `PLAYERMERGE27.storage01`: all 4,188 successful
emitted COPY outputs matched, but two failed/unprepared calls prevented whole-COPY
closure. This is an older recorded source phase, not a newly reproduced defect.
Establish current caller correctness/relevance before prescribing a code fix;
weakening geometric admission to erase refusals is not acceptance.

`OBS.late-history` preserves `RRG26.natural-native`: same-zero-base native/original
output and original-before/live diagnostics matched, while native/live equality
did not. That comparison was not a fresh live execution and did not feed original
pixels into native rendering. Recover the required preceding history and establish
current player relevance before calling it a current resident regression.

Timing is optional diagnosis. The historical approximately 19.9 fresh-World/s
tracing measurement is not a current player result or a completion FPS floor.
Responsiveness, progress and resource bounds remain mandatory elsewhere.

## Checks and next artifacts

`tools/rendering_gaps.py` checks complete denominators, unchanged contract/baseline
bindings, required configurations, closing-task references, classifications and
retained scope conditions. The CLI verifies immutable lead payloads/assertions,
keeps baseline and current source-binding states distinct, and copies bounded
behavior-stage support without turning freshness into acceptance. Leads outside
the selected rendering baseline are labeled explicitly. No engine status changes.

```sh
python3 tools/reconcile-rendering-dependencies.py --gaps-only --run-guards \
  --report research/runtime/rendering-required-gaps-results-NEW/review.json
```

Use verified local SQLite evidence and the storage guard. The checker returns
before original media/byte analysis; guards do not launch the game, GL or hardware.
Generated reports use SQLite references. Exact review receipts and verified local
backups preserve this batch separately from the main checkout.

The [final static review](rendering-required-gaps-results-20261030/review-02.json)
records the checked inventory and evidence leads. Fourteen gap guards, twelve
completion-contract guards, twelve admission guards and eighteen mocked launcher
guards pass. The initial review retains its earlier source phase; exact
scope-condition copying is additionally enforced in the final phase. The
[contract-commit history review](rendering-required-gaps-results-20261030/contract-history-01.json)
accounts for committed transitions since the preceding review; it does not assert
a retrospective gate pass or fresh engine validation.

The 36 closing briefs are unordered requirement groupings. **Artifact 4 still
needs to choose the shortest dependency path:** decide requiredness first, reuse
applicable existing evidence, join compatible acceptance journeys and schedule
only concrete missing implementations exposed by that review. No effort estimate
or optimal execution order has been established here. **Artifact 5 still needs
the executed acceptance record and separate supported-player/original-recovery
decisions.** This gap register cannot substitute for either.
