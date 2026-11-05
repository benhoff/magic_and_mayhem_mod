# Live worker pending CPU-read fault — 2026-11-04

This batch starts at `a8915d130` and adds `NP.worker-pending-cpu-fault`. One charged original-game journey reaches a decoded queued V5 CPU_READ, matches its independent producer request, fails the producer before native readback, then completes public Hard Restart and fresh normal Quit. Current acceptance is a replay over the completed live records after correcting a Python analysis assumption. The original automated report remains failed and immutable; no second game run or refreshed historical hash is claimed.

## Boundary and driver

The previous worker journey observed zero CPU materializations at both World points. A GUI timer cannot reliably observe a transient pending ticket. `LiveCommandRenderer::cpuReadQueued` therefore exposes the first decoded CPU_READ in a queued batch before that batch executes. `LiveCommandSession` and `NativeRenderWorker` forward the optional observer; configure it before polling. No host pointer or C++ object layout is added to the wire.

The opt-in `PendingCpuFaultTest` adapter runs on the render worker. The existing restart driver first requires actual World progress, opens Mini through input, arms the adapter and invokes the public Save action. If the Save dialog appears first, it submits a private test filename. The adapter waits at most one second for a matching committed row in the existing diagnostic CPU-reader journal, checks active transport/session identity and records the native counters/prior reply. It changes only producer failure state/reason through `mnm_ring_fail`; it never claims the writer or alters cursors/payload. A second bounded worker-only diagnostic wait captures the producer refusal. Existing terminal health checks then prevent the queued read from executing. After joining the worker, the adapter requires unchanged readback/materialization counters and reply hash, cleared pending state and zero native surfaces.

The journal is enabled with its existing diagnostic/pilot gate. The scenario uses native-only software llvmpipe, dedicated worker, decoded stream5, transport3, skipped movies, disabled native music and no full rendering capture. Quit confirmation retains its existing bounded diagnostic images; this is not a pixel comparison or nominal player acceptance. A 60-second missing-pending-read deadline and the existing whole-journey deadline bound failure paths.

The current validator requires explicit worker/version observations, ordered arm/fault/cleanup/restart transitions, the decoded ten-word request, the independent original request/refusal, terminal no-reply/readback checks, distinct identities, old-launcher absence and one final normal completion. Old World-only records cannot pass pending-read validation. The first generation may terminate before the renderer poll notices its failure; terminal ownership/input/timer checks remain mandatory in either host path. Ordinary non-test failure and restart policy is unchanged.

## Exact observation and source phases

The first World event and fresh World event both have CPU materializations0. The later Save-boundary request is ticket1, surface86,800x600, physical history7, sequence2549. The producer journal records request row30, context9 and reason1 (GetDC). This is an observed GetDC request following public Save activation, not proof of successful Save serialization or recovered upstream caller semantics. The producer's matching reply-wait refusal is stage7, with elapsed151ms. No completion row exists for that ticket. The worker records zero prior/new native readbacks and materializations, no reply file, cleared pending state and zero surfaces.

Stable terminal cleanup lasts532ms. Public Hard Restart produces distinct session/channel/launcher identities and observed old-launcher absence. The failed generation is `restarted` with `rendering_accepted:false`; it gets no normal completion credit. The fresh generation retires7balanced Worlds,664native route calls, zero forwarding and zero surviving graphics owners/pixel bytes, and records original normal exit0. The first failed launcher has no `original-process-result.json`; its actual exit code remains unobserved. The refusal implementation's `ExitProcess(124)` is not substituted for a process receipt.

`pending-cpu-fault-01.json` reports an analysis refusal after the completed journey: the validator incorrectly required equality between the journal's canonical `GameSurface::object` pointer and the refusal's caller interface pointer. Here they are12534140and12534152. `runtime/render/owned_session.h:cpu_journal_surface` explicitly canonicalizes the journal object, while `game_cpu_read_refuse` records its argument. The corrected join requires nonzero recorded pointers and the unique ticket/session, preserving both addresses. It does not infer a stable alias offset or original ABI relationship.

Only `tools/test_native_restart_support.py` and its Python test changed after that execution. The native Qt, renderer, hook and compiler inputs remain byte-identical to the run. All26current Python controls pass, including distinct-pointer acceptance and wrong ticket/session/zero-pointer refusals. `live-retained-reanalysis-02.json` executes the complete remaining acceptance checks over the exact retained flow, raw journal/refusal, ownership admissions/outcomes, balanced resident report, graphics retirement, routes, compiler/source closure and stopped private prefix. It records old execution hashes and new analysis hashes separately. This is current analysis of unchanged engine execution, not a successful rerun of the acquisition command.

The initial replay's missing-first-exit-receipt refusal also remains retained. Replay02 keeps that status unobserved and requires the actually recorded fresh exit, restart outcome and old-launcher absence. The source-phase corrections change neither the old reports nor engine validation claims outside this scope.

## Controls and preservation

The initial Python negative phase exposes four missing-field admissions; exact source bytes and the failure are retained before tightening validation. The first C++ build refuses a missing explicit `QJsonObject` include; its logs and four exact intermediate source versions are retained. The passing second build covers25then-current Python cases, two worker-thread adapter cases (materialization/explicit-reader refusal forms), wrong original ticket, GUI injection and late-reply refusal, the existing actual-GL V5 read test and four terminal/fresh-worker cases. Production is rebuilt with `BUILD_TESTING=OFF` before the original attempt. The later pointer-analysis correction brings the fresh Python count to26; no native code changed or native test was silently rerun.

The GL control compares1,920,000WORDs over four successful reads, both failed/cancelled queued boundaries with zero terminal readbacks/replies, two fresh CPU sessions, five wire refusals and36malformed reply checks. `PR04.terminal-controls01` keeps this headless policy separate from `PR04.pending-cpu01` live observation. Understanding, implementation, comparison, integration and replacement remain independent; no original comparison/replacement status is promoted.

All520selected raw artifacts, both generations and the private host case are retained. Three complete trees pass dual-SQLite custody and real exact restoration before pinned retirement. Budgets, claims, negative phases, source worktrees, canonical inputs and operator sources remain preserved. New generated research reports are SQLite references; the physical claims file is a bounded required producer input. The committed-history review of `913af9c..a8915d130` accounts for586paths with no unresolved receipt gaps.

## Running the bounded scenario

Prepare a new complete prospective claims/source declaration and launch TSV for each execution, using the retained preparation operators as the exact recipe. Earlier declarations remain bound to their original source phase and are refused after analysis changes. Do not reset or reuse the retained one-attempt budget/report names.

```sh
python3 tools/test-native-owned-campaign.py \
  --native-only --restart --independent-world --pending-cpu-read-fault \
  --prefix-template working/fixtures/prefix-seed --source-game working/game-nocd \
  --qt-build-type Release --no-native-music --claims <fresh-claims-file> \
  --report <new-immutable-report-path>
```

Run this through the repository storage guard. Source fingerprints, admission, normal Quit and custody are assertions; FPS is diagnostic. The native targets are `pending-cpu-fault-test`, `native-cpu-materialization-test` and `resident-terminal-restart-test`; the GL targets run under Xvfb. Python controls are `tests/test-native-live-restart.py` and `tests/test-native-only-contract.py`.

Successful CPU reply delivery during an ordinary dedicated-worker Save/Load is the next bounded rendering check. Physical context loss, arbitrary failures racing after the final health check, repeated-run reliability, saved/in-place/media recovery, original pixels/ABI and nominal campaign progression remain separate. Diagnostic collection may refuse incomplete/timed-out evidence and does not prove every concurrent failure timing.
