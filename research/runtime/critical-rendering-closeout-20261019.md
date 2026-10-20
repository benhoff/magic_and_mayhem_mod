# Critical rendering acceptance and scope reconciliation

The selected acceptance target is **correct output** using the explicit corrected
Wine upload profile. The user chose this target and asked to preserve the old
Wine case as a diagnostic regression. This is a native acceptance policy, not a
claim that the original driver always produced correct pixels.

The complete corrected-storage cohort passes 222/222 frames and Quick Battle
passes 323/323 on both llvmpipe and NVIDIA. Each frame retains independent GPU
and physical comparisons, paint ordinals and physical marker admission. All
corrected source/default witnesses match. Tolerance remains one output unit;
alpha remains excluded. Neither original pixels, frame ordinals, references nor
native producer inputs were changed. All 238 old Wine frames are still checked:
ordinal 170 retains its 126 GPU + 126 physical pixel difference. That exact
expected difference is diagnostic; any additional or moved difference fails.

The new acceptance checker refuses incomplete cohorts, backend fallback, missing
witnesses, changed denominators, altered thresholds, unstable inputs and oracle
pixels supplied to native code. The software runner also now removes inherited
Zink/NVIDIA selectors before selecting llvmpipe. Previously a mixed inherited
environment could request a nonexistent Zink CPU device.

[Selected acceptance](critical-rendering-correct-output-20261019.json) retains
four PASS profiles and both expected diagnostics. Its full raw execution and
six unmodified per-profile records are retained in
[the results directory](critical-rendering-closure-results-20261019/acceptance-raw.json).
A source-selection review excludes only the uncompiled Qt session-lifecycle
header from the drawing executable's oversized declaration: none of the 49
retained compiler dependency files includes it. The old full declaration and
all recorded hashes remain unchanged; this does not validate that changed header.

Three canvas work assessments now read **implemented within scope**:
`RS.canvas-preworld-reuse`, `RS.canvas-producer-sequence`, and
`RS.world-initial-state`. Their original definitions already bound the selected
observed history and exclude global physical allocation/COM aliases and every
unobserved writer. Fresh replay matches all 1,061 full checkpoints and 16
independent unchanged-original World completions. The first-World continuity
check matches all 480,000 words and explicitly reports that physical allocation
ownership and all-writer observation are unproved. Selected original startup
fixtures and native lifecycle/refusal controls also pass. This resolves the
code assessment; it does not promote the raw partial recovered status or imply
that all sessions and original branches are recovered.

The [scope crosswalk](critical-rendering-scope-resolution-final-20261019.json)
separates supplied mechanisms, guarded refusals and excluded domains. Full
`NR.directdraw-retirement` remains an unresolved aggregate with three concrete
closure cases:

1. Bind every required original caller/indirect route to a supplied operation or
   justified refusal, and establish admitted backing generations/writer intervals.
2. Exercise physical context loss/Restore separately from the native failure and
   fresh-generation hard-restart policy; require no stale owners, fallback pixels
   or false normal completion.
3. Complete the declared 14-journey/47-requirement/21-operation matrix and sustained
   19/60 FPS profile with current source-bound evidence and owner retirement.

The UI shows passing selected profiles separately from broader pending acceptance,
preserves prior failures in history and displays exact required cases and closure
conditions for unresolved domains. Existing unrelated High-tranche source-link
additions are explicitly pending review; pinning their current bytes repairs the
UI's structural completeness without renewing their code or execution assessment.

The retained second-drive snapshot and raw artifacts are under
`/var/tmp/mnm-critical-closure-20261019` (pinned against cleanup). Refused and failed
attempts remain available. Original-manifest checks surround the original-consuming
experiments. Existing evidence is not rewritten. The separate committed-history
review for `34730ab8..0a7410fd` reports all 12 exact file pairs matched receipts,
no overwritten evidence and no unresolved history gaps; uncommitted source changes
remain separately accounted.

The independent retained-alias comparison now passes all nine Surface2/3/4
pairs and all 5,265 words across five complete planes per pair. Canonical identity
and alias-visible mutation remain correct after creator-surface release and
independent surface creation; all native owners retire. The 144 compared caller
slots contain 27 meaningful identity values and 117 reserved zeros. The caller
keeps its DirectDraw device alive until the aliases retire. An earlier diagnostic
releasing that device first faults in Wine on the next alias Lock; it is retained
and excluded from this native policy. This is a selected Wine system-memory
comparison, not original-game physical backing or all-writer proof. See
[the successful raw result](critical-rendering-closure-results-20261019/retained-aliases.json)
and [the final scope review](critical-rendering-scope-resolution-accepted-20261019.json).

For the isolated commit, both complete backend cohorts were rebuilt and rerun
against HEAD plus this closeout, excluding unrelated renderer/build edits.
All four selected profiles pass again; both legacy cases retain the exact
expected difference. The new immutable result is
[the isolated-commit acceptance](critical-rendering-commit-results-20261020/acceptance.json).
The standalone alias result has a separate compiler/import closure retaining
every executed input and excluding unrelated World/Qt declarations.
