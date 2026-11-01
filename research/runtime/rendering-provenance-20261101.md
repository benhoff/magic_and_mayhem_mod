# Cursor and destination-history provenance — 2026-11-01

This bounded EP06 follow-up starts at `4fa166010` and advances its owned source
checkout to committed master `a4d3a6ee3`, including the capture-worker repair,
before the final captures. Earlier source variants, failed attempts and exact
preimages remain immutable. [Final batch review](rendering-provenance-results-20261101/final-review02.json)
records the exact combined sources and per-case totals. No original file,
rendering admission bound, producer lease, watchdog deadline or gameplay rule is
changed. Native/driver comparisons, caller observations and player acceptance
remain separate dimensions.

## Confirmed findings

**The late pixel depends on prior destination data.** The original-only
[dependency trace](rendering-provenance-results-20261101/history-dependency03.json)
executes the pinned unmodified NoCD raster routines for the retained first
800×600 late frame. An optional per-record RAII watch changes neither the
baseline output nor the retained live/zero identities. Changing only entry
pixel `(563,433)` to0 or65535 makes final target `(548,433)` take that value.
Changing only the target's entry value leaves the entire output unchanged;
changing a neighboring source does not change this target. The watched source
value survives the retained prefix through record1261/backend9. This confirms
an actual prior-canvas dependency with high confidence. Same-value writes are
not identified, and the source's earlier writer/definedness is not reconstructed.
Original-before planes enter only these isolated original diagnostic processes;
they never seed native rendering.

**The two invalid rectangles receive matching low-level error behavior.**
The [provider control](rendering-provenance-results-20261101/invalid-copy-providers01.json)
creates actual DirectDraw1 RGB565 offscreen/system-memory surfaces without a
clipper. Wine builtin DirectDraw and the captured guarded native facade both
pass a valid WAIT fast-copy control and return `0x88760096` for the historical
invalid source rectangle and invalid destination rectangle. The keyed case uses
WAIT plus source-key0. Each provider checks1,036,800 complete source/destination
words; rejected copies preserve all checked pixels. Native object counts return
to zero and the private prefix retires. This is a supplied-context comparison,
not execution of the historical cursor callers or proof for unknown caps,
primary surfaces, other interfaces or physical drivers.

**Current cursor calls can be joined to concrete state.** The opt-in
[cursor companion format](../formats/cursor-copy-state-v1.md) repeats exact
selected v2 COPY rows and records original saved cursor state, published Qt
coordinates/extent, cooperative HWND and queried window/client/native-desktop
state. Its existing COPY lease serializes writes. A bounded8192-row stream,
closed census identity, exact row bijection and zero unused tail are mandatory.
An unstable publication stays explicit. The normal path never consumes this
journal. Published Qt state is not the actual preceding hooked GetCursorPos
return; saved restore rectangles can refer to an earlier cursor position.

The final640×480 software diagnostic completes normal Quit with64 selected
cursor calls, zero refusals,1,303 caller-bound COPY commands and2,796,560 retained
independent COPY words. All default comparison guards pass, including original
Right down/release acknowledgements and actual fullscreen World presentation.
The original oracle reconstructs84 Worlds/112,200 draws from one owned initial
source chunk;148 presented Worlds and212,644,040 viewport/history pixels match.
The final800×600 diagnostic also exits normally and passes the default original
Right-key and fullscreen guards, but its independent pixel comparison **fails**:
13,659 differing pixels across9 presentations out of165,997,240 compared pixels.
The first observed divergence is World50/paint162, with295 differing pixels;
earlier presented Worlds through49 match. The oracle reconstructs53 Worlds and
97,486 draws from one complete owned source chunk. The exact failing comparison
is [retained unchanged](rendering-normal-results-20261031/operations-provenance-high-comparison04.json).
Successful verification of this investigation does not assert pixel equivalence.

**Direct distortion is reachable in the current high-resolution player.** Its
complete producer stream contains2,146 operation3/backend9 calls, starting in
World2. This closes the route-level reachability question for this context.
It does not identify the cause of the later divergence: many presented Worlds
containing that operation match before World50. The full owned prefix, exact
native command/output stream and independent oracle outputs remain retained.
These diagnostics skip intros and enable extra observation, so they admit no
nominal default/software acceptance rows.

## Validation infrastructure corrections

The first cursor probes overlapped the independent Right-key exercise and the
short fullscreen observation window. Their input/fullscreen refusals are retained.
The final diagnostic sequence centers the pointer through the key exercise,
then exercises valid image edges/corners through ordinary Qt events. Its explicit
fullscreen holds are two seconds; ordinary test timings and all production
lease/watchdog limits remain unchanged. This separates test actions; it does not
claim a recovered explanation for every missing original key poll.

The default comparer rejected the first high run's Right-key acknowledgements.
A guarded `--drawing-only` mode can inspect such cursor diagnostics while
explicitly withholding input admission; it rejects nominal or differently bound
acquisitions. It still requires actual fullscreen presentation. The final cases
use the default comparer with every input and fullscreen guard enabled.

That diagnostic exposed a pre-existing accounting defect: a late fullscreen
exception could leave top-level `success=true` after a successful pixel comparison.
[The contradictory report](rendering-normal-results-20261031/operations-provenance-high-comparison02.json)
and its exact producer remain retained. Exceptions now revoke overall success,
and nominal admission rejects a collector carrying an error even if its success
flag is true. The [corrected repeat](rendering-normal-results-20261031/operations-provenance-high-comparison03.json)
retains matching scoped pixels with `success=false` for the fullscreen failure.
No failed report is rewritten or used as successful full acceptance.

Eleven cursor-reader controls reject mixed modules, altered call/generation/
argument bindings, missing or extra rows, truncation, overflow, bad reserved
fields and incoherent publication claims. Optimized and ASan/UBSan C controls
exercise complete row copies, capacity and saturating counters; the actual PE32
bridge builds without CRT imports. Six diagnostic-policy controls,14 normal
admission controls and10 pilot controls retain strict input/nominal boundaries.

## Remaining work and next trigger

The retained old cursor requests are out of bounds and the supplied driver
contexts corroborate the native refusal. Their historical coordinate writer,
actual preceding input return and exact provider/caller state are still absent.
Current no-refusal samples do not reconstruct that lost history. Keep geometric
admission strict. An actual recurring refusal should be captured with this
companion plus the input-return/caller boundary, not converted into success.

The next rendering investigation starts with the already retained high03
capture; another broad acquisition is unnecessary. Replay its complete owned
prefix through World50 and compare intermediate raster checkpoints against the
independent original oracle. Locate the first differing draw/read, including
source coordinate, row offsets, coverage and prior writer. Distinguish a raster
semantic difference from a history/ownership difference before changing code.
Retain a small deterministic regression for the isolated cause, repair it, then
rerun the failed high case and the passing low control with new evidence IDs.
The historical `(563,433)` writer/definedness remains a separate unanswered
question; current route reachability does not prove that identical dependency.
Never supply original-before pixels to native initialization.

All18 EP06 operation requirements remain pending acceptance in both required
configurations. Current scoped COPY, input and full-frame comparisons are leads
for those rows; they do not close a whole operation/configuration automatically.
The six existing default-player leaves, separate original-recovery verdict,
software verdict and EP09 fault work retain their own scopes. No product-scope
choice or balance judgment is needed for the next evidence acquisition.

## Storage and accounting

Generated evidence uses SQLite references. The first four completed capture
cases transferred all source snapshots, raw files and `.retain` contents to both
stores, passed real restores and reclaimed5,165,453,312 allocated bytes. Active
combined-source analysis was excluded. Restore through the evidence-workspace
tool before any later physical consumer; readers can use retained reports directly.

The [new-commit history review](rendering-provenance-results-20261101/committed-history02.json)
checks3,580 first-parent transitions across `4fa166010..a4d3a6ee3`. It retains the
first report's missing-raw-receipt findings, then fills1,753 unique evidence-file
transitions through the SQLite journal. It asserts no historical gate pass or
fresh engine validation. The owned source advance has a mirrored source snapshot
and retained recovery stash `261e3a3d7921d8231ad5ab6187a8bf475bf727f6`.

Run `tools/test-cursor-copy-state.py`,
`tools/trace-rendering-history-dependency.py`,
`tools/compare-cursor-invalid-copy-providers.py` and
`tools/run-rendering-acceptance-pilot.py --scenario normal-quick --operations-provenance`
with new immutable report identities and the machine storage guard. The existing
independent checker and `tools/review-cursor-copy-provenance.py` validate each
stopped capture; `tools/review-rendering-provenance.py` joins the bounded batch
without admitting acceptance. Execution intermediates stay under ignored
`working/`; source/control evidence and negative attempts are retained in SQLite.

The final coverage audit has no errors and every new generated research artifact
resolves through a verified SQLite reference. The new observer/tool contracts
have current scoped support; unrelated historical validation warnings remain.
The [final summary](coverage/summary-operations-provenance-final-20261101.md)
retains those separate dimensions.

Physical replay of these existing absolute-path-bound acquisitions uses the
retained owned checkout `working/worktrees/operations-provenance-20261101`.
Restore its completed execution directories with `tools/evidence-workspace.py`
before a physical reader. The failed high03 comparison is a preserved negative
result; rerunning the unchanged comparison should still fail. Use a new report
identity for every later comparison or repaired-source acquisition.
