# Campaign Preferences live round trip (UI35)

The V12 campaign Mini/Preferences bridge now has an automated real-game round
trip, ending with normal original Main Quit. It validates selected effects and
dialogue settings in a fresh Celtic region 1 campaign. No manual input was
required. This is live observation of native integration with original engine
callbacks, not a full original comparison or native engine replacement.

## Reproducible check

Build the Qt shell with the campaign Preferences harness, then run:

```sh
python3 tools/test-live-qt-campaign-preferences.py \
  --shell <built-shell> --source-root <compiled-tree>
```

The helper creates private Wine, Xvfb and configuration directories; original
files are verified before and after. Native widget actions open New Game, choose
Adept, Enter, then drive three Mini/Preferences visits. Original Escape and the
original Quit/Yes confirmation use input confined to the isolated X display.
The harness has a deadline and cleanup; success requires normal original Quit,
not timeout survival. `--validate-run <directory>` rechecks preserved evidence.

## Observed behavior and confidence

Pinned NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The engine callback trace is exactly:

```text
Main New Game (3,0), difficulty (18,65538), Enter (18,0),
Mini Preferences (17,2), effects preview (10,65537), Cancel (10,1),
Mini Preferences (17,2), effects preview (10,65537), OK (10,0),
Mini Preferences (17,2), Cancel (10,1),
Mini Quit (17,3), original Yes, defeat OK (6,21), Main Quit (3,4).
```

All native admissions and original World ticks/resumes use one engine thread.
Preferences has original parent ID17/depth6; Mini remains mode2/context5/depth5.
Preview changes effects immediately while the uncommitted dialogue choice stays
in Qt's draft. Cancel restores the starting engine values and leaves the staged
configuration unchanged. OK invokes the original writer. The written profile,
native store and reopened engine settings all agree. The final unchanged Cancel
preserves the accepted store.

There are three original World resumes after Preferences, each before the next
fresh Escape/Mini action. Exit has two additional resumes: Quit/Yes resumes World
before the original defeat report; report OK resumes World before Realm/Main
return. Final ACK is 14, the channel is retired, and the launcher exits with
status zero. Confidence is high within this selected sequence.

The first validator expected four total World resumes. The successful game run
recorded five; inspection established the extra original Quit/Yes resume before
the defeat report. The corrected validator rechecked the preserved trace and
normal-exit report. This is an updated assertion about the same execution, not a
second run or a retrospective gate claim.

[Machine-readable evidence](campaign-preferences-live-engine-bridge.json)
retains the source fingerprints, trace, native states, World resumes, final
configuration checks and artifact hashes. Disposable run:
`working/tests/live-menus/run-rbp0dh5i`; engine evidence:
`working/experiments/menu-observer/run-837ca26h`. All 2,927 originals passed both
verification steps. Six targeted Qt regression checks passed on the isolated
build, including the campaign synthetic lifecycle and V6/V10/V11 compatibility.

## Remaining boundaries

Resolution and game-speed settings were unchanged. Display rebuild, timer/pause
cadence, loaded campaigns, Realm/Quick Battle callers and cross-launch campaign
persistence remain unvalidated here. Original helper bodies are retained rather
than reconstructed. Understanding, implementation, comparison, integration and
replacement statuses remain independent. UI34 and earlier evidence keep their
historical fingerprints; source-only changes do not renew those claims.
