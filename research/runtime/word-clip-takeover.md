# Guarded clipped direct-word takeover

Opt-in `MNM_WORD_SPRITES=clip-takeover` (mode4) replaces only the admitted main
WORD plane at No-CD `0x596cb8` and `0x597086`. Existing modes remain explicit.
Original auxiliary drawing and simulation stay active.

The contiguous direct-word frame and bounded canvas/clip/model preflights remain
mandatory. A guarded copy is drawn completely before committing pixels and all16
recovered workspace words into the engine canvas. Original cdecl anchor mutation,
zero return, defined flags, selected caller registers/FP state and LastError are
retained. Source data and auxiliary payloads are never written.

Native admission additionally requires masked x87 exceptions, a supported
24/53/64-bit precision control and the next physical x87 push slot empty. TOP and
abridged tags come from the entry's saved caller FXSAVE image. Unmasked controls,
reserved precision and occupied push slots forward to original unchanged. Empty
requests also forward. These are intentional safety restrictions; fault delivery
and complete x87 instruction/data pointers are outside the comparison contract.

Mode4 diagnostics use MNMWTK01 and MNMWAT01 rather than shadow identities.
The common MNMWRD01 mode is4, bypass count records actual native commits,
original comparison count stays zero. MNMWTK01 field6 counts native admissions,
field10 actual original forwards, field18 zero. Sampling retains two distinct
clipped and six distinct interior auxiliary-bearing inputs for independent
original/native full-canvas and workspace replay.

Confirmed within scope (2026-10-10):

- Actual entry/route comparison:22,048 cases,22,000 native body-zero positives
  including17,600 auxiliary-bearing variants,48 empty original-once forwards.
  All110,240 five-way forced refusal checks preserve selected original results,
  classified reason, exactlyone original call and LastError.
- Four rounding modes across24/53/64-bit precision;22,000 unmasked IE guards
  and2,240 occupied push-slot guards match original-once execution. The latter
  are restricted to masked vertically hidden frames to keep original overflow
  conversion away from destination pixel addressing. Fault delivery excluded.
- ASan/UBSan passes1,440 placements,79 atomic refusals and3 empty noops.
- Wine four World startup observations report13,312 native commits including
 705 clipped,zero original forwards/comparisons/refusals/error/stop. All observed
 calls use forward0x596cb8 and53-bit x87 precision; live scalar remains open.
- Eight distinct auxiliary-bearing captures(two clipped,six interior) match
 unchanged original and independent native replay across1,420,800 fullcanvas
 WORDs andall16 workspace words. This establishes equivalence for the retained
 eight-input cohort; it does not compare every one of13,312 reported commits.
- Existing mode3 regression remains22,048 matches and110,240 classified
 refusals; historical live-shadow records retain their original hashes.

The first mode4 run passed all computational checks but failed the harness's
expected occupied-slot count(1,280 declared,2,240 actual). Extra bottom/corner
placements are also completely hidden for one-row frames. The failed result is
preserved; scope/count corrected prospectively and the final execution rerun.

[Isolated result](word-clip-takeover-isolated-20261010.json),
[live composition](word-clip-takeover-live-20261010.json),
[independent replay](word-clip-takeover-replay-20261010.json) and
[shadow regression](word-clip-takeover-shadow-regression-20261010.json) pin source
and prospective contract hashes. Full per-case/command outputs remain under
working/. Committed-history review coversd412789..0191a55 with42 file and94
behavior transitions,zero unresolved gaps; concurrent journal insertions retain
all earlier receipts unchanged and in relative order.

No default enablement,all-assets/indexed/payload/full-session or performance
claim. Next broader work: original font/text consumers and owned live glyph
composition, auxiliary/effect and indexed routes, additional camera/battle
scenarios, sustained delivery and session recovery. These are separate gates.
