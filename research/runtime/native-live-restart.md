# Actual-game resident failure and Hard restart

The bounded acceptance harness injects the existing terminal command-producer
failure state only after resident World imports and visible paints. It then
observes the normal shell failure path, waits for a stable terminal state, uses
the public Hard restart button, enters World through a fresh campaign and exits
through the ordinary original Quit callback. Both GUI-resident and dedicated-worker
World owners pass with the final repaired sources. This is live observation of
intentional native policy; original comparison and live replacement stay none.

## Confirmed failure and repair

The worker route reproduced a real shell crash. After `residentFailed()` reset
`renderWorker_`, the same timer callback called `pendingCpuRead()` on that null
owner. A separate GDB execution records SIGSEGV, `NativeRenderWorker::pendingCpuRead
(this=0x0)` and the polling callback at its pre-fix `main.cpp:280`. The old source
and full backtrace are retained; confidence in this native failure is high.

The callback now returns immediately after terminal cleanup. The final worker
execution reaches Hard restart and fresh gameplay without the crash. This does
not change drawing, recovery admission or the original simulation.

## Final bounded results

[The source-bound combined result](native-live-restart-bound-20261017.json)
retains two independent actual-game journeys and 1,554 unchanged source hashes.
Both use Apprentice, V3 command transport, private Xvfb and llvmpipe:

| World owner | Stable failed state | Fresh normal-Quit completed/imported/verified returns |
| --- | --- | --- |
| Dedicated worker | 519 ms | 7 / 7 / 7 |
| GUI resident | 541 ms | 5 / 5 / 5 |

Each journey presents live World before failure, injects command state FAILED
with reason INVALID, verifies all nine terminal-state predicates, proves no new
failed-generation presentation during the interval, clicks the public Hard
restart button, verifies a different launcher/channel/World nonce and the old
launcher's absence, then presents fresh World and completes original Quit Yes,
report Continue and Main Quit. Both fresh original processes exit zero. Private
Wine shutdown/wait and both 2,927-file immutable-original manifest bookends pass.

The failed worker session has completed/imported/verified counts 4/3/5 and a
cancelled pending generation. Failure cancellation discards that generation;
normal balanced retirement is asserted only for the fresh session. No transparent
continuation or reuse of the failed generation is claimed.

Supporting controls pass the real process-group/prefix kill/wait, toolbar and
fullscreen shortcut, staging cancellation and failure cases; four synthetic
native GL failure/fresh-worker cases; and four report-validator tests with 32
acceptance/refusal checks. These controls are separate from the actual game.

## Reproduction and retained attempts

In an isolated checkout with a verified `working/source-game`, matching
`working/manifests/game-nocd.tsv`, and a stopped owned DirectPlay test prefix:

```sh
python3 tools/draft-coverage-claims.py --behavior HOST.resident-live-restart \
  --scenario HOST.resident-live-restart.campaign --output working/claims-next.json
python3 tools/working-retention.py run --reserve-gib <estimated-local-output> -- \
  python3 tools/test-native-live-restart.py --claims working/claims-next.json \
  --source-game working/source-game --prefix-template <owned-stopped-prefix> \
  --independent-world
```

Repeat without `--independent-world` for the GUI-resident owner. The prefix
template's ownership and DirectPlay hashes are verified before cloning; only the
new private copy receives its new location-bound marker. No template bytes change.
Use the main checkout's storage guard when the isolated output is on another drive.

All runs remain under `working/tests/native-live-restart-20261017`, a retained
second-drive task containing its registered isolated checkout. Final worker run:
`checkout/working/tests/native-live-restart/run-s0ex0bja`; GUI run: `run-ng70c8ph`.
The task also retains the initial source-stability refusal, prefix ownership
refusal, missing installation-manifest launch refusal, pre-fix GUI success,
untraced worker crash and GDB worker crash. Setup refusals are not renderer
failures. The earlier GUI success is historical after the worker fix. See the
[provenance and attempt inventory](native-live-restart-provenance-20261017.json)
and [unchanged crash acquisition](native-live-restart-crash-20261017.json).

This is intentional native lifecycle policy, not recovered original recovery.
The injection neither replaces the original simulation nor claims the original
writer's role or changes its cursors/payload. A normal prefix-scoped stop and
wait precedes the fresh launch. The test uses an isolated prefix and staged
installations; the old and fresh launchers/channel identities must differ.

The terminal witness checks retired render owners, stopped polling/input timers,
empty hidden presentation, suspended/detached input, disabled retry/fallback,
and available Hard restart. All must remain true without new presentation for
at least 500 ms. Fresh World must import three generations and paint three frames.

Excluded: physical GPU/context loss, arbitrary game freezes, in-place recovery,
preservation of unsaved progress, audio/media restart, pixel equivalence and
concurrent restart/close races. The test selects legacy audio/media and skips
movies. Synthetic acceptance-report refusal controls do not grant live credit.
