# Main Menu Preferences engine bridge (V6)

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The [recovered contract](preferences-engine-contract.md) remains the baseline.
This adapter owns Qt presentation for the Main Menu caller only. The original
engine retains control construction, drawing, input, screen ticks, fades, audio,
settings writing and display/resource rebuilding. Mini activation remains OFF.

## State, actions and ownership

`protocols/include/mnm/menu_v6.h` preserves V1–V5 offsets and introduces a
77,824-byte `MNMMCMD6` channel. The engine publishes 48 bytes at offset 44,708;
the host publishes seven signed semantic values at offset 44,756 inside its
existing seqlock. Neither payload contains pointers or host object layouts.

| Engine payload | Meaning |
| --- | --- |
| Words 0–3 | OK/Cancel mask, control availability mask, caller ID 3, stack depth |
| Words 4–10 | Music 0..15, effects -2500..0, High/Low 0/1, Full/Cut 0/1, dialogue Fast/Medium/Slow 0/1/2, game speed 0/1/2, border On/Off 1/0 |
| Word 11 | Reserved zero |

Availability uses original radio indices 0..11 and music/effects bits 12/13.
Original initialized state and disabled state at control +0x3d authorize each
control; transient pressed/selected state at +0x39 does not. Constructed radios
use vtable `0x005c6d40`, sliders `0x005c6d10`, and Preferences text buttons
`0x005c6c6c`. Destructor/base tables are unsuitable identity checks.

Main action 18 invokes original Main callback local index 3. Preferences screen
10 must be the current ticking singleton `0x006a4948`, initialized with vtable
`0x005c648c`, at the top of a bounded original stack with a Main parent. Heap
arrays, callback function/receiver identities, local control indices, exact
slider bounds, complete group membership/count/capacity and selected ordinals
are revalidated before every command. Fade, pending push, replacement, modal
and return states cannot authorize commands.

| Semantic action | Original work |
| --- | --- |
| 19: OK | Preflight all seven values and changed-control availability; select changed groups through `0x004ce730`, set changed audio through `0x004cdf40`, then button callback `0x004a9840`, local index 0 |
| 20: Cancel | Ignore the host draft and invoke original button callback index 1; restore original entry audio and skip the writer |
| 21: Preview | Argument 0/1 selects music/effects; call only that changed original slider setter and its original callback `0x004a9700` |

Effects attenuation converts to original slider position by adding 5000.
All validation precedes mutation. A bad last field cannot partially apply an
earlier slider or radio. One request is outstanding; stale, odd-sequence,
retired, unavailable and duplicate commands do not invoke callbacks. Original
`GetLastError`, thread ownership and forwarding tick behavior are retained.
Acknowledgement establishes dispatch, not completed navigation or durable save.

## Qt behavior

Widgets expose semantic settings and audio-edit signals. The controller seeds
engine settings/ranges on entry, preserves the draft across audio acknowledgements,
coalesces pending audio edits, and queues OK/Cancel while a preview is outstanding.
Radio edits remain local until OK. Engine-derived availability disables individual
controls. The native preview retains its original local sample settings and
layout ranges; the live policy overrides those with recovered engine units.

Window close from Preferences queues original Cancel, waits for Main readiness,
then invokes original Main Quit. Original-menu fallback permanently retires the
channel; it does not retry an outstanding command. Every new session stages a
disposable copy, so a write affects that session's `game/CFG/prefs.cfg`, not the
immutable media or the source working installation. Cross-launch user preference
persistence is implemented separately by [UI24](preferences-persistence.md).

The V6 observer retains V5 results and V3 spell selection. Explicit older-version
compatibility fixtures remain available. No campaign, realm or Mini caller is
admitted by this Preferences adapter.

## Reproduction and evidence

```sh
cmake --build working/build/qt-shell --target mnm-qt-shell menu-preferences-bridge-test menu-preferences-controller-test preferences-test menu-bridge-test menu-battle-bridge-test menu-spell-bridge-test menu-mini-bridge-test menu-result-bridge-test menu-result-controller-test
ctest --test-dir working/build/qt-shell -R 'qt-(menu-(bridge|battle-bridge|spell-bridge|mini-bridge|preferences|result)|preferences)' --output-on-failure
python3 tools/test-menu-observer.py --preferences
python3 tools/test-menu-observer.py --results
python3 tools/test-live-menus.py --preferences
```

The fixture executes pinned original Main, Preferences slider/button, group
selection and slider-set instructions. Audio services, sample playback, profile
writer, display services and the forwarding common tick are controlled stubs.
It validates actual callback effects and transactional/ownership guards; it does
not validate those substituted services. Its final evidence is
`working/tests/menu-observer/run-8ejvo_6o/report.json`. The V5 regression is
`working/tests/menu-observer/run-h0ywui9w/report.json`. Nine focused Qt tests pass.

Bounded live evidence: `working/tests/live-menus/run-acj4df79/report.json`,
with staged experiment `working/experiments/menu-observer/run-x5luesy7`.
The automated Qt-button/slider run confirms eight ready checkpoints and final
acknowledgement 9. Its exact dispatch trace is Main Preferences → FX preview →
Cancel → Main Preferences → FX preview → OK → Main Preferences → window-close
Cancel → Main Quit. Cancel restores the seven entry values and leaves the staged
file byte-identical. OK changes effects -250 to -500 and dialogue Medium to Slow;
the original writer's keys and reopened engine snapshot agree. The Qt radio draft
survives each audio acknowledgement. Shutdown completes with launcher status 0.
The fixture and live DLL manifests have identical runtime source fingerprints.

Initial diagnostic runs `run-3vk530c1`, `run-9jiqu7xx`, `run-xpe_hqsv` and
`run-o_ddqp1e` failed control admission or harness status checks. They do not count
as live validation. Constructor review corrected radio/text-button identities;
the final run uses those guarded identities. Audio services ran in Wine/Xvfb,
but audible output and physical devices were not measured.
Original manifests are verified before/after artifact-consuming runs. No manual
testing is required by this validation workflow.

Observation event 3 arguments 65536/65537 identify music/effects setter dispatch,
while Preferences arguments 0/1 identify OK/Cancel. Event 5 is a deduplicated
Preferences guard diagnostic: 1 identity/initialization, 2 stack readability,
3 stack top/depth, 4 caller, 5 arrays/callbacks, 6 radio identity/index/state,
8 slider value, 9 group shape, 10 group membership; 11–16 identify slider
vtable/index/min/max/state/callback and 17–20 button vtable/index/state/callback.
Vtable/index/bound/state diagnostics include the rejected scalar as result.
The observer keeps its existing 256-record bound.

## Remaining boundaries

Physical sound and CD playback, real hardware input, resolution/resource rebuild,
all remaining caller variants, gameplay pause/cadence, failure recovery for actual
profile writes, power-loss durability and broad graphics-device combinations
remain unvalidated. Selected-setting process restart is covered separately by UI24. The original writer ignores profile API failures and writes
more keys than this menu exposes; its original behavior is retained. Do not infer
a full engine replacement from this bounded presentation/action integration.

Incremental behavior/evidence accounting is seeded in the scoped
[Preferences register](preferences-coverage-register.json), separately from the
movement register currently being developed in this shared workspace. Run
`python3 tools/audit-re-coverage.py --register research/runtime/preferences-coverage-register.json`
to audit its status, function/dispatch links, evidence and source freshness.
It uses the accounting package's pinned binary inventory. No live equivalence or
engine replacement is claimed by its `live_observation` status.
