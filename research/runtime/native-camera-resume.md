# Longer camera journey and Hermes readiness

The longer camera/Mini/Cancel route failure reproduces on master `9f6f5f80`.
All eight resumed period presses reach the active input channel. Native World
presentation continues, but the three selected camera bodies stop advancing
while an original Hermes card says “Here lies a magical Place of Power.” This
is a harness readiness failure. No renderer or input-forwarding defect was
reproduced by this investigation.

## Evidence and repair

`working/tests/native-camera-resume-20261017` retains the isolated checkout,
original acquisition reports, passive input traces and screenshots. Baseline
`run-g7z162s7` and diagnostic `run-yz5knqtt` both fail the resumed route assertion.
In the diagnostic run, `camera-resume-0-native.png` shows ordinary World;
`camera-resume-1-native.png` and `camera-resume-2-native.png` show Hermes.
The original CPU images are diagnostics because resident World CPU pixels may
be stale. External read-only camera-memory sampling was denied by the OS;
no memory writes or workaround were used.

The camera helper now recognizes only a witnessed Hermes heading in fresh
captures, dismisses through ordinary viewport clicks, and requires two quiet
captures. Resident runs classify the native image; dual runs require native and
original agreement before clicking. Each helper has a maximum of 32 captures
and 12 dismissals. Unknown modals refuse the scenario rather than receiving
unconditional clicks. Camera completion requires all three selected bodies and
four fresh settled turns after any witnessed interruption, within the existing
eight-key limit. Diagnostic and dismissal time cannot shorten the requested
30 seconds of active stress; renderer throughput samples still include it.

An initial repaired pilot, `run-cy19opct`, exposes a second race: Hermes appears
between the final readiness capture and Escape, leaving the journey waiting for
Mini. The retained `camera-waiting-for-mini-native.png` confirms the card. That
pilot remains a failure. The Escape helper now guards the transition itself,
waits for the original Mini callback, and retries at most four times only after
another witnessed Hermes dismissal. The final acceptance independently matches
the callback sequence and Cancel/Quit argument to the original event journal.
No renderer, engine state, dispatch, input forwarding or balance behavior changes.

The seven original readiness controls and two transition controls pass alongside
six route and 21 campaign controls (36 test methods). They cover stale CPU
images, dual-image disagreement, missing quiet closure, image mutation, NaN/bool
durations, capture/dismissal limits, interrupted camera epochs, blind Escape
retries and forged/reused callbacks. These are synthetic refusal controls,
separate from actual-game execution.

## Acceptance boundary

**The camera readiness repair passes targeted functional checks on both World
owners. Full campaign acceptance remains failed at the unchanged software 5 FPS
floor.** GUI-resident runs `run-xtw1yirk` and `run-sjjlbgs4` report 4.9904 and
4.7642 FPS; dedicated-worker `run-2r07u61r` reports 4.7611 FPS. No threshold was
lowered and no failed acquisition was relabeled as a passing campaign.

The [bounded functional report](native-camera-resume-bound-20261017.json)
independently rechecks retained actual-game camera readiness, original callback
and Quit-answer journals, all six producer families, caller census, complete GPU
destinations and retained Python pixel witnesses. It records
`targeted_functional_acceptance=true` and `full_campaign_acceptance=false`.
The external analysis script and its hash are retained in the report; it calls
the unchanged production validators and checks structural flow without claiming
a performance result. Original acquisition errors remain in the
[raw GUI report](native-camera-resume-repaired-06-raw-20261017.json) and
[raw worker report](native-camera-resume-repaired-07-raw-20261017.json).

| Functional observation | GUI resident | Dedicated worker |
| --- | ---: | ---: |
| Active seconds before / after Mini Cancel | 36.058 / 36.065 | 36.061 / 36.063 |
| Fresh turns / selected bodies, each phase | 4 / camera0,1,2 | 4 / camera0,1,2 |
| Witnessed Hermes dismissals before Mini | 1 | 1 |
| Original Mini arguments | Cancel4, Quit3, Quit3 | Cancel4, Quit3, Quit3 |
| Balanced World completed/imported/verified | 511 / 511 / 511 | 466 / 466 / 466 |
| Matched admitted GPU callers | 5,543 | 5,252 |
| Independently checked GPU destinations | 5,669 | 5,354 |
| Compared destination words | 1,382,078,400 | 1,303,116,800 |
| Retained Python cases / words | 16 / 3,374,400 | 16 / 2,896,000 |
| Closed active copy census / retired exclusions | 11,252 / 7 | 10,499 / 10 |

Both selected acquisitions complete original Quit No, resumed World, Quit Yes,
report Continue and Main Quit with original launcher exit0. Nineteen of 43
optional routes execute, with zero forwarding/history refusal, copy failures,
active census misses or destination mismatches. Clipped/self/keyed GPU-source
calls remain absent in these intervals; CPU-keyed routing is not independent
pixel proof. Ordinary World/native/RGBA readbacks remain zero; opt-in GPU-copy
diagnostics are separate. All 2,927 immutable originals verify before/after,
all declared sources remain unchanged, and private Wine stop/wait succeeds.

[Provenance](native-camera-resume-provenance-20261017.json) preserves every
attempt, input trace, retained screenshot, staged binary/census fingerprint and
pilot source version. Scoped copies remove only blanket `apps/skirmish/`,
`game/` and unrelated `tests/raster-splice-reference.cpp` inventory from these
four camera/copy claims. Compiler dependency receipts and their required-source
sets contain none of those paths. Raw reports and their hashes remain unchanged;
concurrent gameplay/raster work receives no new rendering validation.

The exact incoming-history review covers `a904544f..9f6f5f80` and the concurrent campaign commit
`9f6f5f80..9a24c154`, with zero unresolved
file or behavior receipt transitions. It is accounting, not fresh engine proof.
 Unknown modals, other speakers/maps,
complete dialogue semantics, camera3 callers, whole-session original pixel
comparison, physical loss and hardware/performance acceptance remain outside
this repair. The selected original switch maps orientations to camera0/2/0/1;
this work neither adds camera3 nor claims independent observation of orientation
state. GPU copy destination checks remain distinct from original World pixels
and CPU-keyed output.
