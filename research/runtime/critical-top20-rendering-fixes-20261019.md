# Critical top-20 rendering fixes — 2026-10-19

Reviewed the first twenty registered partial entries by descending importance,
then behavior ID. All twenty are Critical (5/5). The selection is frozen at
`34730ab8`; later unrelated work and commits are preserved. These are contracts
of different sizes, not twenty identified missing renderer features.

The confirmed session defects are fixed: receipt admission is exclusive even
for dangling symlinks; failed completion/retirement writes cannot grant success
or a new generation; late completion cannot rewrite a stopping/retired session;
normal completion records renderer retirement and the independent checker
requires exact schema, boolean retirement flags and integer exit codes.
The regression failed before the fix. Current Qt and Python controls pass.
Confidence is high for these reproduced transitions; no new original-game
launch/restart or complete replacement is claimed.

Two control defects are also fixed. Resident-bind collection now drains final
published bytes after child exit; deferred-consumer baseline/timeout/publication
schedules reproduce that boundary deterministically. The checkpoint uncertainty
fixture now misses a real metadata mutation, preserving negative admission;
a complementary skipped read-only description remains eligible for attachment.
All 21 checkpoint cases and 330 complete frame comparisons pass. The newly linked
negative/positive pair was executed again under its new scope identities.
The SFT synthetic budget test now includes the existing 12-byte owned run:
33 bytes refuse and 34 admit the metric/pixel/mask/run fixture. Production
allocation limits were not relaxed.

| Critical contract | Result and boundary |
| --- | --- |
| `NP.native-cpu-instruction-resume` | Five instruction classes, 45 architectural-state and 875 FX-byte assertions; 150 transform cases pass on the frozen snapshot. |
| `NP.native-only-generation-restart` | Fixed late completion and failed-retirement persistence; Qt process-tree/restart and current receipt regressions pass. |
| `NP.native-only-launch-contract` | Fixed exclusive receipt admission, failed completion persistence, renderer-retirement reporting and strict independent terminal validation. |
| `NP.native-output-ownership` | Host isolation/admission unit suite passes. Existing original-game acceptance remains separately historical. |
| `NP.pixel-forwarding-guard` | Actual PE32 installed/missing/duplicate/unknown/forwarding controls pass. Unexercised game routes remain open. |
| `NP.surface-final-word-read` | Both native allocation routes pass 64 caller cases and four final-word reads each, with zero remaining surfaces/devices. |
| `NR.callback-publication-isolation` | Worker publication and deterministic after-exit drain pass; callback pumps and owned-lease violations remain zero. |
| `NR.canvas-producer-replay` | Native sequence and CPU producer controls pass. Complete new live producer/queue closure is not asserted. |
| `NR.command-checkpoint-attachment` | Corrected stale uncertainty fixture; all 21 cases/330 frame comparisons and two freshly linked read-only/mutation cases pass. |
| `NR.directdraw-retirement` | Native facade and retained GPU resource controls pass their bounds. Complete required caller/writer/alias closure remains unresolved. |
| `NR.displacement-finite-edge` | Native GPU retained-destination/strict-default displacement controls pass. |
| `NR.drawing-storage-session-comparison` | All 238 unchanged reference frames compared. The same 252 pixel comparisons fail at ordinal 170; no tolerance/reference change. |
| `NR.palette-resources` | 21 GPU frames/15 refusals and two actual PE32 lifetime/recreation schedules pass on the frozen snapshot. |
| `NR.readonly-description-continuity` | Eight description/mutation schedules plus freshly linked accepted read-only and refused missed-mutation checkpoints pass. |
| `NR.recorded-hardware-session-parity` | Hardware-selection guards pass. New NVIDIA presentation validation is pending: this user session is headless and existing X11 displays deny access. |
| `NR.recorded-physical-frame-admission` | All 238 software presentation frames have same-image ordinal/paint/swap receipts; pixel parity remains independently failed. |
| `NR.resident-bind-admission` | Baseline/tracker/reentry controls pass. Fixed final-prefix host drain race; deferred baseline/timeout schedules pass on current sources. |
| `NR.resident-command-pressure` | Native/sanitized deterministic tests and actual PE32 pause/stall/cancel/invalid-ACK admission controls pass. |
| `NR.world-history-channel` | Four focused Qt/GPU tests and twelve actual PE32 publication/refusal schedules pass on the frozen snapshot. |
| `NR.world-live-history` | Same finite synthetic controls pass; a new installed/live original-equivalence journey remains separate. |

The complete unchanged storage replay still fails at ordinal 170: 126 native GPU
and 126 physical pixels differ, while all source/default witnesses match. All
238 physical frames have paint/swap and same-image ordinal identity. This is a
negative parity result, not a reason to remove the native cursor, relax tolerance
or replace the original reference. NVIDIA presentation was not executed: the
user confirmed this session is headless. Existing corrected-driver/hardware
results elsewhere retain their own source snapshots and acceptance policies.

No raw understanding/implementation/comparison/integration/replacement milestone
is promoted. Existing bounded mechanisms and deliberate refusal domains remain
distinct from full DirectDraw closure and complete-session acceptance. The
[execution index](coverage/critical-top20-execution-20261019.json) lists every
immutable new result, the failed attempts and their separate successful followups.
Original manifest checks passed before/after the recorded-reference experiment;
all output used the 90 GiB storage guard. No cleanup or archiving was performed.

Concurrent committed history `34730ab8..0a7410fd` was inspected separately:
twelve file versions have exact parent/current SHA-256 receipt chains, with no
unmatched pair. This is retrospective accounting, not a past gate-pass or new
engine-validation claim. See the [history report](coverage/critical-top20-history-20261019.json).

Final session review also checks explicit restart retry after failed request or
retirement persistence: restored storage permits a new request, while the first
failure remains intact. [Fresh Qt/Python result](critical-top20-results-20261019/session-retry.json)
supersedes the earlier session source snapshot; its bytes and hashes remain
unchanged. Other snapshot tests retain their stated freshness boundaries.

The scoped commit preserves eight concurrent source versions included in broad
prospective declarations as exact [frozen provenance inputs](critical-top20-results-20261019/source-inputs/README.md).
Their raw fingerprints are unchanged; explicit bindings retain the inputs without
including unrelated active implementations or claiming current validation for
them. The commit census describes only the committed source tree.
