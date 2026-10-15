# Resident native Save/Load journey

Reviewed 2026-10-15. The [live Apprentice Celtic1 journey](native-storage-save-load-passed-20261015.json)
passes same-session Save/resume/Load/resume/normal Quit with original serialization
and simulation, native menus, resident World and all six valid optional producer
families. The separate [original raster comparison](native-storage-world-reviewed-original-20261015.json)
passes all three captured World checkpoints. Full replacement remains false.

## Confirmed failures and fixes

The first [Save-entry failure](native-storage-panel-refusal-20261015.json) reaches
the original Mini Save callback and then native bevel route0 at original splice
`0x58de5a`. The native facade reports stale World CPU history, family1/route0,
generation53, and exits124 before pixel mutation or original fallback. Confidence
is high for this observed reader; it is not an arbitrary CPU-reader census.

`RenderPrepareStorageV1(action)` is a process-local PE32 stdcall service in
`MnmRender.dll`: action0 means Load, action1 Save; other values refuse. Native
Mini dispatch calls it before the corresponding original callback. Nonresident
mode succeeds without a transfer. Resident mode obtains callback/copy ownership,
preflights tracked GPU histories, rejects borrowed/unknown ownership, and uses
the existing ordered CPU_READ/session/ticket/generation/nonce transfer before
original panel Lock. It releases the tracker while waiting and retains copy
ownership. No native pixels or host pointers enter the menu wire. This boundary
does not add ordinary World Lock readbacks or change original serialization.
Failure branches beyond the existing transfer controls still need targeted tests.

That admitted Save and exposed a [second failure](native-storage-source-history-refusal-20261015.json):
the native source compositor then read destination-dependent panel pixels after
discarding its CPU World knowledge. Native source state now distinguishes
undefined, CPU-current and GPU-current words. Panel readers may explicitly
snapshot established native GPU history after publishing pending CPU writes.
The snapshot never defines previously unknown words. GPU-only keyed copies
propagate conservative authority; sparse uploads cannot overwrite intervening
GPU words with cached CPU bytes. Other readers keep their admission guards.

Actual GL controls verify sparse writes, keyed GPU copies, panel synchronization,
unknown-history refusal and zero final surface owners. The ordinary canvas
sequence regression also passes. Asynchronous resident CPU readers remain
outside this implementation: their existing readback guard still refuses them.

## Live result

The source-bound run `working/tests/native-campaign/run-qhrkx_qf` verifies:

- Original callbacks New Game, Apprentice selection, Enter, Mini Save, Save,
  Mini Load, Load, Mini Quit, original Yes, Defeat Continue and Main Quit/exit0.
- Fresh native World generations50/126/257 before Save, after Save and after
  Load. New source imports are required independently of cached viewport paints.
  Original storage completion and World resume events must fall between the
  storage callback and the next Mini action.
- A 351,580-byte original save with unchanged SHA-256
  `205c9ba01f4bfbf49616c77520913f96559c028193d4dd1bcfb9fc7e12b88a9c`
  across Load and normal-Quit merge into the isolated owned store.
- 310 completions/imports/verified returns, normal retirement, no pending World,
  zero optional producer forwards/history refusals, and3,472 native optional
  calls. All43 routes install;17 execute and26 are unexercised in this journey.
  This adds no routes to the earlier camera/spell coverage; previous evidence
  retains its own hashes and current-source freshness limits.
- 22 ordered process-side CPU snapshots, plus2 native source-compositor panel
  snapshots. Both are explicit CPU-reader work, separate from zero ordinary
  World/native/RGBA readbacks and zero per-World pixel replies. Reports expose
  total native readbacks as well as explicit producer snapshots and the ordinary
  remainder; the totals must reconcile.
- 80 native surface creations, peak55 surfaces/5,297,952 bytes, real DirectDraw
  disabled/absent, and software llvmpipe13.47 engine FPS above an explicit10FPS
  functional floor. This is not hardware throughput or complete COM teardown proof.

The World retains logical canvas2 through Load. Surface activity and existing
resource controls do not prove a changed-World-canvas handoff. New canvas identity,
other maps and viewport dimensions remain separate acceptance boundaries.

## Independent World comparison

The native replay consumes the contiguous source-only prefix through generation257
and emits entry/final diagnostics at generations50/126/257. It reads no original
oracle. A separate32-bit process maps the hash-pinned unchanged No-CD executable,
executes the exact admitted original raster entries and recovered original
additive/colour wrappers, and checks defined raster caller AX. All480,000 WORDs
at each checkpoint match:1,440,000 comparisons, zero mismatches,4,542 raster
caller checks and302 primitive draws in the three-checkpoint reference set.
The replay retires all native surfaces. Its reviewed report verifies all110
repository compiler inputs against the prospective declarations, including the
private original comparator. The earlier successful comparison remains immutable
under its own evidence ID; it does not bind the revised scenario/runner bytes.

The private original process first uses a diagnostic native entry canvas; those
bytes never seed native rendering. It also repeats each checkpoint independently
from authored all-zero and all-`0x5aa5` canvases. Both original outputs equal the
native final at every WORD in these selected checkpoints. Agreement under two
authored histories is bounded evidence, not a general history-independence proof.
These are World draw outputs before presentation, not an independent continuous
Qt/HUD/movie/session comparison or a complete restored-simulation comparison.

## Retained input boundary and reproduction

An [Initiate run](native-storage-initiate-input-refusal-20261015.json) completes
Save and fresh native World resume without forwarding/history refusal but fails
to reopen Mini after repeated Escape. The native input channel was active and
press generations advanced. The cause remains unresolved; tutorial gating,
original behavior and native input integration are not distinguished by that
evidence. Apprentice selection plus ordinary scene clicks/ Escape passes; it
does not establish an Initiate fix. Earlier driver/radio-button failures and raw
inputs remain retained separately.

```sh
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-campaign.py --save-load --difficulty 1 \
  --resident-world --other-pixel-producers --profile-resident --capture-resident \
  --legacy-audio --skip-movies --min-fps 10 --timeout 420 \
  --source-game working/tests/pixel-staging/run-qeshupqr/source-game
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-storage-comparison.py --campaign <report.json>
```

Original manifests verify before/after each consuming experiment. Raw game,
capture and comparison inputs are retained under `.retain` pins. The incoming
`c74e9c5b..938024ec` review covers44 exact file and108 full behavior fingerprint
transitions: [initial inspection](coverage/native-storage-incoming-history-20261015.json)
and [one retrospective pending receipt](coverage/native-storage-incoming-history-final-20261015.json).
This accounting makes no past gate or new movie-validation claim.
