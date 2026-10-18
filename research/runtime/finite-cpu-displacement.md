# Finite CPU displacement

The CPU displacement adapter now retains a destination pixel when its otherwise valid sample would leave the owned allocation. This closes the concrete allocation-edge forwarding failure recorded by the previous camera investigation. It follows the native World policy already selected in `compat/legacy/canvas_world.cpp` through `SpriteComposite::retainOutsideDisplacement`.

The recovered `mnm_raster_pixels` API remains strict. Its optional sample-boundary adapter calls a native policy only for an admitted displacement offset in 0..16 whose low-WORD sample lies beyond the owned physical plane. The policy skips that destination without reading or writing it. Valid samples retain original linear addressing, including right-edge wrapping into an existing next row and owned stride padding. Negative/excessive offsets, invalid destinations, malformed RLE, aliases and mismatched formats do not become policy skips.

A complete preflight still precedes native writes. Its boundary callback has no side effects. Retained-pixel accounting changes only on the successful write pass. Original footer/workspace, selected caller ABI, flags/DF/FX and LastError behavior remain covered by the production-splice comparison. The renderer service policy header has no build-specific dependencies; recovered code accepts an optional storage-boundary decision, and the runtime adapter selects the intentional native behavior.

## Controls

- 38 guard-page/ASan/UBSan cases cover both RGB565/555, all 17 offsets 0..16, valid row wrapping, strict refusal, destination retention, late malformed rows, invalid offsets and invalid physical destinations. A NOACCESS page follows the owned plane.
- The full raster regression passes 9,323 native cases across all 23 splice sites, with existing atomic and workspace controls.
- 192 captured-shape cases cover 16 original sine phases, both formats and both DF states at the failing physical origin and two interior controls. Every production call is native; original-once forwarding is zero. Twenty-eight cases retain 60 unowned-tap destination WORDs. The strict API still gives 28 atomic refusals.
- 92,565,504 physical WORDs are checked. Every defined original sample matches actual original execution. Only the explicitly unowned samples are replaced by retention of their initial destination. The padded observation storage used to execute the original does not become native ownership.
- Five counter-decoder controls reject missing/truncated/wrong-magic/overflowed records, inconsistent pixel/call counts and branch counts above independently recorded native traffic.

The captured comparison uses the original `0x57dbd0` table initializer and pinned `0x5968a4` routine. In its recorded x87 context the table is `[8,11,13,15,15,15,13,11,8,5,3,1,1,1,3,5]`. No original artifact or binary inventory boundary changes.

## Initial live observations and remaining boundaries

Two source-stable short camera journeys complete ordinary gameplay, all seven alternate raster requirements, settled camera0/1/2, Mini Cancel/resume and normal original Quit, at the unchanged 5 FPS software floor. They execute 1,267,472 and1,225,121 native calls with zero forwards/history refusal. Their normal-exit finite counters are zero, so neither receives positive live branch credit for this repair. The second uses the failing acquisition's read-only RGB tracing context. The longer journey also passes with two 60-second active phases and 3,267,712 native calls, zero forwards/history refusal and zero retained-edge events. It therefore grants no positive finite-branch live credit. The unfulfilled camera declaration is retained in the raw acquisitions; no empty or falsely satisfied live scenario is published. The new policy has scoped implementation/headless integration, with comparison/replacement remaining none.

The authored CPUID context changes only raw family bits before the original initializer selects its own dispatch. It is distinct from a physical alternate CPU. No instruction/dispatch/drawing-input/counter writes force live route execution. All private Wine prefixes and original-manifest/source/input bookends remain part of acceptance.

Five natural callers remain unused candidates: PCX words, brighten, row copy, raster15 and camera3. Their native implementations and isolated checks exist, and the PE/module/reference investigation narrows their reachability without proving global dead code. This repair neither invents callers nor removes those routes from the 43-route denominator. Complete original GPU/session parity, arbitrary raw reader emulation and complete native replacement remain separate.

## Evidence and reproduction

Prospective claims selected `NP.finite-cpu-displacement` and its bounded control/camera scenarios before acquisition; only the control scenario is fulfilled and registered. The native policy is distinct from recovered original output outside valid memory. Earlier results, including the camera failure and 28 original-once fallbacks, retain their historical hashes.

Run `tests/raster-finite-displacement-test.c` with the raster/pixel kernels under address/undefined-behavior sanitizers, and `python3 -B tests/test-raster-finite-counts.py`. Run `tools/test-raster-displacement-boundary.py` with a new declaration and the exact retained `ab4c9b75` captured frame; it also executes the complete raster original comparison. For live investigation, use the owned original-game CPU wrapper and normal camera campaign. Require positive parsed finite-branch traffic before claiming that branch was observed; passing journeys with zero such traffic remain separate observations.

[Counter format](../formats/raster-finite-displacement-v1.md) describes the 208-byte record emitted only during normal process termination. No drawing, tracker, history transfer or exception path performs this file I/O. Abnormal or missing output is unavailable evidence. Raw acquisition, setup refusals and provenance are retained under `working/tests/finite-displacement-20261018` and `/var/tmp/mnm-finite-displacement-20261018`, with the main-drive retention guard outside the private namespace. Existing evidence and the cold archive are untouched.

The immutable [controls](finite-cpu-displacement-controls-20261018.json), [captured boundary](finite-cpu-displacement-boundary-20261018.json) and [acquisition catalog](finite-cpu-displacement-acquisition-20261018.json) preserve the exact result hashes and scope. Source coupling leaves older broader renderer/coherence records historical; this increment does not renew their original full-session claims. The arrived commit through `b6b4451c` was already reviewed in `coverage/producer-rest-arrived-history-20261018.json` with zero unresolved transitions; HEAD has not advanced during this task.

## Current producer-proof renewal

The affected older producer policies have fresh prospective evidence:29 parser/selector tests, six actual PE32 debugger controls,512 original empty-RGB cases/eight opt-outs plus5,028 positive pixel cases, and192 current strict/finite boundary cases. Focused original/unit records retain every required contract source and the reviewed executed import/compiler closure; their larger raw declarations remain immutable. Unexecuted build-only dependencies are not presented as extra engine validation.

The legacy `NP.raster-displacement-physical-boundary` contract now describes the retained strict API and its28 atomic refusals. Production behavior is explicitly the finite policy; historical original-once fallback is not asserted current. Earlier scope/evidence hashes remain intact.

The renewed original-game CPU/RGB/27-route witness passes1,347,324 native calls, zero forwards/history refusal,1,776 exact read-only RGB observations, all seven alternate requirements and normal Quit. Its shutdown record observes six finite-edge native calls retaining17 destination WORDs. Those facts are bound to the prospectively selected overlapping producer policies. The separate finite-policy live milestone requires its own declared confirmation and is not retroactively attached to that run. The first dedicated confirmation passes1,261,533 native calls but has zero edge events; all attempts are retained.

The renewed immutable [parser/selector controls](finite-cpu-displacement-renewed-unit-20261018.json), [original RGB cases](finite-cpu-displacement-renewed-original-20261018.json), [strict/finite boundary](finite-cpu-displacement-renewed-strict-20261018.json), [PE32 debugger controls](finite-cpu-displacement-renewed-debug-20261018.json) and [original-game caller witness](finite-cpu-displacement-renewed-game-20261018.json) preserve exact claims, raw report hashes and scientific limits. Five unused-candidate natural callers remain distinct from this observed allocation-edge branch.

## Final bounded observation boundary

Nine accepted source/input-stable game journeys execute13,406,642 native calls in total, with zero forwarding/history refusal and normal original Quit. One prospectively CPU/RGB/27-route-bound run naturally reaches the repair six times and retains17 WORDs. That is an actual original-game observation, not a synthetic route invocation. Five separately declared finite-policy confirmations all miss the rare edge. They do not retroactively acquire the earlier positive run's scope binding. The finite policy therefore stays scoped/headless in the register, while the overlapping caller policies receive their fresh live observations. Comparison and replacement remain none for this intentional native policy.

The [complete immutable catalog](finite-cpu-displacement-complete-acquisition-20261018.json) retains every pass, zero-event confirmation, setup refusal and raw count/source/claim hash. The unfulfilled standalone live scenario is not published as an empty or falsely satisfied coverage row. No helper, dispatcher, counter, drawing input, denominator or historical hash is changed to manufacture credit.

The implementation defect is repaired and has been observed in an accepted original-game caller context. Remaining scientific gaps are five unused-candidate natural callers, current live empty-RGB admission, the separate finite-policy scenario's positive-event scope binding, complete original GPU/session parity and complete native replacement. These are not missing implementations for the43 optional handlers. Broader pre-existing historical coverage debt remains separate from these current source-bound producer proofs.
