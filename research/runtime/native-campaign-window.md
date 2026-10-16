# Native campaign window

This is intentional native policy for the two authored arena duels. The Qt
window uses the existing CampaignSession and frozen arena adapter; it does not
recover original campaign menus, maps, rewards or balance.

New campaign begins the first duel paused. Load campaign restores a checkpoint
paused, including pending orders, casting and projectiles. Save campaign writes
the entire typed campaign/world envelope atomically, with the standard file
picker confirming replacement of an existing destination. A publication whose
durability cannot be confirmed is reported as an error. Standalone save and
campaign save remain separate actions and wire formats.

The controller owns campaign lifecycle, routes semantic orders and ticks through
CampaignSession, and pauses when a result settles. The session releases the
terminal battle; the view shows the persisted result and current region. Continue
starts the selected next mission; Retry starts a fresh attempt after defeat or
draw. Both stay paused until Play. Final completion disables Continue. Loading
an idle checkpoint never auto-starts a battle or settles its stored result again.
Load candidates validate before replacing the current owner. A failed load leaves
campaign bytes unchanged and playback paused. No reward or combat-rule changes.

The test-only window executable sends actual mouse/button events and handles Qt
file dialogs. Independent processes compare mid-flight checkpoint and final
victory bytes with the headless CLI, reopen settled results unchanged, continue
to defeat, retry to campaign completion, reject checksummed resource/content and
corrupt checkpoints, and exercise timer/pause. Existing standalone window tests
remain a separate regression. Tests use synthetic arena data only.

Remaining boundaries: old-save rewind remains allowed; result identity is local
to the adopted checkpoint lineage. No cross-process event transaction, crash or
injected I/O failure claim, installed-map integration, or original UI equivalence.
