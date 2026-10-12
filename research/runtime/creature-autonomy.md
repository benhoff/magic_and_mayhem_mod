# Original autonomous creature decisions

NS26 extends NS25 from supplied decision answers to recovered decision producers
and autonomous behavior handlers. Build: No-CD
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image VAs, not host pointers. The implementation uses
logical values and typed services, independent of Qt and Wine.

The requested target is full original AI. This work has not achieved that target.
The controller and nine autonomous substates are reconstructed; complete route
continuation, periodic pre-decision/follow work, queued orders, scenario/strategic
policy, ranged aiming, animation/action execution and effect ownership still
require recovery, composition and original comparison. The Crocodile encounter
application continues to use its previously bounded encounter policy.

## Recovered contracts

| Original entry | Native decision | Evidence and remaining service boundary |
| --- | --- | --- |
| `518420` | Controller precedence and tail maintenance | Calls recovered producers in original order; state changes from consequence callbacks remain visible to subsequent branches. Pending route/action execution is separate. |
| `516220` | Immediate nearby opponent | Status/form gates, visibility or shared special-vision admission, moving opponent exclusion, physical HTH scaling, health/class score, one masked RNG draw per admitted candidate, strict traversal ties. Reach is a service. |
| `51d3a0` | Idle orientation to a threat | Living hostile visible opponents within weighted distance16; LOS is checked before strict nearest admission. Fine target coordinates use config198/19c. LOS and turning are services. |
| `524910` | Greedy target admission | Flight footprint shifts, original26-neighbor order, strictly decreasing distance, previous-cell exclusion, legal-probe mode1, reach checked before stepping. Arrival at the adjusted target without a prior reach success returns false. Reach and legality are services. |
| `51e090` | Assistance reach | Wrapped XY footprint bounds and absolute Z separation at most1; independent of the receiver's position. Only width2 expands each side. |
| `5186c0` | Attack route response | Clears marker when destination-to-target distance exceeds3 and three times that distance is at least current-to-target distance. |
| `5198b0` | Local hazard score | Center weighting, layer0 exclusion, tile propertyA0 and type/protection gates, original27-neighbor order. Footprint scanning advances X across rows without resetting it. Hazard classification522f50 is a service. |
| `51d200` | Local escape | Center bypasses legality; first legal minimum-hazard candidate; center winner or no winner resets. Movement failure does not reset or try another candidate. Legality mode0 and movement512800 are services. |
| `51d7b0` | Interaction destination | Class/human/inventory42 gates, config580 bit4, owner eligibility, ordered object kinds0..4/6/19, distance10, empty tile and reachability. Ordinary items reject any living same-owner destination reservation, including the actor itself. Kind19 is leader-only and bypasses that reservation scan. |
| `51db30` | World-object destination | Kind61, distance10, owner/status/occupancy/reachability gates; negative FearHate and ranged Evasion radii reject threatened destinations. Same/allied moving or substate9 actors reserve matching destinations. |
| `51d660` | Healing inventory | Leader classes0/24/25/26, nonhuman, injured, unblocked; scans42 slots, item kinds0..4/6, unsigned magnitude*4 fit or below-half health, count/availability. Uses only the first qualifying entry; an empty first entry prevents later use. Callback consumption, local inventory refresh and empty-slot cleanup retain their original order. Inventory execution is a service. |
| `519330` | Priority response | Composes NS25 recent-attacker selection with actual admission. The recent/age>80 branch returns even when no candidate wins. Otherwise flag815 enables spell **70 (`0x46`)** avoidance: effect-owner diplomacy direction, geometry gate,18 region edges, signed width/height bounds, movement-kind gates and strict farthest-region selection. Threat collision and movement orders are services. |
| `51c3a0` | Autonomous state admission | Type27 reset precedes gates; type22 and blocked states/status/form flags refuse. Clears readiness and selected engagement/incoming counters, marker and installs requested substate before behavior11 callback. Behavior execution/reentrancy is a service. |
| `51c510` | Nine-substate dispatch | Home attack, retreat, Troll/Redcap corpse actions, assist attack, escape, return home, interaction and destination pursuit. Selected spell/source/target descriptors, target-moving notification, action/route requests and peer corpse changes are explicit logical fields. |
| `51c880` | Local retreat | Center safety baseline, nearby hostile eligibility, minimum opponent distance, normalized improvement by number of displaced axes, ordered ties and movement failure. Original27 offset table is separate from escape. |

Names are descriptive labels. Findings above have high confidence within their
instruction-derived scope. In particular, `51d7b0` and `51db30` are world-object
selectors, and `51d660` is inventory maintenance; their former experiment labels
"spell selector" and "escape selector" were not gameplay contracts.

## Runtime fields

The packed original record is not the native object layout. Additional projections
include inventory count178,42 entries at17c with stride18 hexadecimal (kind0,
magnitude4,count10,availability14); selected destination650/654/658;
route phase688, route mode694, target-moving attacker6a4, cooldown66c;
selected attack/spell570 and cast descriptor61c/638/63c; regionDE7;
config movement kind44 and fine aim offsets198/19c. Region records have stride294
hexadecimal, XYZ at4/8/c and18 edges at10 with stride1c; width-indexed signed
bounds and movement-kind cases are retained instead of assigning guessed names.
Effect spell ID4c and owner212 remain distinct from effect update type28.
These structures are documented projections, not portable wire formats.

## Reproduction and validation

`tools/export-creature-autonomy.py` verifies the immutable manifest before and
after a hash-pinned, read-only export. Retained static evidence is under
`working/tests/original-ai-ns26/static/` and `static-v2/`; prior static exports
remain under `working/decompiled/creature-behavior-yfao5xs_/`.

`tools/test-creature-autonomy.py` compiles native64, private Linux/i386 reference
and ASan/UBSan/LSan executables, checks prospective source/claim bindings and
compiler dependencies, and compares unchanged original bodies. Geometry, pool
iteration, maximum health and consequence endpoints are declared explicitly.
The harness checks ordered calls/RNG, return values, all actor-pool bytes and
all27 tested original bodies. These checks establish scoped control semantics,
not complete geometry, original effect execution or a whole native battle.

Early failed runs are retained under `working/tests/original-ai-ns26/compare-dev*`.
One discovered contract correction separates target admission's legality mode1,
escape's mode0 and interaction's separate reachability517db0. Another fixture
correction removes the original divide-by-zero domain from composed combat
scores; refusal of that domain is not a valid original engine execution case.
The initial passing run (`NS26.autonomy-initial-original/native`) compares30,720
fixtures across27 unchanged original bodies; native, reference and sanitizers
pass. The final source-bound run (`NS26.autonomy-original/native`) also passes30,720
cases across27 bodies with ASan/UBSan/LSan. The outcome digest
`9896365934819237747` matches native64, original-reference32 and sanitized64.
The standalone `mnm-original-creature-ai` CMake library builds successfully.
All11 declared source/build inputs match after execution. Both immutable
manifest checks pass for2,927 files; tested original text and input bytes remain
unchanged. Initial result fingerprints are preserved as historical evidence.

After the100GiB guard stopped an earlier final attempt, the user approved a
bounded temporary90GiB reserve. The approved run used a private policy file,
added14,001,471 bytes (13.4MiB), and verified the default100GiB policy unchanged.
No cleanup or archival was performed. Full original AI and original-world
integration remain pending as described above.

The storage guard refused one export while free space was97.724GiB. The subsequent
non-destructive review (`working/retention/original-ai-ns26-preflight-refusal-20261012.json`)
found101.622GiB free and no pressure; a new guarded export passed. No cleanup,
archive or budget change was performed.

The state-mutating API receives the actor owned by the supplied world's slot
vector. Peer changes, including a peer reference resolving to that same actor,
therefore retain original aliasing. A detached actor copy is not an owned-world
mutation API. The paired harness uses that ownership contract.

The standalone native library target is `mnm-original-creature-ai` under
`reconstruction/creature-ai/`. It has no Qt, Wine or original-binary dependency.
The original binary is used only by the independent comparison harness.
