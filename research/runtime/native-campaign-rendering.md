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
