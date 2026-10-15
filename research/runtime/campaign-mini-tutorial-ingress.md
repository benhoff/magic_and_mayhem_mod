# Initiate tutorial and Mini ingress

The pinned No-CD key handler `0x46c070` translates Escape to scan code1. Its
layout0 campaign branch at `0x46c1af..0x46c1eb` checks context `0x689920`, then
byte `0x6c4859`. With context outside1/2 and a zero byte it writes Mini mode2
to `0x6a50db` and pushes `0x6a5088` through `0x557040`. A nonzero byte skips
the push. This is a selected branch, not recovery of the entire key handler.

`CelticScript1.CFG` events28–32 enable the opening Hermes, Select Zombie Spell,
Summon Zombie, Select Zombie and Move Zombie prompts only at difficulty0.
Events29/30 require ordinary spell-selection/cast commands, rather than an
arbitrary scene click. Existing original-render observations already document
the Select Zombie Spell prompt consuming Escape.

The new private original reference checks layout0, contexts0/5 and tutorial
byte0/1/255. Key translation and screen-stack push are supplied dependencies;
the original handler and branch bytes are unchanged. This cannot establish
the complete tutorial state machine or prove a live byte value from a screenshot.
The live controls and tutorial-aware Save/Load journey have independent evidence below.

## Diagnosis and driver correction

The retained [negative controls](campaign-mini-tutorial-negatives-20261015.json)
reproduce Mini timeouts with original graphics/native Qt menus before Save and
resident native graphics after Save. These use the same six-second driver dwell;
their different engine rates put the tutorial at different phases. Neither
failure is an original/native pixel comparison. The native run retains fresh
World progress without rendering fallback or history refusal.

The [private original execution](campaign-mini-tutorial-original-20261015.json)
passes all six gate cases, with the complete original branch bytes retained as
an explicit recovered-range exception. Discovery still lacks the surrounding
input function; this does not silently promote its inventory coverage. No live
observation of byte `0x6c4859` was added.

The storage driver now recognizes the presented title in bounded, retained OCR
crops and performs the ordinary prescribed input. During Initiate startup it
waits for spell selection, summon, creature selection and movement in order,
then requests Mini after the command prompt clears. It sends no scene click
for unclassified text or the pauses between prompts. The former generic scene
click could clear the spell selection between Select Zombie Spell and Summon
Zombie; a retained original-rendering attempt then stalled even before Save.
Original Mini admission and the final callback/resume trace remain independent
of the helper's claimed input success.

An earlier Save between spell selection and summon reached Load and then stalled
at the restored summon prompt. That diagnostic still used generic scene clicks;
its cause is not established and it does not prove an original serialization or
restored-spell defect. Mid-command saves and later tutorial continuation remain
outside the new journey. No tutorial flag, game rule, save byte or simulation
state is written by the helper.

The original-rendering journey completed Save/Load/normal Quit in
`working/tests/live-menus/run-pj9bpleq`, but its final compiler-provenance check
refused missing protocol14–17 entries in the old build manifest. That result
is not promoted as a validated control. The menu bridge build now records the
actual Clang dependency file, including all55 repository compiler inputs, and
fingerprints the complete declared menu-header set. The control validator still
requires every compiled input to match its prospective source declaration.
Fresh source-bound controls are recorded separately below.


## Fresh original-rendering control

[The reviewed control](campaign-storage-initiate-original-render-20261015.json)
passes in `working/tests/live-menus/run-ligb31pl`, observer `run-9dhjoibh`.
It completes the four opening tutorial commands, Save, original World resume,
Load, original World resume, original Quit Yes, native defeat Continue and Main
Quit/exit0 on one original engine thread. Its354,647-byte save remains unchanged
and merges into the isolated owned store. The driver retains title crops, full
viewport images and ordered physical actions. Sources remained stable; the
complete55-input menu compiler closure matches the prospective declarations;
original manifests verify before/after and the harness completes private-prefix cleanup.
Original graphics/audio and simulation remain active. This establishes a control
journey, not a complete restored-state or original/native pixel comparison.


The first resident-native tutorial-aware rerun
`working/tests/native-campaign/run-za7dox_s` stops during opening progression when
Tesseract exits SIGFPE on a background-only crop. The
[immutable failed report](campaign-storage-initiate-ocr-failure-20261015.json)
retains the helper traceback, stable sources and stopped private prefix. This is
an OCR-tool failure, not a renderer failure. Failed or timed-out title recognition
now records its error and returns unrecognized text; it cannot generate a tutorial
click. The ordered four-command requirement and original Mini/Save/Load/Quit trace
still have to pass. A focused regression supplies plausible title output with a
failed OCR return status and verifies it cannot admit input; timeout is likewise
unrecognized. Earlier original control evidence predates this helper adjustment
and retains its recorded hashes; a fresh repeat is recorded separately.

A subsequent unchanged-engine attempt `working/tests/native-campaign/run-r0raru91`
never admits native Main commands: observer events contain only the initial
Main entry, driver stage0 has no actions/checkpoints, and native menu frames
continue until terminal cancellation. Its
[negative startup report](campaign-storage-initiate-startup-refusal-20261015.json)
and raw channel/display are retained. Cause is unresolved. The one-shot startup
channel admission is a candidate boundary, not an established diagnosis or a
validated fix. This attempt exercises no tutorial Save/Load and is not counted
as its passing control. Reliable admission across repeated startups remains a
separate pending limitation even if a bounded storage journey passes.


## Fresh resident-native result

[The final native report](campaign-storage-initiate-native-passed-20261015.json)
passes in `working/tests/native-campaign/run-pmqcbbx8`, observer `run-axf0860q`.
The four-command proof precedes the first Escape. Original callbacks are exactly
New Game, Region Enter, Mini Save, Save, Mini Load, Load, Mini Quit, defeat
Continue and Main Quit, with successful storage-return/World-resume events in
their required intervals. Native source generations50/487/647 are fresh at the
three phase checkpoints; the initial checkpoint precedes tutorial progression
and the actual first Save. The355,142-byte save SHA-256
`28bba4f5bb8a031e397e64ff0abfd523055047068d1f245ef743c603d3436812`
is unchanged across Load and owned-store merge.

All709 Worlds complete/import/return, with no pending generation and normal
retirement. There are7,653 native optional producer calls, zero forwards/history
refusals,43 installed routes and17 exercised in this journey. This adds no routes
to previous camera/spell evidence.22 ordered process CPU-reader snapshots and2
explicit source-panel snapshots remain separate from zero ordinary World/native/
RGBA readbacks and zero World pixel replies. Real DirectDraw is disabled/absent.
Software llvmpipe averages13.37engineFPS above the explicit10FPS functional floor;
this is not hardware performance validation.90 surface creations, peak55surfaces
and5,297,952bytes are observations, not full COM teardown or changed-canvas proof.

Sources and inputs remain stable, original manifests verify before/after, normal
Quit exits0 and the private Wine prefix is stopped. Native and original controls
are independent journeys at different clock rates, not paired save bytes,
complete restored simulations or a whole-session presented-pixel comparison.
Mid-command tutorial saves, later tutorial continuation, changed World canvases
and the retained startup-admission failure remain explicit boundaries. The
previous Apprentice/World pixel results retain their own historical source hashes.

```sh
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-campaign.py --save-load --difficulty 0 \
  --resident-world --other-pixel-producers --legacy-audio --skip-movies \
  --min-fps 10 --timeout 420 \
  --source-game working/tests/pixel-staging/run-qeshupqr/source-game
MNM_MENU_GAME_SOURCE="$PWD/working/tests/pixel-staging/run-qeshupqr/source-game" \
MNM_CAMPAIGN_DIFFICULTY=0 \
  python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-live-campaign-storage.py --legacy-audio
python3 tools/working-retention.py run --reserve-gib 1 -- \
  python3 -B tools/test-campaign-mini-ingress.py
```

The original control uses the current built `working/build/qt-shell/mnm-qt-shell`;
the native harness builds it before its run. Keep Wine sessions serialized and
preserve the isolated source fixture and raw experiment directories.


The [final-helper original repeat](campaign-storage-initiate-original-reviewed-20261015.json)
passes in `working/tests/live-menus/run-myg3cjuv`, observer `run-x1n5vwp0`:
Save/resume/Load/resume/normal Quit, unchanged355,405-byte save/owned
merge, stable sources, all55 actual menu compiler inputs and original manifests
before/after. This result binds the final OCR failure-handling helper source;
the preceding original-rendering result remains immutable under its earlier ID.


Coverage keeps all negative records in the registry and storage scenario, with
original prospective claims and source hashes intact. They are deliberately
excluded from the successful storage behavior's stage-support links: a failed
startup/helper/journey must not make its integration status current. The audit
therefore reports their positive milestone claims as retired/link-removed. The
final successful native run alone supports the current native storage stages;
the selected original gate has its separate six-case reference.
