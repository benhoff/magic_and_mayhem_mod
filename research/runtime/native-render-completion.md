# Native rendering completion contracts

The four requested changes cover CPU history admission, combined producer route
accounting, terminal resident cleanup/fresh restart, and physical leases across
facade flips. These are separate policies with separate execution evidence.

## CPU authority

Resident GPU World output invalidates the matching CPU backing. Changed-byte UI
overlays do not reconstruct World history. Destination blends, displacement,
shadow, fades, panels and DC snapshots require current CPU history. Source copies
require current CPU history unless admitted through the explicit ordered GPU-copy
service described in [native-history-readers.md](native-history-readers.md).
A stale selected reader must terminate before either native mutation or forwarding
to the original CPU body. Full un-clipped fills and opaque copies from complete CPU history can establish it.
Independent partial writes retain stale authority. Raw arbitrary pointer reads
outside the selected adapters remain outside this admission contract.

## Producer accounting

The campaign harness must explicitly opt into all seven optional producer
families after cleaning inherited overrides. Per-route installed/native/forwarded
and history-refusal diagnostics distinguish execution from mere selection. A route
with zero calls is unexercised; forwarded calls prevent a full replacement claim.

## Resident failures

A failed resident session must release presentation leases, clear the viewport,
suspend input and offer the existing hard restart. Restart stops the old producer
before replacing channels and renderer ownership. In-place recovery is excluded.

## Held leases

CPU balance and physical DC ownership travel with swapped storage. Cached DC
identity stays with its logical object. Busy after a flip does not imply that no
swap occurred. The bounded driver schedules need fresh comparison evidence.

## Executed controls — 2026-10-15

[The final source-bound controls](native-render-completion-controls-20261015.json)
pass current-history queries, identity monotonicity/nonce admission, three terminal
stale-reader cases, partial/full fill authority, backing authority across a held
flip and final retirement. Four independent real Wine11.16 Surface2 schedules on
a focused exclusive 640x480 RGB565 double buffer match all HRESULTs and sampled
backing pixels. Both output hashes are
`b94b387948d6f617c0a0350ff250fed051cf76da5dee584986759a7f417e6930`.
The Wine driver hash is
`24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28`.
Focus is an actual driver precondition: unfocused schedules were retained as
failed experiments, not used to claim equivalence. Indexed8 remains the separate
historical renderer-backend evidence.

The same controls exercise malformed commands, producer failure, first-frame
timeout and GUI cancellation after presentation, each followed by a fresh actual
GL worker/session and frame. All GUI leases return. GUI cancellation models the
GL-error caller; physical context loss and actual-game freeze/restart acceptance
remain pending. The host latches terminal input suspension across menu/media
callbacks, releases rendering owners, clears presentation and exposes Hard restart
while the producer runs, or Launch after it exits. New launches clear the latch.

[The resource regression](native-render-completion-resources-20261015.json) retains
408 independent clipping cases, six HWND states, 80 five-format lifetimes and zero
final owners. Actual original/native splice regressions pass:

- [Raster](native-render-completion-raster-20261015.json): 9,323 native cases,
  1,557 forwarded/hidden cases and 111,702,784 physical WORD checks.
- [Panels](native-render-completion-panel-20261015.json): 22,272 native cases
  and 1,392 original-once refusals, including caller state and unlock effects.
- [Image/effects](native-render-completion-pixel-20261015.json): 5,004 native
  and 372 forwarded cases, 5,765,312 physical WORD checks.
- [Minimap](native-render-completion-minimap-20261015.json): 486 terrain,
  2,054 overlay and 756 outer-border native cases, with 276 forwarded/no-marker
  cases and 133,986,384 physical WORD checks.
- [Rectangles](native-render-completion-colour-rect-20261015.json): 2,585 native
  and 1,511 forwarded/hidden cases, 11,984,896 physical WORD checks.

## Remaining integration boundaries

The original safe-refusal milestone remains historical. The subsequent explicit
GetDC materialization and ordered GPU-copy services are documented in
[native-history-readers.md](native-history-readers.md). Other dependent readers
retain admission guards until their current history is established.
An arbitrary raw pointer read outside the selected adapters is not intercepted.
A readable CPU allocation is not evidence of rendered World history. The facade
held-flip schedules do not establish held-lock command-adapter integration or
in-place recovery. The default native host-memory no-loss policy remains separate
from physical driver loss.

The combined producer option remains partial: original forwards and unexercised
routes stay visible, and `complete_replacement` stays false. The subsequent bounded
normal-Quit combined pass is recorded in [native-history-readers.md](native-history-readers.md).
Independent full-session pixels, later-map transitions and strict complete-mode
admission remain separate acceptance work.
Hardware performance validation is unavailable in this session because the
installed NVIDIA stack cannot communicate with its driver. The explicit software
run is retained independently; no software fallback is labeled hardware proof.

Historical reports, source hashes and failed/interrupted experiment reports are
preserved. The incoming-history review covers b327c3a3..720c187b, 37 exact content
transitions and their receipt chains, with zero unresolved transitions; it makes
no past gate or fresh execution claim.

## Combined public gameplay refusal observation

[The final combined report](native-render-completion-combined-refusal-20261015.json)
verifies all seven staged producer DLL/compiler closures and all 53 route
installations. It records 533 native calls, 6 original forwards and 38
unexercised routes. The real gameplay producer terminates with exit124 on stale
CPU World generation1; the report correctly has `success: false`. Original
manifest verification and private Wine termination complete successfully. This
validates route observability and the terminal admission path; it does not
validate a complete gameplay journey. CPU materialization or a different admitted
representation is still required to let the dependency-sensitive reader continue.

The [generated coverage snapshot](coverage/summary-native-completion-20261015.md)
retains the independent stage/current-evidence accounting. The full audit report
is retained under `working/tests/native-completion-audit-final-20261015.json`.

The subsequent reader-routing change is documented in
[native-history-readers.md](native-history-readers.md). Its evidence supersedes
the first combined launch's reader identification and raster-admission limitation;
historical reports and their source fingerprints remain unchanged.
