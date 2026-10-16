# Native V3 live startup and storage

Review date: 2026-10-16. This work separates launcher preparation, native resource
admission, media ownership and the wrapping command transport. Historical failures
remain failures; successful native execution is separate from original pixel proof.

## Confirmed changes

- Fresh Wine bootstrap exceeded its 120-second preparation budget before original
  startup. A private bootstrap excluding optional CLR/browser/desktop helpers
  completed in 9.102 seconds; excluding the menu builder alone took 93.558 seconds.
  These observations do not isolate a single optional component as the cause.
  `directplay_dx6_launch.py` restricts those exclusions to its bootstrap child;
  actual game DLL policy, pinned builtin hashes and refusal-before-write remain.
- Live native menus used checkout `working/game-nocd` even when another source game
  was selected. They now load from the admitted experiment's staged `game` directory.
- Resource decoding previously happened on the GUI thread on live menu publication.
  It now finishes before native channels and original process launch. Failure
  refuses both. Structured `NATIVE_MENU_ASSETS` output records the admitted root,
  duration and ordering; a warm control measured 299 ms, not a cold worst-case bound.
- While a movie owns input, menu polling retains the original host heartbeat but
  defers consuming and delivering menu-ready state. Completion delivers the latest
  state and permits the next semantic action once. Existing request guards remain.
- The storage automation had a separate fixed 240-second timer, including preparation
  and both full intros. Its first uninterrupted run expired before campaign entry.
  It now consumes the runner's existing bounded `MNM_CAMPAIGN_TIMEOUT_MS` and records
  `deadline_ms`; the outer CLI still caps at 600 seconds. FPS, ownership, freshness,
  zero-forwarding and pixel checks remain unchanged. The retained
  [deadline refusal](native-v3-startup-intro-deadline-refusal-20261016.json) records
  shutdown reason 3 after that harness failure, not a new transport diagnosis.
- `MNM_RENDER_COMMAND_WIRE_VERSION=2|3` provides a strict diagnostic matched control;
  ordinary continuous launches default to V3. Finite capture remains V1. GAP events
  now retain the reason plus basename/line site, without a wire layout change.

## Focused controls

`tools/test-native-startup-resources.py` compiles and executes the actual
`LiveMenuSession` with fake preparation/original processes. Two admission cases
verify resource-before-channel/process ordering and refusal without launch. A
movie-readiness case verifies heartbeat advancement, unread state during playback,
latest-state delivery after completion and one semantic dispatch. It is not an
original-game, asset-decoder, media-decoder or OpenGL equivalence test.

`tools/test-directplay-dx6-launch.py` retains six refusal checks and adds fresh
bootstrap coverage: inherited game DLL overrides survive; optional bootstrap-only
exclusions do not mutate the caller's environment. The suite passes three cases.

## Retained negative evidence

The older V3 pre-World GAP4 (`run-sh6ibpik`, observer `run-78w1i5m0`) lacks an exact
failure site. Reason 4 has multiple callers. No claim identifies a history guard
or establishes a transport regression from that record. Other no-start attempts
failed before bridge traffic and must not be counted as rendering failures.

The isolated V2 `run-gtf54d_d` reached normal Quit with 203 native frames and 45
balanced Worlds, but failed the existing minimum of 64 Worlds. The profiled
`run-aua4l84i` reached 331 balanced Worlds and normal Quit but failed a worst-window
paint-rate requirement. Neither result is relabeled as a passing test. One movie
request in that run was producer-cancelled before any decoded/presented frame;
this proves a producer timeout branch, not which heartbeat/acceptance/playback
deadline fired. Resource ordering addresses a concrete host hazard; these records
do not prove it was the sole cause of the timeout.

## Acceptance boundaries

Matched controls use the same current sources and staged game, with ordinary
Escape after two seconds of accepted intro playback. Full intro completion is a
separate mode: `--require-intro-completion` requires both Intro0 and Intro1 to
complete, with decoded and presented frames and no producer cancellation.

The final storage journey requires native World, Save/Load, both display rebuilds,
changed-canvas reload, fresh World resume and normal Quit. Its optional resident
capture enables `test-native-storage-comparison.py`: each selected World raster is
compared word-for-word with unchanged 32-bit original code, including two authored
baseline variants. This is selected World-raster proof around storage, not full
presented-session equivalence, original movie-decoder parity or restored simulation
state equivalence. Generic action-bound sessions, broader optional routes, actual
fault/restart and physical hardware remain independent gaps.

Reproduce the full journey from an isolated supported installation:

```sh
python3 tools/test-native-campaign.py --command-wire-version 3 \
  --resident-world --profile-resident --capture-resident \
  --other-pixel-producers --legacy-audio --save-load --recreate-world \
  --normal-quit --difficulty 1 --min-fps 5 --timeout 600 \
  --require-intro-completion --source-game working/source-game
python3 tools/test-native-storage-comparison.py --campaign <successful-report.json>
```

Use the main checkout's retention guard, private display and Wine prefix and
original-manifest bookends. Evidence results and remaining limitations follow
below after execution; source hashes in historical records are never refreshed.

## Current results

- Matched [V2](native-v3-startup-v2-storage-control-20261016.json) and
  [V3](native-v3-startup-v3-storage-control-20261016.json) controls have identical
  1,288-source fingerprints at baseline 3fbfffe2 and complete 495/479 balanced
  Worlds, Save/Load, both rebuilds, changed-canvas reload and normal Quit. Their
  [V2](native-v3-startup-v2-storage-controller-20261016.json) and
  [V3](native-v3-startup-v3-storage-controller-20261016.json) controller records
  retain ordinary Intro0 Escape input. Later Cure commits and the harness deadline
  repair make their broad source supersets historical; hashes are unchanged.
- The [current uninterrupted V3 journey](native-v3-natural-intro-storage-live-20261016.json)
  completes Intro0 with 1,777 and Intro1 with 1,004 decoded/presented frames, no
  producer cancellation, then the entire storage/rebuild/reload/normal-Quit flow.
  All 480 Worlds have matching import and verified original return counts; 723 native
  command frames and 619 painted frames are recorded. The selected original save
  is 350,336 bytes and remains unchanged on reload. Both display rebuilds complete,
  and the final saved map resumes on a fresh native canvas. Menu asset admission
  precedes channels/process launch and takes 291 ms from the staged game.
- Zero ordinary World readbacks and zero forwarded pixel producers are recorded.
  Two explicit producer CPU readbacks are counted separately; this is not a claim
  that every readback class is absent. A 793,147,633-byte contiguous resident source
  capture is complete and retained for independent checkpoint comparison.
- [Current focused host controls](native-startup-resources-current-controls-20261016.json)
  pass both resource cases and the movie readiness case. The
  [prefix guards](native-v3-startup-prefix-guards-20261016.json) pass three cases
  and six refusals. The [triage index](native-v3-startup-triage-20261016.json)
  preserves unsuccessful development results and standalone Wine timing probes.

The storage harness emits no ordinary smoke-flow cadence steps. Its requested
`--min-fps` is not new worst-window cadence evidence: validation selects the
storage-flow resource/freshness guards instead of the ordinary smoke validator.
The retained profiled V2 paint-window failure and older 19 FPS diagnostic failure
remain performance boundaries. These software functional results do not establish
sustained hardware presentation, physical audio, whole-session output parity,
later-map progression or actual fault/restart. Complete replacement remains false.

## Independent storage pixels

The first comparator attempt retained a
[three-checkpoint admission refusal](native-v3-startup-checkpoint-count-refusal-20261016.json).
The replay utility now admits exactly three or four distinct requested checkpoints,
requires all to be produced and retains surface retirement. It neither drops the
rebuild checkpoint nor turns missing/duplicate checkpoints into a pass.

The [fresh independent original comparison](native-v3-storage-world-original-20261016.json)
passes all four selected World generations:

| Phase | Generation | RGB565 words | Mismatches |
| --- | --- | --- | --- |
| Before Save | 51 | 480,000 | 0 |
| After Save | 136 | 480,000 | 0 |
| After Load | 276 | 480,000 | 0 |
| After rebuild / Main Load | 421 | 480,000 | 0 |

All 1,920,000 words also agree under the two authored original baselines; this is
not a general baseline-independence proof. The complete captured native prefix
through generation 421 is replayed, including the canvas change. CPU reader and
sequence controls pass, all surfaces retire, compiler/source/input fingerprints
remain stable, and original-manifest verification passes before and after. The
original executable hash is the pinned NoCD 40209ca7 build. Three comparator and 17
portable storage regressions also pass.

This closes the selected current V3 live storage boundary and its four independent
World raster checkpoints. The existing 323-frame portable recorded whole-interval
GPU/display comparison retains its own different input scope. No result here
supplies full action-bound presented-session, original decoder, restored simulation,
multi-hour original transport, fault/restart or sustained hardware acceptance.
