# DRAW-02 step 2: actor latch continuity

This continuation lives on `draw02-step2-selection-20261012`, based on
`6afb42c3`, in a separate registered worktree. Concurrent cursor-mutex work in
`draw02-end-to-end-20261011` is preserved. This is a step-2 increment, not a
complete presented-frame acquisition or DRAW-02 completion claim.

The opt-in whole-pool recorder observes active actor raw +68/+70 fields
independently of the current hit. Exact callback call/phase identities and
completed original cursor prefixes bind the scalar observations. Two matching
collections are a consistency check, not atomicity or allocation ownership.
See [wire and exclusions](../formats/drawing-actor-latches.md).

Evidence is append-only. Earlier fixture compile/link refusals and the first
640x480 campaign attempt are retained as negatives. The input harness requires
800x600; that attempt refused before injecting any action, then shut down its
private session. The successful runs use the established DRAW-02 high-resolution
installation settings, pinned byte-for-byte by `prepare-drawing-reference-input.py`.
Every original-consuming preparation/capture verifies both original manifests.
No file under `original/` is written.

`draw02-actor-latches-original-02.json`/`analysis-01.json` first observe callback 2
changing wizard slot 0 raw +68/count from 0 to 1, with 674 exact cursor joins and
457 timely hover-away witnesses. `visual-01.json` corroborates the bracket after
click and pointer movement; those screenshots have unsynchronized journal-byte
brackets and cannot assert one exact original drawing frame.

An atomic first-owner admission refinement follows; the initial records remain
historical. The prospective register also initially attempted a file-backed data
anchor for the runtime-only pool address. It is removed without altering the
binary inventory: source/static evidence and runtime address documentation stay
in place; setter/resetter function links remain. Test/compiler dependencies are
explicitly linked. Corrected-contract execution is recorded under new IDs.

Validation covers the PE32 sampler, strict wire decoder, exact callback/cursor
joins, wrong owner/phase/record/coordinate/bitmap/count/bounds/loss refusals,
missing/changed actor and invalid-sample continuity stops, stale cursor exclusion,
and Region callback result/error/full FX state with latch sampling enabled.
Native test pages are distinct from original campaign evidence. All captures use
metadata mode: zero full primary planes. Original drawing remains active.

Use the main checkout's storage guard for each fixture and capture. A private
Wine prefix and private Xvfb display isolate execution; `LIBGL_ALWAYS_SOFTWARE=1`
is the declared environment. The capture CLI is:

```sh
python3 tools/capture-drawing-interactive.py \
  --source-game working/reference-input-highres \
  --prefix-template <completed-private-prefix> \
  --observer-mode metadata --engine-cadence --semantic-state \
  --hud-state --hud-input-point --region-input --actor-latches \
  --campaign-pause --campaign-guidance \
  --schedule tests/fixtures/drawing-journeys/hud-hit-camera-schedule.json \
  --claims <fresh-prospective-declaration> --report <new-report>
```

Draft declarations with `draft-coverage-claims.py` for
`NR.drawing-actor-latch-observation` and `NR.drawing-region-mouse-observation`,
scenario `DRAW02.actor-latches-original-callbacks`. After capture, independently
execute `analyze-drawing-actor-latches.py` with a fresh declaration for
`NR.drawing-actor-latch-binding`/`DRAW02.actor-latch-metadata-analysis`.
Never refresh old source hashes to claim current support.

## Remaining step-2 acquisition boundaries

The new evidence concerns sampled raw latch persistence for one original wizard.
Actor allocation/generations, uninterrupted selection and full selection/tooltip
semantics remain open. Region callback whole-function/control dispatch recovery,
modal/guidance source ownership, other drawing threads and source/physical frame
causality remain incomplete. Numeric pool IDs cannot bridge allocation reuse.
The older two-stage protected source chain covers its admitted cohort only;
excluded frames, unobserved uploads, alternate/GDI/movie routes, hardware/scanout
and general resource/handle lifetimes remain open. No global frame-completeness,
unobtrusive observation, native parity, live suppression or 60 FPS claim follows.
All 47 journey requirements/21 operation gates and steps 3–7 remain open.

Final corrected-contract result: `draw02-actor-latch-region-fixture-04.json`
passes 32 paired calls and 35 forwards. `draw02-actor-latches-original-04.json`
retains 693 samples, 15 Region pairs, 19,994 API records and normal original Quit.
`draw02-actor-latches-analysis-03.json` links all 693 samples, finds 635 timely
cursor points and 490 hover-away witnesses. The observed wizard's raw latch and
count change 0 to 1 across callback 2. 540 subsequent same-identity sampled latch
observations precede the first observed boundary. No full acquisition gate closes.
