# Combined native campaign session

Intentional host policy, not a recovered gameplay rule. The public `--live-menus
--native-commands` route stages both adapters into one fresh, hash-checked copy of
the pinned no-CD build. Original menu callbacks, World simulation and drawing
continue; native Qt controls and native command/GPU presentation own the user
interface. No full drawing replacement or pixel equivalence claim is made.

The menu session prepares native presentation after staging and before launching
its private Wine prefix. Render channels remain versioned files, with a shared
launch ID for command/recovery mapping. Menu readiness suspends native gameplay
input; engine-confirmed Enter/Cancel returns the native viewport and focus.
Command refusal follows the existing original-window fallback path and is a test
failure. Frame and input polling stop with the menu producer's lifecycle.

Historical failure: public combined flags exited 2 at b70c9e6. The integration
change removes that admission conflict and shares the ordinary native channel
setup. Cold Wine startup has a two-minute initial frame bound; recovery continues
to use the existing ten-second bound. Live startup, menu handoffs, sustained
interactive rendering, visual completeness, gameplay input and performance are
pending until independently observed. New branches/failures must be retained
under new evidence IDs, without editing historical result hashes.

## First live failure and stricter gameplay admission

`native-campaign-premature-escape-20261009.json` preserves the first live attempt.
The native frame counter advanced during the original Region Entry fade. The
old smoke driver incorrectly labelled those frames gameplay and immediately
sent Escape. The original event trace has New Game and Enter but zero completed
World ticks; Mini ingress records Realm context 4/mode 5, which the campaign
Mini adapter deliberately does not admit. The inspected `gameplay.png` is the
fading Region Entry screen. Confidence is high within this captured route.

The test now requires three same-thread, initialized, independently observed
World tick completions before sending gameplay input. This changes test
admission, not engine readiness or menu protocol. Private Xvfb runs explicitly
use Mesa software rendering; hardware/display runs remain separate.

`--stress-seconds 20 --menu-cycles 3` exercises physical camera arrow keys,
comma/period rotation, scene/HUD clicks and wheel input before and after three
Escape/native-Mini/Cancel returns. The test counts native completed command
presentations separately from visible Qt `frameSwapped` callbacks. Each gameplay
phase must average at least `--min-fps` (default 20) in both counts, with no
one-second window below half that floor or sampling window longer than two
seconds. Startup/loading and native menu intervals are excluded. This bounded
latency/throughput policy is a test requirement, not the recovered engine's
frame pacing. Injecting orders is not proof of movement or combat outcomes.

## Ordered copy and presentation visibility findings

`native-campaign-copy-gap-20261009.json` preserves the next failure after World
readiness was corrected: independent World ticks and native Mini Cancel/World
resume were observed, but overlapping commits on two original threads caused
GAP and a failed recovery. The explicit combined route now enables the existing
bounded ordered-copy scheduling policy. This is a native policy, not recovered
original thread ordering. An optimized `RelWithDebInfo` shell build is the public
launcher default; `MNM_QT_BUILD_TYPE` selects an explicit alternative.

`native-campaign-smoke-passed-20261009.json` records the first positive full route:
88 native presentations, 56 visible Qt paints, zero recoveries/fallback, three
original World completions and original Cancel/resume on one thread. This result
predates later stress/stacking/capture changes and remains historical.

The early whole-window gameplay capture showed two HUD/minimap placements.
The later native framebuffer and independently copied original-owned primary
image showed the same single HUD. The original Wine desktop can overlap the Qt
window during startup, contaminating X11 window screenshots. Native presentation
now lowers only the identified new Wine desktop, without unmapping it; semantic
Enter/Cancel restores Qt focus. Test captures distinguish the native framebuffer
from the original owned frame stream. Comparisons remain unsynchronized.

Initiate's first gameplay tutorial absorbs camera/Escape input while prompting
for Zombie spell selection. A 20-second injection phase reached the tutorial,
with matching native/original images, then stalled before native Mini return;
its private Wine prefix was stopped deliberately to retain failure/timing.
This does not establish a native renderer crash or working camera/movement.
Apprentice is an explicit normal native Region Entry radio selection for active
gameplay stress; the default smoke retains Initiate. No engine balance/config
patch or diagnostic readiness bypass is used.

## Active gameplay performance failure and shader correction

`native-campaign-apprentice-stress-slow-20261009.json` retains a fail-closed active
Apprentice run: four input phases and three native Mini Cancel/World resumes
completed without recovery or fallback. Phase averages were 42.4/27.9, 59.7/34.3,
59.2/32.5 and 59.5/34.6 native/visible-paint FPS. The first camera phase had a
9.14 FPS one-second window, so the fixed 10 FPS minimum failed. Input completion
and engine callback evidence do not turn that performance failure into a pass.

A ten-second CPU-clock profile of the native process retained 2,919 samples,
zero lost samples, with 83.01% in Mesa JIT code, 11.58% in Gallium and 0.89% in
the shell. The symbol and DSO reports are retained beside this document. This is
software-renderer CPU attribution, not an identified hardware GPU bottleneck.
Canonical RGB565 presentation now selects constant masks/shifts before the
generic-mask uniform division path, preserving the existing bit replication.
`native-campaign-rgb565-regression-20261009.json` passes exhaustive 65,536 RGB565
values through the shared GPU texture and independent CPU oracle, plus indexed,
24/32-bit, lifetime, command replay and refusal checks at normal/1.5 Qt scales.
The gameplay effect requires its own fresh run; historical evidence is unchanged.

The first shader-only rerun also failed the original timing floor, despite all
three Mini returns and zero fallback/recovery. It is retained under
`native-campaign-fast565-stress-slow-20261009.json`; the color optimization alone
is not a performance fix. Opaque unmasked, unkeyed, untranslated native surface
copies now use `glCopyTexSubImage2D` from a distinct integer source texture.
Masked, keyed and palette-translated copies retain their shader path. Overlap
callers retain frozen-source/ordered-key semantics. Five independent renderer
checks passed: recorded owned backend, formats, aliases/ownership, native blits
and persistent surfaces. The live performance effect is assessed separately.

The original manifest verifier now hashes the same 2,927 files in one Python
process, retaining byte-sorted paths and identical TSV bytes, instead of spawning
two processes per file. Both experiment sides still verify immutable inputs.

## Final bounded live results

`native-campaign-directcopy-stress-passed-20261009.json` retains the first passing
20-second, three Mini return stress run: 5,486 native frames/3,334 visible paints,
zero recoveries/fallback; first phase passed narrowly at a10 FPS minimum window.
Its source/scope predates the additional spell/creature target attempts.

`native-campaign-spells-stress-passed-20261009.json` is the fresh scope-bound
result after those additions. Four 40-second physical-input phases and three
native Mini Cancel/World resumes pass unchanged20 FPS phase-average/10 FPS
one-second floors. Source fingerprints remained stable; before/after manifests
verify 2,927 immutable inputs. Native/visible paint rates by phase:

| Phase | Sample seconds | Native FPS | Visible FPS | Worst 1s FPS |
| --- | ---: | ---: | ---: | ---: |
| First World | 42.582 | 50.94 | 33.14 | 14.00 |
| Cancel1 | 42.365 | 54.69 | 33.28 | 23.64 |
| Cancel2 | 42.446 | 54.68 | 32.77 | 18.12 |
| Cancel3 | 42.282 | 55.34 | 33.84 | 20.91 |

9,621 native presentations and5,862 visible paints completed without recovery or
original-window fallback. Engine-side callbacks independently observe New Game,
Apprentice difficulty, Enter and three Cancel/resume actions on the same thread;
three initialized World tick completions authorize the start. Native captures
include changing camera views, selected actor, movement markers, overlapping
foliage, terrain, river and HUD/minimap. The retained final PNG is diagnostic.
Mana remained 0/15 in inspected captures; spell/target attempts do not establish
successful casting, summoning or combat. `movement_verified` remains false.

The paired10-second 99 Hz profiles are retained in
`native-campaign-spells-profile-20261009.json`: native 80.62% Mesa JIT/12.47% Gallium;
original 39.01% Mesa JIT/35.78% kernel32/14.31% Chaos. No lost samples. Software
rendering remains a material cost; the final pass does not establish a causal
speedup,60 visible FPS or physical-GPU performance. Historical timing failures
remain negative. Unbounded sessions, other regions, arbitrary menu contexts,
synchronized pixel comparisons and complete original drawing replacement remain
separate outstanding milestones.
