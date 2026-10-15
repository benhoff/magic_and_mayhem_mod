# Controls improvements without new UI

This is intentional native input policy for the pinned No-CD `40209ca7`
campaign, layered on original selection, camera, spell-bar and command services.
It adds no widgets, portraits, overlays, command buttons or balance changes.
Original drawing, targeting cursors, simulation scheduling and admission remain.
Offline policy, synthetic adapter, original comparison and live playtest are
separate milestones. A callable original service is not proof of its complete
semantics or of a modern command equivalent.

## Use and defaults

Run `./tools/run-rts-controls.sh` to build/start the existing Qt shell with this
adapter requested. Existing launch controls and campaign menus are used. The
wrapper applies the normal 100 GiB reserve guard, estimating 2 GiB of output.
Alternatively set `MNM_RTS_CONTROLS=1` when running `tools/run-qt-shell.sh`.
The adapter is staged in a disposable installation and enabled only after its
DLL/configuration hashes are checked. Ordinary launches without this explicit
setting retain their original inputs. No original or installed source is patched.

The wrapper also handles an existing working executable extended by logging/DLL
imports: it verifies every original section byte-for-byte against the immutable
source and derives a new canonical installation copy. It never restores or
rewrites `working/game-nocd/Chaos.exe`. Changed original sections refuse, while
the new source path is passed through the existing menu-session source option.
Direct OpenGL staging still requires its normal pinned source installation.

Edit `patches/rts-controls.json`, or point `MNM_RTS_CONTROLS_CONFIG` at a custom
file, before launching a new session. Unknown fields, duplicate keys, conflicting
chords, invalid slots, and out-of-range settings refuse. Camera bookmark and
number-group chords are reserved. JSON config has no in-game settings UI.

| Input | Intentional behavior |
| --- | --- |
| Ctrl + left selection | Extend the existing additive modifier; ordinary Shift remains available |
| Ctrl + number | Save current selection as that team; original single-team membership retained |
| Shift + number | Append selected creatures to the team |
| Ctrl + Shift + number | Remove selected creatures from that team |
| Number / double tap | Recall team / recall and focus; empty searches retain selection |
| Alt + number | Recall team and focus |
| F1 / double tap | Select ordinary player wizard / select and focus |
| F4 | Focus first selected living player actor |
| F2 / Shift + F2 | Cycle idle summons / select all eligible idle summons |
| F3 / Shift + F3 | Cycle ungrouped summons / select all ungrouped summons |
| Ctrl + T | Select all living owned actors of the first selected actor's type |
| Tab | Cycle types within the starting mixed selection, retaining the original set |
| Alt + Q/W/E/R/A/S/D/F/Z/X | Select spell-bar slots 0..9 through the original icon callback |
| Escape | Original cancellation/menu behavior; no separate escape policy |
| Ctrl + F5..F8 / Alt + F5..F8 | Save / recall four camera locations |
| S | Submit ordinary move-to-current-cell commands for selected living actors |

`double_tap_ms` defaults to 400 (0 disables it). Repeats do not count as a new
tap or advance selection. `mouse_scroll_speed` writes the existing
`[MOUSE] MouseScrollSpeed` preference in both staged plaintext and encrypted CFG;
all other settings remain byte-identical after decoding.

Idle is a conservative native predicate: original behavior1, no follow reference,
queue, command mode, movement transition or engagement. It is not a recovered
universal idle definition. Guard/follow creatures are excluded by these checks;
Place-of-Power ownership/guard classification remains a live verification item.
Finding ungrouped creatures only changes selection, never their orders.
The wizard shortcut covers ordinary player type0, not transformed wizards.
Type selection is over owned actors, not a recovered visible-screen filter.
Original double-click team selection is retained; Ctrl+T supplies type selection.
Tab's cached set uses adapter lifetime generations advanced by actual slot reset;
external selection, slot replacement, pool/session changes invalidate the set.

Spell shortcuts are for current original spell-bar entries, not fixed spell IDs.
They require a living owned ordinary wizard as the bar receiver, reject inventory
entries, preserve original mana/target rules and do not cast automatically.
Repeated selection of an already targeted slot is idempotent. Closed or unsettled
spell-bar eligibility and real keyboard/event timing need a live playtest.

S requests an existing move order to the current tile; original delivery clears
queue/follow and applies its movement admission. This is a native stop shortcut
policy, not a recovered Stop command or guaranteed immediate standstill during a
transition. Hold Position and attack-move remain unimplemented: coordinate attack
behavior4 is not an attack-move equivalent. Adding those behaviors requires
separate command/AI investigation and validation. No unsupported action is
silently replaced with a force attack or repeated simulation-memory write.

## Original interface evidence and boundaries

Static instruction inspection of the immutable No-CD source supports these
bounded interfaces; confidence is high for the accesses/calling conventions,
provisional for whole user-visible input semantics:

| Address / field | Scoped observation |
| --- | --- |
| World vtable `5c5dec` → `46c070` | `thiscall`, window/message/VK/lParam, `ret16`; original converts VK through MapVirtualKey |
| View input slot `5c6a54` → `4fdad0` | `thiscall`, eleven words, `ret44`; all words are forwarded; only left additive modifier is extended |
| Actor `+68/+70`, `5073d0/5074f0` | Primary/secondary selection and original selection-counter/effect helpers |
| Actor `+8e9`, `52abc0` | Original single team number, `ffffffff` absent, numeric recall/save uses original selection helpers |
| `5060a0` | Actor slot reset; seven displaced push instructions contain no relative operand |
| `4f7ae0/4f7a20` | Focus actor / set camera fine XYZ; original wrapping retained |
| `6a2dd0`, `4d4d60` | Spell-bar entries stride `c6`, count `1518`, receiver `18ec`, selected slot `1908`; callback distinguishes spell and inventory rows via `1b5` |
| `537bd0` | Copy one length-tagged command to current producer buffer, advance byte pointer/count |

Original pool pointers are reread per operation; no host pointer is saved in
native policy state. Counts are bounded to 256, identities/owners/health are
checked, selection helper list/profile dependencies are preflighted, Stop checks
producer capacity for the entire selection, and unsupported cases forward the
original handler. Installation checks the full source hash in staging and all
expected entry/table bytes at runtime, obtains permissions before publication,
and preserves LastError. A protection restoration failure retains a loaded DLL
with disabled forwarding handlers rather than leaving dangling entry pointers.

The current adapter admits settled campaign World screen2/mode5 only, outside
chat/dialog/menu ownership and on its input thread. Quick Battle, multiplayer,
transformed wizards, arbitrary map inputs and long-session replacement remain
pending. The input callback and keyboard handler include undiscovered/unassigned
inventory boundaries; this work does not classify their complete bodies or reset
that discovery backlog.

## Reproduction

`tools/test-rts-controls.py --claims DECLARATION --output NEW_WORKING_DIRECTORY`
checks 64/32-bit native policy, sanitizers, the actual configured adapter against
synthetic API/service peers, real PE32 compilation, malformed configuration,
and pinned-source import/preference staging. The original manifest is checked
before and after staging. It does not execute original selection, targeting,
orders or full gameplay. Register/ledger evidence distinguishes this explicitly.

Live protocol: in a fresh disposable campaign, verify ordinary and Ctrl/Shift
click/drag, team save/append/remove, repeat/double tap, wizard selection/focus,
mixed-type Tab after mouse motion, death/reuse, idle exclusion for Power guards,
spell selection/Escape/invalid target/mana refusal, queued move interrupted by S,
all camera orientations/bookmarks, then menu/save/load/new mission and focus loss.
Confirm actual selection/count/target/queue state, not only that keys were sent.
Keep failed trials and source hashes under new evidence IDs.
