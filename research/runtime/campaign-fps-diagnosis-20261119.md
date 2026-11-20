# Loaded campaign-map FPS diagnosis, 2026-11-19

The reported screen is the overhead campaign map with dotted roads and the
walking wizard. The user confirmed this after inspection of a private copy of
`Ben.sav`. It is distinct from an in-mission World and from the opening-map
campaign benchmark. No production rendering or gameplay code changed here.

## Confirmed delivery boundary

The [retained source frame](campaign-fps-20261119/loaded-02-reference.png) shows
the overhead map. The [loaded-map trace](campaign-fps-20261119/loaded-02-native.json)
contains zero completed World generations and UI-only output. Its final
20-second no-input interval has the following measurements:

| Measurement | Result |
| --- | ---: |
| Source UI PRESENTs / native exports / Qt swaps | 79 / 79 / 79 |
| Mean interval between source PRESENTs | 254.075 ms |
| Mean interval between Qt swaps | 253.786 ms |
| Export to GUI pickup, mean / maximum | 0.759 / 2.515 ms |
| GUI pickup to Qt swap, mean / maximum | 2.698 / 24.724 ms |
| Blocked exports in this interval | 0 |
| Whole-trace native viewport paint, mean / maximum | 0.346 / 19.073 ms |

[Exact interval analysis](campaign-fps-20261119/ui-producer-analysis-01.json)
pins the original report identity. A [retained analysis driver](campaign-fps-20261119/reproduction-driver.json) recomputes the paired counts and timing distributions without another game run. Intervals use its single native host clock.
Counts are observations, not a physical monitor/scanout measurement. Aggregate
export refusals elsewhere are opportunities, not distinct dropped-frame counts.

This interval localizes the sparse displayed updates to upstream UI PRESENT
production. The native consumer and Qt deliver the available frames promptly;
they do not spend roughly 250 ms painting each map frame. It does **not** prove
an intentional original FPS cap, identify the original hot function, or measure
the user's moving-wizard or dialog interval at exactly 10 FPS.

## Why the World counters did not explain this screen

`NativeRenderWorker` consumes ordinary UI PRESENTs, but supplemental independent
World drawing requires a completed compatible World and sealed World-mapped UI.
`CommandConsumer::sealedGeneration()` requires `hasWorldOutput()`, and
`ResidentWorldSession::displayState()` is inactive without an initialized World.
The overhead map can therefore display through the native command consumer while
having zero World generations and no independent World redraw clock.

The FPS overlay counts actual Qt viewport swaps in all these modes. The user's
earlier complete session also contained in-mission World activity: 5,602 balanced
imports/returns, 15.331 engine Hz, 28.823 native World FPS, 21.682 ms mean native
World work and a 484.537 ms maximum. Its mean first-World-UI-seal wait was
39.464 ms. Those [retained counters](campaign-fps-20261119/user-resident-world-report.json)
are separate from this campaign-map interval. The user session had detailed GUI
tracing disabled; its averages cannot locate a particular map/dialog dip.

The existing native Realm Viewer is experimental and does not supply the whole
original map/walking-wizard contract. Enabling it, removing current-World UI
pairing, changing simulation speed or emitting duplicate frames would not
establish a fix for the reported animation responsiveness.

## Collection and remaining work

The user's original save hash is
`0798696f6a43a4874e97df39ad52ad379fb0ca953e69d2b9a42d07c0bcead04f`.
Only a private copy was staged; original bytes remained unchanged. Sources and
original media checks were stable, and owned Wine processes were stopped after
the diagnostic. The original save is retained locally as a SQLite-backed input;
no publication is requested. The [support custody manifest](campaign-fps-20261119/support-custody-01.json) binds tested binaries, private configuration, logs and failed-dialog captures to their surviving local SQLite store.

The [loaded diagnostic](campaign-fps-20261119/loaded-02.json) used native M1
rendering, native terrain lighting, retained World images, matching-UI policy,
bounded frame tracing and the existing Main Load path. The broader storage
harness **failed** its deadline because it expects a restored in-mission World,
whereas this save opens the campaign map. That failure remains immutable. Its
retained map trace supports only the narrow delivery observation above; it is
not a successful Save/Load, campaign journey or performance acceptance result.

Opening-map attempts did not reproduce the reported low rate: sampled visible
windows were approximately 37--60 FPS with original lighting and 42--60 FPS with
native lighting. Both later failed Mini/dialog handling. Their scene differs
from the reported campaign map, so they do not clear this issue. Setup refusals,
missing OCR executable, dialog rejection and deadline failures remain in the
attempt records. A missing historical applicability archive granted no reuse
credit. No failed run or historical claim was renewed.

The next causal measurement belongs in the legacy campaign-map/UI producer:
separate its update pacing, animation admission, original CPU drawing and
publication work while the wizard walks and dialogs open. The present result
locates the limiting boundary; the exact original producer cost and a smooth
native campaign-map animation replacement remain open. No performance fix,
60-FPS guarantee, source-equivalence or engine milestone promotion is claimed.
