# Native rendering completion contract

Contract `native-rendering-completion-v1`, revision 1, reconciled 2026-10-30
against product source `648ce2191c6b5c5d5b18a2508dcddc2ee6fec7d7` and the
[reviewed baseline](../research/runtime/rendering-reviewed-baseline-20261030.md).
This defines the target and required proof. **Supported-player acceptance and
original recovery remain pending.** A reviewed contract is not a release result.

The [authored closure map](native-rendering-closure.json) is authoritative for
this contract, 15 dependency groups, 106 linked behaviors, 14 journeys and 21
operation/failure requirements. Its `completion_contract` section supplies the
versioned configurations, completion gates, supplier rules and unresolved scope
decisions. Existing historical execution reports and hashes are unchanged.

The [required gap register](../research/runtime/rendering-required-gaps-20261030.md)
and the map's `required_gap_register` section assess every journey subrequirement,
operation and gate against that baseline. They preserve evidence leads, eight
conditional scope decisions and unordered closing briefs; they grant no new
acceptance or execution order.

## Fixed target

The target retains original No-CD single-player simulation and permitted
preparation while native rendering owns visible pixels and required history.
The original executable SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Native simulation/task-service replacement and gameplay balance changes remain
outside this milestone. Retail and multiplayer support require separate evidence.

The primary player uses `native-only-rendering-v1` through `run-native.sh` or
`run-x86.sh`, which delegates to `tools/run-native-owned.py --native-only`.
Its target environment is x86 Linux; baseline executions use x86_64, Qt/X11
(xcb or XWayland), OpenGL and the original PE32 process in Wine on a separate
private X server. This is not an all-architecture, all-driver or legacy-Windows
compatibility claim. Exact host, backend/driver, display and asset/source versions
must accompany each accepted configuration.

## Configurations and profile boundaries

These are acceptance targets, not newly certified configurations.

| Configuration/profile | Entry or selection | Required boundary |
| --- | --- | --- |
| Primary default player | `./run-native.sh` or `./run-x86.sh` | Strict native pixel ownership; RGB565 World input; native terrain lighting with capture disabled; native enabled movies. Both 640x480 and 800x600 frames and display rebuild/rebind are required. Record actual host OpenGL provider and music/voice selection. |
| Primary software variant | `./run-native.sh -- --software-rendering` | The same strict ownership and journey/operation requirements on the actual Mesa software provider. Software output does not certify physical NVIDIA presentation. |
| Public resident harness | `tools/test-native-campaign.py --fps-policy diagnostic` | Records its actual policy/backend. A harness pass is not automatically a strict-player pass or independent complete presented output. |
| Isolated recovery | `tools/run-native-owned.py --device-recovery`; add `--native-only` for a complete-native claim | Separate replay/retry/raster/IPC and fault-continuation evidence. It does not certify ordinary shared-texture presentation or physical hardware reset. |
| Explicit lighting/media variants | `MNM_TERRAIN_LIGHTING=original` or `observe`; admitted skip/music options | Identify the changed preparation/media configuration. Original lighting is permitted preparation, with no native-lighting replacement credit. Disabled movies cannot close enabled-movie requirements; disabled music cannot claim enabled music. |
| Original renderer compatibility | `./run-x86.sh --original-renderer` | Separate diagnostic backend; no native-pixel takeover credit. |

The public launcher defaults to native terrain lighting. A native worker failure
may retain original lighting for continuation, but its failed/nonzero outcome
must remain a failure. Selecting original/observe explicitly is a distinct
preparation backend, not original pixel fallback. Music/voice backend choices
must be recorded; full audio-engine replacement is not added to this renderer
contract.

The strict CLI currently rejects startup `--fullscreen`. Original padded
fullscreen failures and any claimed native fullscreen/padding transition retain
separate requiredness/output checks. Native padded-storage fixtures and the
opt-in original allocation repair do not settle original alternate ownership.
ARM/Asahi, other original builds, RGB555 offline previews and broader platform
support retain their separate evidence boundaries.

## Ownership and permitted original suppliers

The [reviewed dependency decisions](native-rendering-dependencies.json) define
which services may remain original. Camera/map/traversal preparation,
sorting/visibility/animation selection, queue preparation, screen coordination,
serializers and validated table/configuration production may remain where those
decisions permit. Their required mutations, ordering, lifetime and failure
behavior still need accounting. Native surfaces, pixel operations and required
history must remain owned; original dynamic rasterization/GDI output cannot
supply the completed frame within a complete-native claim.

Porting the separate `SceneSession`, replacing every actor/task service, or
installing a standalone camera/minimap/screen model is not automatically a
prerequisite for native pixel completion. A required unknown supplier still
blocks its claim until recovered or resolved by an exact justified boundary.

## Required completion proof

All existing 14 journeys, their subrequirements and 21 operations remain.
Required proof binds actions and contiguous presented frames to appropriate
independent references, checks caller-visible side effects/returns, and verifies
resource conservation. Component canvases, recorded intervals, original-only
observation, admission receipts and clean exit codes keep their own limits.
Native Qt menu layout and safer admission are intentional policies with semantic
checks; they do not claim original UI/driver pixel identity.

Normal journeys require normal original Quit, balanced World delivery and
retired cursor/graphics/process/channel owners. Fault journeys instead check the
declared terminal-stop/fresh-generation or isolated-continuation policy, expected
failure and no duplicated work. Force-stop is not normal completion. Unexpected
refusals block normal acceptance; declared negative cases require their expected
return, output/state and lifetime checks and cannot disappear from accounting.

Each claimed configuration needs appropriate current source/input-bound evidence.
Shared component evidence can be reused only with reviewed applicability; the
contract does not invent an automatic Cartesian product of every option. The
supported map/save/duration domain must be stated without waiving actual later-map
progression, victory/defeat or the other fixed requirements. No universal-duration
or arbitrary-map claim follows from the retained bounded runs.

FPS remains diagnostic under the user decision of 2026-10-25. No fixed FPS floor
is introduced. Functional response deadlines, forward progress and resource
bounds remain mandatory; historical benchmark failures stay unchanged.

## Open scope decisions

Eight stable decisions remain explicit in the authored contract: original
alternate ownership, padded fullscreen, natural callers/readers/writers, prior
pixel history, supported map/save domain, screen/provider lifetimes, live RGB555
requiredness and physical/platform compatibility. Each names its affected groups,
blocking condition, research and closing evidence/boundary. No zero observation,
fixture, safe refusal or unavailable host implicitly excludes a required path.

Original recovery closes only within its named build/provider scope with required
contracts accounted for. Supported-player acceptance closes only with appropriate
current journey/operation/output/lifetime/fault evidence and no unresolved required
blocker. These decisions can finish separately. Neither this contract nor a static
checker grants either decision.

## Check the authored contract

```sh
python3 -B tests/test-rendering-completion.py
python3 tools/reconcile-rendering-dependencies.py --contract-only --run-guards \
  --report research/runtime/rendering-completion-contract-results-NEW/review.json
```

The contract-only mode verifies the immutable baseline, unchanged requirement
denominator, 31 source witnesses against the product revision, runtime-policy
selection and explicit scope decisions. It reads no original media and performs
no engine, GL or gameplay execution. Generated reports go directly to SQLite.
Source/policy drift requires an exact contract review, not refreshed historical
execution hashes. Any amendment must preserve explicit scope-change accounting.

The detailed groups, journeys and dated execution notes below retain their
recorded scopes. A dated statement of current validation does not become current
at this reconciled source merely by appearing in this document; use the baseline
and exact execution bindings for freshness.

## Bottom up original contract closure

For each group, identify the required original callers, dispatch branches and
providers. Record build/address links, inputs, outputs, mutations, ordering,
ownership/borrowing, failure/retry paths and confidence. An authored service
callback can establish the caller's ordering but cannot establish the callee's
semantics. Native policies remain separate from recovered baseline behavior.

| Group | Existing scope | Remaining recovery or connection boundary |
| --- | --- | --- |
| C01 Surfaces | Owned facade; formats, Lock/DC, aliases and held-flip fixtures | Required original callers, external aliases/writers and failure/release ordering beyond bounded providers. |
| C02 Pixel history | Ordered restoration, protected backing, GPU copy plans and selected live destinations | Required raw readers and natural clipped/self/keyed GPU-copy callers. |
| C03 World queue | Native dispatch/raster, continuous delivery and bounded live rendering | Complete required producer dependencies, mutable ownership and installed composition. |
| C04 Terrain and camera | Bounded map/camera/section/submission implementations; traversal and generated previews | Original preparation may remain; account for required construction/update/resource lifetimes and effects without making a full native camera port mandatory. |
| C05 Scene producers | Concrete terrain/water/picking, creature/effect/artifact and particle contracts | Runtime suppliers, entity/resource churn and full caller/ownership composition. |
| C06 Lighting and palettes | All ten bounded palette builders, chains, selection/delivery, previews and selected light contracts | Runtime configuration, complete refresh/input production, failure and scene ownership boundaries. |
| C07 Sprites and visibility | Sorting, visibility, occlusion and selected sprite/effect comparisons | Complete producer/owner mutations at required scene call sites. |
| C08 HUD minimap text | Pixel kernels, text/finalizer composition and compared minimap selector | Whole-caller state production, border/marker composition and installation of the new selector. |
| C09 Screens | Complete local tick,borrowed dispatch and cursor-background helper comparisons;real native cursor-copy provider connection | Remaining helper/vtable semantics,screen lifetime and callback interactions. Portable helper installation is separate while original screen control remains a supplier. |
| C10 Alternate ownership | Initializer/copy/displacement contracts and normal original session observations | Allocator, publisher, initial pixels, base/source relationship, writers, release and requiredness; native displacement initializer is not installed. |
| C11 Input | Forwarding and selected physical interactions | Required state transitions, cursor/hotspot/picking combinations and strict Escape/Mini readiness. |
| C12 Movies | Native playback and bounded enabled startup | Required skip/end/error/return ownership and profile combinations. |
| C13 Storage transitions | Save/Load, display rebuild and fresh resumed Worlds | Later-map progression and wider rendering resource retirement/rebind. |
| C14 Shutdown and recovery | Original provider shutdown, actual-game Hard restart, synthetic graphics recovery | Natural failure reachability, external credits, live isolated continuation and physical-reset boundaries. |
| C15 Admission and acceptance | Strict admission, transport controls and recorded software/NVIDIA comparisons | Whole journey coverage, required routes and current-source functional acceptance; performance is diagnostic. |

The machine map supplies exact behavior IDs, source/test witnesses and research
links for each group. It is a required-dependency starting map, not an assertion
that every registered rendering diagnostic is a mandatory replacement. B01 must
reconcile the complete required frontier; any newly discovered required behavior
gets an explicit link and scope-change review rather than disappearing between
these groups.

## Top down journey closure

Each journey keeps every existing requirement from the suite. Close it with
appropriate independent original references, current native execution, bound
actions, contiguous frame identities and required semantic/lifetime checks.
Recorded replay, component output and live original-only observations retain
their own boundaries. Intentional native UI policies use semantic acceptance.

| Existing journey | Contract dependencies | Specific remaining acceptance |
| --- | --- | --- |
| Enabled movies | C01 C02 C09 C11 C12 C14 C15 | Both intros, skip/end and menu return in each claimed profile. |
| Main and setup menus | C01 C02 C08 C09 C11 C14 C15 | Settings/cancel semantics, focus and presentation ownership. |
| Campaign entry | C01 C03 C04 C05 C06 C07 C08 C09 C11 C13 C15 | Required admission/loading branches and independent first complete output. |
| Quick Battle entry | C01 C03 C04 C05 C06 C07 C08 C09 C11 C15 | Setup/loading/first-World actions bound to the claimed native profile. |
| Active World | C01 C02 C03 C04 C05 C06 C07 C08 C09 C15 | Sustained complete output, side effects, forward progress and resource limits together; FPS is diagnostic. |
| Camera motion | C03 C04 C05 C06 C07 C08 C11 C15 | Pan, all four orientations, limits, minimap and picking together. |
| Creatures and effects | C01 C02 C03 C05 C06 C07 C08 C15 | Required attachments, lighting, visibility and blending with semantic/frame checks. |
| HUD minimap tooltips | C01 C02 C05 C07 C08 C09 C11 C15 | State-bound markers, outlines and tooltip open/close in presented output. |
| Cursor interaction | C01 C02 C05 C08 C09 C11 C15 | Hotspot, picking and selection across scene/menu/storage transitions. |
| Pause and Preferences | C01 C02 C06 C08 C09 C11 C13 C14 C15 | Strict movement-to-Mini failure plus pause/cancel/apply/display combinations. |
| Save Load | C01 C02 C03 C04 C05 C06 C08 C09 C11 C13 C14 C15 | Current profile actions, fresh resource identities and post-load output. |
| Map transitions | C01 C02 C03 C04 C05 C06 C07 C08 C09 C11 C13 C14 C15 | Actual next-map progression and retirement; loading an existing save does not close it. |
| Battle results | C01 C02 C08 C09 C11 C13 C14 C15 | Both victory and defeat, return actions and native resource teardown. |
| Shutdown | C01 C02 C03 C09 C11 C12 C13 C14 C15 | Normal, failed and restarted generations with process/channel/native-owner retirement. |

C10 is a conditional dependency of active World, camera motion, Save/Load and
map transitions until B02 resolves its requiredness. It is not silently admitted
or excluded by the ordinary-path links above.

All 21 existing operation/failure requirements also map to groups in the machine
map. A completed journey cannot silently waive a required copy, text, palette,
history, restore or cleanup branch. Conversely, a kernel comparison cannot close
a journey's action or presentation requirements.

## Finite blocker list

Classifications are independent: **unrecovered** means missing original semantics;
**unimplemented** means a required understood native behavior lacks code;
**not integrated** means the required caller is disconnected; **not validated**
means missing appropriate acceptance; **known failing** means a retained check
fails. A scope decision does not erase an unresolved contract. No blanket
unimplemented label is applied to the completed bounded components.

| Blocker | Classification | Exit |
| --- | --- | --- |
| B01 Required route/dependency census | Unrecovered and decision required | Account for entry points/providers for all journeys, including allowed original suppliers and unknowns. |
| B02 Alternate ownership | Unrecovered and requiredness unresolved | Recover the required natural ownership chain or justify an exact bounded exclusion; global reachability remains unknown. |
| B03 External screen services | Unrecovered | Recover required helper/vtable behavior and screen/callback lifetime beneath the local tick. |
| B04 Required scene/helper connections | Not integrated | Connect required providers and prove caller effects, ownership and output; name allowed original suppliers. |
| B05 Strict camera/HUD input Escape/Mini | Historical bounded passes; current combined acceptance pending | Retain prior source-specific results and obtain appropriate current callback/resume/Quit, output and ownership evidence. Historical cause remains separate. |
| B06 Public FPS diagnostics | Diagnostic; historical benchmark failed | Retain the measured result and instrumentation limits. No fixed FPS threshold blocks functional completion. |
| B07 Action-bound acceptance | Not validated | Appropriate evidence for all existing 14 journeys and 21 operations. |
| B08 Transition combinations | Not validated | Movie returns, Preferences variants, actual later maps and both results with fresh resources/output. |
| B09 Natural caller/history coverage | Unrecovered and not validated | Required natural caller/output proof or justified exact exclusions for unresolved routes and reader/copy branches. |
| B10 Actual gameplay fault matrix | Not validated | Profile-specific terminal restart or isolated committed-state continuation with bounded resources and no duplicate work. |
| B11 Physical compatibility | Unrecovered and not validated | Keep legacy-driver/reset claims blocked without actual evidence; declare any bounded provider compatibility explicitly. |
| B12 Fullscreen padded rectangle | Known failing configuration | Correct and compare presentation before claiming enabled-padding fullscreen support. |
| B13 Current source/reference bindings | Not validated for complete scope | Verify actual transitive sources/inputs and rerun affected acceptance after drift; preserve historical results. |

The [NT17 execution](../research/runtime/rendering-seventeenth-five-20261023.md)
retains B05's two readiness failures and B06's public outcome of
17.253776873910518 FPS against its historical19FPS gate. These are retained
failures, not newly rerun diagnoses or proof that the renderer caused them.
Current strict10/30second B05 cases now pass; the historical ingress cause remains unclassified. B06 is retained as an instrumented diagnostic
under the2026-10-25 user decision; it is not relabeled a passed benchmark.
Its successful4096-return controls do not close those different scenarios.

The [ownership sessions](../research/runtime/alternate-ownership-sessions-20261024.md)
cover eight original windowed processes and sixteen battles with no alternate
publication. They do not close B02. The
[isolated recovery controls](../research/runtime/native-isolated-render-recovery.md)
do not close physical-reset or live-game continuation acceptance.

## Work packages and completion decisions

| Package | Track and dependencies | Reviewable completion |
| --- | --- | --- |
| P1 Required original dependencies | Bottom up; can start now | Required caller/provider graph, recovered contract receipts and exact remaining unknowns or justified exclusions. B01 B02 B03 B09 B11. |
| P2 Required provider connections | Implementation/integration; depends on P1 only for affected caller/ownership decisions | Connected declared rendering path with allowed original suppliers, side effects and owned lifetime. B04 B12. |
| P3 Functional readiness | Defect resolution; can start independently | The same movement/Escape/Mini/Cancel/resume/normal-Quit scenario passes its functional assertions, ownership checks and response deadlines. B05; B06 is diagnostic. |
| P4 Normal journey acceptance | Top down; required P2 connections and P3 fixes | All existing journeys/operations have appropriate current profile-specific reference, semantic/output and resource evidence. B07 B08 B13. |
| P5 Fault/lifecycle acceptance | Top down; required P2 connections | Declared restart/continuation matrix passes; physical limitations remain explicit. B10 B11 B13. |
| P6 Separate closure decisions | Depends on P1 through P5 within the claimed scope | Separate original-recovery and native-acceptance decisions; no unresolved required blocker. |

P1 now records the required dependency inventory and its explicit recovery
frontiers. P2 can connect required providers within those boundaries; P3 addresses the functional readiness failure. FPS measurements remain
diagnostic. Remaining original recovery follows the
concrete P1 frontiers rather than treating every offline helper as a required
live replacement.

Original recovery is complete only when required entry/dispatch/provider
boundaries, mutation/order/ownership and failure semantics are accounted for,
with no unresolved required original supplier. Component equivalence additionally
requires fresh admitted original/native comparisons of replaced behavior and
side effects. Unsafe original behavior can be documented while native safety
policy deliberately differs; the difference must be explicit.

Native acceptance is complete only when the unchanged journeys and operation
requirements, known failures, required integration, lifecycle/fault checks and
functional response and resource checks pass in the claimed profiles. FPS is
diagnostic and does not establish normal-play performance; optional benchmarks
retain their original thresholds. Unsupported original pixel
fallback cannot occur inside a complete-native claim. A successful implementation
or source review grants neither original equivalence nor end-to-end acceptance.

The two decisions may finish at different times. Any excluded configuration or
compatibility claim must be named in the decision. Report recovery, implementation,
comparison, integration and acceptance independently; do not combine them into a
weighted completion percentage.

## P1 dependency inventory result

The [P1 source and original-byte review](../research/runtime/rendering-required-dependencies-20261024.md)
now supplies the required caller/provider graph and explicit frontiers. It
distinguishes retained original preparation from required native pixel/history
services and records the existing tracked-surface/current-Lock admission. Two
local screen helpers gain static understanding only. P1's inventory exit is
recorded; original recovery remains open in F1 through F5. The 14 journeys,
21 operation requirements and profile boundaries remain unchanged. The functional FPS policy is revised below; historical benchmark criteria remain recorded.

## Functional completion policy (2026-10-25)

The user removed fixed instrumented FPS as a completion criterion. The immediate
deliverable is the strict native movement → Escape/Mini → Cancel → resumed
World → Quit No → resumed World → Quit Yes/report/Main/normal exit sequence.
Required callbacks, no-forward ownership, balanced World retirement, visible
output and bounded responsiveness remain mandatory. This is not completion of
all14journeys, P2 provider integration or original recovery. B06 is diagnostic;
B05 has fresh strict10/30second functional execution; broader journeys and the historical ingress cause remain separate. Historical
19FPS failures remain unchanged, with measurement overhead unresolved.

The [functional execution report](../research/runtime/rendering-functional-lifecycle-20261025.md) records the passing strict and public NVIDIA lifecycle scopes, the separate failed Fireball-damage case and the remaining validation boundaries.

## Cursor screen-service connection (2026-10-25)

[CSR1 recovery](../research/runtime/screen-cursor-provider-recovery-20261025.md)
closes complete local4a4370/4a4380 semantics with55296-case original/model
comparison. Its78-case actual Wine/native/guarded provider matrix checks direct
and deferred copy destinations,full queue preservation and complete native
retirement. This narrows C09/B03 without closing the other external screen
services or owner lifetimes. No portable screen/helper hook is installed;the
existing original-to-native pixel-provider connection is the tested boundary.
The14journeys,21operations and all other blockers remain unchanged.
