# Targeted native pixel route scenarios

Work in progress, 2026-10-15. Route execution, original comparison and full
replacement remain independent milestones.

## Confirmed invalid DC hook

The pinned No-CD import census identifies IAT `0x5c51fc` as USER32
`ClientToScreen`, with no `BitBlt` import. Direct disassembly of the original
windowed surface wrappers shows two pushed arguments, HWND and POINT, followed
by use of the returned point to translate the destination rectangle. The prior
`MnmDcCopy` installer patched ten such sites with a nine-argument stdcall thunk.
Matching CALL bytes alone did not establish target API or argument layout.
Those branches were unexercised in the recorded fullscreen campaign.

The corrected launcher removes this feature and rejects obsolete enabled flags
or manifests. The compatibility DLL refuses explicit takeover and contains no
production patch machinery. The standalone native DC-copy service and its GDI
comparisons remain useful; they never established a game BitBlt hook.

Counter wire v2 enumerates six valid families and 43 routes. The ten retired
labels do not become successful tests. Historical v1 reports and fingerprints
remain unchanged. The [current original-call fixture](native-dc-hook-retirement-20261015.json) passes 30 calls, zero stack mismatches and all ten unchanged CALLs, including the original empty intersection return at `0x58c6a0`. Default/off compatibility DLL loads succeed; enabled obsolete loads refuse. The [corrected camera journey](native-pixel-camera-passed-20261015.json) passes; the renewed spell journey also passes.

## Live scenario admission

`--other-pixel-producers` now requires zero original forwards and no history
refusal for the complete run. Repeatable `--require-pixel-route FAMILY/ROUTE`
requires positive native calls on each selected route. Installation alone cannot
pass. `--pixel-camera-scenarios` gives successive ordinary rotation inputs time
to settle and requires all three camera bodies selected by the original dispatcher to advance over at least four settled physical turns.

Resident summon count checks use the current native HUD and independently
observed original creature/cast/combat events. Both image hashes and OCR strings
are retained; stale CPU World pixels cannot classify resident counts. Dual-draw
mode retains its independent native/original count requirement.

The initial spell journey retained a negative final report: a delayed second
summon was used by its helper without being recorded in `additional_summons`.
The final independent validator correctly rejected that unsupported combat claim.
The helper now waits for the actual creation interval and uses only proved
summon identities. The corrected camera journey passes; the renewed spell journey also passes. Both negative reports remain immutable: [missing creation interval](native-pixel-spells-unrecorded-summon-20261015.json), [unselected fourth body](native-pixel-camera-four-body-negative-20261015.json).

World batching intercepts several legacy CPU raster leaves before their optional
interior counters. An absent counter does not establish absent native World
rendering. Each remaining route needs a documented caller/scenario or an explicit
pending reachability boundary. Later map/load transitions, actual-game worker
recovery, independent original/native session pixels and hardware performance
remain separate acceptance work.

## Confirmed camera dispatch boundary

The [pinned static result](native-pixel-targets-static-20261015.json) checks
selector `0x555c3c` and all four entries at `0x555e08`. Cases 0, 1, 2, 3
call bodies `0x5527a0`, `0x552f20`, `0x5527a0`, `0x552b50`: camera0,
camera2, camera0, camera1. Case2 aliases camera0. The camera3 body at
`0x5532f0` is not selected by this table. Preserve that original baseline;
there is no native engine fix to add a camera3 branch here. Indirect or other
callers remain unresolved. Counter progress cannot independently identify the
four physical orientation values because two dispatch cases share one body.

## Remaining valid route boundaries

The earlier 34-route list included ten invalid DC labels and camera3, which this
dispatcher does not select. This leaves 23 other valid routes absent from that
bounded campaign; this accounting correction is not new execution evidence.

| Routes | Current boundary / next specific evidence |
| --- | --- |
| `pixel/rgb_add`, `colour_rect/rectangle` | Resident World captures these leaf requests before their old CPU interiors. An outside-World caller and matching current history are still needed to exercise the optional CPU routes live. |
| `pixel/pcx_words`, `pixel/brighten`, `pixel/row_copy` | Original isolated comparisons exist; a supported live caller/scenario has not been identified. Their entry windows are not discovered function entries in the retained census; keep that limitation. |
| `raster/splice0`, `1`, `4`–`8`, `11`–`20`, `22` | Eighteen generic/fast/slow source/clip/format/blend variants remain unexercised outside World batching. Spells rendered through native World batches do not prove these CPU interiors ran. Their isolated original comparisons retain separate historical fingerprints. |
| `minimap/camera3` | Isolated original body comparison exists; no selection in the reviewed live dispatcher. Other/indirect callers remain pending. |

Required-route checks retain every valid route, including camera3. Explicitly
requiring an unexecuted route fails; it is never silently waived by the ordinary
camera scenario. Zero forwards/history refusal is checked across all six
families, including families outside the required subset.

## Current camera journey

The [six-family camera run](native-pixel-camera-passed-20261015.json) completes
native Mini Cancel/resume, Quit No/resume, Quit Yes, Defeat Continue and Main
Quit with original exit0. Both physical phases advance camera0/1/2 over four
settled turns. All43 valid routes are installed;19execute natively,24remain
unexercised,14702native calls and zero forwards/history refusals. The resident
World completes/imports/returns balance at normal retirement. Original DirectDraw
is disabled. This software llvmpipe run uses an explicit10FPS functional floor
and records15.86engine FPS; hardware performance and original pixel equivalence
remain separate. `complete_replacement` stays false.

The [reviewed target inventory](native-pixel-targets-reviewed-20261015.json)
retains all24 pending routes with their original entry, census callers and
conservative rel32/stored-pointer candidates. PCX WORD, brighten, row-copy,
camera3 and fast-copy entry57e1b0 have zero such candidates and lack discovered
function entries. That limits available live scenarios; it does not prove global
dead code or close indirect/external callers. The other raster variants retain
actual dispatch references and conditional caller work rather than invented
exercise claims.

## Current spell journey and standalone DC service

The [renewed spell campaign](native-pixel-spells-passed-20261015.json) passes
normal Quit with52120native producer calls, zero forwards/history refusals and
3516balanced resident completions/imports/returns. It verifies the second
summon's actual creation interval before accepting its24melee damage events and
killing blow. Player Fireball damage and invalid-target/insufficient-mana attempts
pass their independent original event checks. Resident native HUD count0/15 to1/15
is current; the hashed CPU World images remain diagnostic. The23explicit GetDC
snapshots are separate from zero ordinary native/RGBA/World readbacks. Engine
FPS14.32 exceeds the declared10FPS software functional floor. The same19of43
optional routes execute, leaving24explicitly unexercised. Spells inside native
World batches supply no extra CPU-interior coverage.

The [standalone DC service rerun](native-dc-service-fallback-20261015.json)
compares1024native SRCCOPY results and separately1024genuine GDI SRCINVERT
fallback results, with5120atomic native admission refusals and zero final DC/
surface owners. Native output is checked before fallback, so fallback cannot mask
a native mismatch. This supplies no production game BitBlt target.

Current prospective camera/spell scenarios supply scoped fresh route/history
policy observations. Broader casting/hosting milestones keep their original
scenario/freshness limits; no full-contract or original-session pixel promotion
is inferred from a bounded successful journey. The casting behavior itself was
not prospectively claimed in this invocation, so its broad validation remains
pending despite retained successful original event checks.

Incoming committed history a7d58fcd..c74e9c5b has54exact file and241affected
behavior transitions. Two missing behavior receipt edges have explicit
retrospective pending receipts; the [initial inspection](coverage/pixel-routes-incoming-history-20261015.json)
and [resolved report](coverage/pixel-routes-incoming-history-final-20261015.json)
remain separate from this uncommitted source review. Neither asserts a past gate
pass or fresh engine validation.


The final accounting review also updates the older counter/reader scenario
wording from seven families to six. Earlier execution claim hashes remain
unchanged and cannot certify that revised scenario definition. Their broad
integration freshness therefore stays pending until a new prospective run binds
it. Current live route proof is recorded under the already prospectively bound
`NP.pixel-route-scenarios.campaign`; the family-independent native CPU-reader
scenario retains its own evidence. No historical claim is regenerated.


## Longer camera readiness follow-up (2026-10-17)

The [current-master investigation](native-camera-resume.md) reproduces the longer
30-second post-Mini route failure as an original Hermes modal consuming camera
input. It adds bounded native-image readiness and ordinary dismissal to camera
stress and the Escape transition. Four fresh settled turns and all three selected
camera bodies remain required after interruption; diagnostic time cannot shorten
active stress. Original Mini callbacks independently identify Cancel and Quit.
This repair grants no camera3, original orientation-memory, CPU-keyed or complete
pixel/session proof. Two repaired GUI-resident acquisitions complete the camera,
Mini/Quit and independent GPU checks but fail their unchanged software 5 FPS floor
(4.9904 and 4.7642 FPS); those failed results remain preserved.

The dedicated-worker acquisition also fails that same software floor at 4.7611
FPS. [Targeted functional reanalysis](native-camera-resume-bound-20261017.json)
passes the retained camera, original callbacks/Quit, balanced retirement, zero
forwarding and independent GPU destination checks for GUI and worker owners.
All full campaign results remain failed; no performance threshold was relaxed.

## CPU-interior route follow-up (2026-10-17)

A separate nonresident native campaign reaches31/43 routes and explicitly
requires twelve formerly absent CPU interiors. Its final result passes875135
native calls,zero forwards/history refusal and normal Quit/private Wine stop.
The longer journey first exposed six genuine colour-rectangle forwards; exact
empty-clip bookkeeping is now native and passes256 additional original controls.
The final matrix independently compares every one of the earlier24 absent routes
in both formats, with264188432physical WORD checks across38 related routes.

Seven remaining live raster variants are original CPU-family alternatives;
five other entry windows still lack confirmed game callers. No route is removed
or forced. This source-bound isolated nonresident result does not renew the
concurrent resident camera journey or supply full presentation/World bypass.
See [results, reproduction and exact boundaries](uncovered-native-pixel-routes.md).

## Alternate CPU-context callers (2026-10-17)

The [controlled original CPU-input follow-up](native-pixel-cpu-callers.md)
executes seven additional alternate raster bodies without changing their game
callers, original dispatcher, drawing inputs or counters. Its successful frozen
default-policy journey observes31/43 routes/914079native calls with zero forwards
and history refusal, normal Quit and unchanged5FPS floor. The union across that
authored-family15 context and the retained physical-host31/43 result is38 routes.
Five natural callers remain unknown; camera3 is still unselected by the known
four-case table. No route is waived and no physical43-route run is claimed.
Source differences from current main remain explicit in the scoped result.
The independent empty-RGB candidate remains default-off pending live integration;
failed forwarding and presentation acquisitions are preserved separately.
