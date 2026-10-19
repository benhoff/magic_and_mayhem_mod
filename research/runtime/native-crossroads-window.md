# Native Crossroads campaign window

`NP.campaign-graph-window` connects the bounded native Crossroads chapter to the
existing Qt campaign application. Source-bound automated execution is recorded below. This is native
application policy; original campaign/UI behavior and live replacement remain
separate milestones.

## Player flow

Use **New Crossroads campaign…**, or start with:

```
mnm-skirmish --asset-root GAME_DIRECTORY --crossroads-campaign
mnm-skirmish --asset-root GAME_DIRECTORY --load-campaign CHECKPOINT.mnmc
```

`--installation GAME_DIRECTORY` also supplies a map root. A new chapter starts
its opening paused. The ordinary Summon, Arc Bolt, movement/attack, Play/Pause
and Step controls use the existing session. Cure and Magic Sphere remain disabled
until their route's reward is owned and a living player wizard can cast.

The mission list shows all four authored missions, their availability/completion,
and route rewards. Tooltips explain prerequisites and permanent route commitment.
Between missions, selecting an available row changes the idle mission selection;
Continue starts it and commits an exclusive route. Starting a route disables its
alternative for the rest of that timeline, including defeat/retry. Completed
missions cannot be replayed. The finale completes the chapter with the skipped
route still incomplete. New campaign or loading an earlier save starts/rewinds a
separate timeline under the existing persistence policy.

Mission text lists every typed objective, timer progress and reinforcement waits.
Protection reads **Keep alive throughout** even when its persisted objective flag
is complete, because the protection guard remains active. Mission text and the
control column scroll when needed. Legacy staged objectives retain their three
stage display. Idle graph saves, either active route, earned loadouts and completed
chapters reopen paused. Continue/Retry and terminal disabling use the existing
campaign coordinator. A refused load preserves the owned session and presentation.

## Boundaries and architecture

The controller owns the immutable installed map resource context and the campaign
session. `campaign_view.cpp` translates those into text and semantic mission IDs;
widgets receive no wire offsets, engine addresses or graph indexes. The picker
calls a semantic controller selection action. No simulation, combat, reward or
wire policy changes are introduced. Existing native versions and identities keep
their resource adapters. The previously headless typed Last Stand profile can also
load in this window without indexing the legacy three-stage objective array.

Plains uses the existing diagnostic terrain sprites and raised native unit glyphs;
the finale uses the existing arena view. This does not establish original pixels,
lighting, animation, mission scripts or balance equivalence.

## Automated acceptance

CAMPAIGN04.crossroads-window prospectively binds 635 unchanged source
keys and the actual-widget scenario. Both complete full-health roads and their
finales pass through ordinary unit picking, spell buttons, right clicks and Step;
each queued order matches the headless strategy. 17 normal Qt processes and
1 ASan/UBSan/LSan smoke process pass using the offscreen widget platform.
The sanitizer scope is the 24-tick opening and its controls, not the full chapter.

29 exact save comparisons cover new/start/selection, opening and both route
checkpoints, fresh-process victories, earned rewards/finale transitions, completed
reopening, explicit defeat/retry and actual Cure/Sphere queue/effect continuation.
A retained version-4 objective save reopens unchanged, including typed objectives
and terrain. All 118 existing CTests pass. Source/input stability, compiler
dependency closure and original 2,927-file manifest bookends pass. Only the new
native application behavior gains automated preview integration. This validates
Qt offscreen widgets/dialogs/painting, not X11 or human visual acceptance; older
shared-source evidence keeps its historical fingerprints and limitations.


`tests/campaign-graph-window-driver.cpp` is compiled only into its test executable.
It sends ordinary mouse/button events to actual Qt widgets, uses real nonnative
Qt file dialogs and obtains widget captures. The bounded production strategy uses
a read-only session facade: its proposed orders must be reproduced by unit picks,
spell buttons and right clicks, with exact queued-order serialization checked
before each Step. It cannot enqueue directly into the production session.
Complete journeys retain full health and unchanged combat values.

`tests/test-campaign-graph-window.py` constructs fresh headless reference journeys,
then checks the opening, both exclusive routes and finales through the window.
Fresh application processes reopen mid-mission, between missions, after defeat and
at chapter completion. Dialog round trips, locked row/button refusals, selected
mission/start/victory/complete bytes, earned spell queues and effect continuation
must equal the headless references. Explicit one-health pending-projectile fixtures
exercise defeat and retry; separate earned-spell fixtures provide valid targets.
Those fixtures do not replace the complete full-health journeys.

Qt's offscreen platform executes the real widgets, dialogs and paint code without
an X server. The first development Xvfb attempts failed before application startup:
the server could not close its `/tmp/server-*.xkm` keyboard map under the shared
filesystem quota. That environment diagnosis supplies no campaign validation.
Final acceptance runs inside a task-owned temporary-directory mount namespace
(`bwrap --bind / / --dev-bind /dev /dev --bind TASK_TEST_TMP /tmp --die-with-parent`), so both hardcoded
legacy temporary paths and Xvfb keyboard files avoid the shared `/tmp` quota.
The automated result does not claim X11 event delivery, physical display behavior,
human visual acceptance or interactive usability. Bounded sanitizer smoke and the
existing CTest suite are recorded separately from the full normal widget journeys.

The acceptance runner prospectively binds source/scope declarations, installed
inputs, compiler dependency closure and original-manifest bookends. Development
attempts remain under `working/` and are not positive evidence.


Capture review found that the first result instruction was too long for the
existing centered overlay. The graph prompt is now “Choose a mission, then
Continue.” CAMPAIGN05.crossroads-window-layout prospectively binds the corrected
635 source keys and repeats the entire acceptance: both roads/finales,
18 widget processes including the 24-tick sanitizer smoke, 29 exact comparisons,
all 118 CTests, stable installed inputs/compiler closure and original-manifest
bookends. CAMPAIGN04 remains immutable evidence of the earlier source version;
only CAMPAIGN05 supports current-code preview validation after the prompt edit.
Additional installed-profile-switch and legacy terrain widget regressions passed
on the preceding implementation; their raw logs remain under
`working/tests/crossroads-window-compat-20261017-v2/` and promote no older milestone.
