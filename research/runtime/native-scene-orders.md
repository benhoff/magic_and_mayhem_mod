# Native scene selection and move controls (NS13)

Reviewed 2026-10-05. `NP.scene-orders` is an intentional native diagnostic UI
policy, separate from recovered original input mappings and commander/summoned
creature gameplay. It changes no balance rules or original artifacts.

## Contract

The scene provides a creature selector, bounded X/Y/Z target-cell fields and a
Queue move action. Available choices are noncleaned native creature motion drivers
in slot order, up to NS11's 32-driver bound. Stationary blockers and other families
are excluded. Choice labels show their logical cell and movement action; full
unsigned slot/generation handles remain behind the presentation boundary.

Selection is transient application state. It neither advances a tick nor changes
a checkpoint. The controls express semantic selection and target-cell actions;
the widget owns no simulation session, legacy addresses or navigation adapter.
The application Orders controller validates full-generation identity against owned
state before queuing through `MovementSession::move`. It synchronizes selection
on refresh and before a move. Cleanup, release and slot reuse clear stale selection
without implicitly choosing the replacement. Invalid selection preserves the prior
selection; out-of-map or rejected orders preserve the world. New application
sessions begin with no selection; selection is not serialized.

Queue move adds a native pending command and leaves position, animation and tick
unchanged. Step applies queued orders through the existing tick/planner/reservation
policy. Reachability and occupancy outcomes belong to that policy: selecting a
blocked destination is not silently rewritten by the UI. Save includes pending
commands, so a queued move can be saved before Step and applied after restore.
Target fields survive ordinary refresh/ticks; changing selection initializes them
to that creature's logical position. Empty selection disables fields and Queue move.
The interactive view outlines the selected displayed body; this transient marker
is excluded from diagnostic PNG/RGB565 exports and simulation state.

## Validation

Build `apps/world-scene`, run CTest `native-world-controls`, or retain a report:

```bash
python3 tools/test-scene-orders.py BUILD/world-controls-test NEW_OUTPUT
```

The production QWidget is connected to the production Orders controller and a
synthetic native multi-creature session. Real widget selection/spin/button signals
check selected-only ordering, no implicit tick, target preservation without signal
recursion, no-selection refusal, invalid/stale identity and target rollback,
blocked simulation outcomes, cleanup/release/generation reuse, other-family
exclusion, disabled controls and 32-choice refresh. Generation `0xfedcba98` stays
exact. The runner preserves a widget screenshot and a pending-order checkpoint;
a fresh process applies it and produces bytes identical to uninterrupted movement.
Selection starts empty independently of the saved pending command.

The accepted [record](native-scene-orders.json) pins both normal and ASan/UBSan
reports, current source fingerprints, binary/artifact hashes and test logs. Both
builds pass the widget/controller checks and one exact fresh-process continuation.
All 105 normal CTests and five focused sanitizer suites pass. These tests consume
owned synthetic inputs, not original game artifacts. Main-window callback and
selection-outline integration are reviewed and compiled; an installed full-window
interaction/picking comparison is not claimed. Historical scene/sandbox evidence
keeps its hashes and may remain stale after shared-source edits.

[Committed-history review](native-scene-orders-history-review.json) found two
unreceipted render-transport documentation versions in `1d1ca72`.
[Resolution](native-scene-orders-history-resolution.json) accounts for those exact
versions with a pending retrospective receipt. This is documentation accounting,
not a historical gate-pass or new transport execution assertion.
[History extension](native-scene-orders-history-extension.json) also checks
subsequent committed engine work before this milestone.

## Remaining boundaries

Mouse/world picking, group selection, cancel/stop buttons, automatic tick playback,
faction/player permissions and original command/hotkey equivalence remain open.
Mixed asset profiles, original multi-entity admission, attachments, lighting and
visibility integration, AI/combat, commander/summoned-creature behavior and live
replacement remain outside this diagnostic milestone.
