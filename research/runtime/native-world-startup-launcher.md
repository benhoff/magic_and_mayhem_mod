# Public native startup launcher

`tools/run-native-world.py --startup-history [--verify]` automatically reproduces
the bounded Quick Battle map2/zero-magic-items startup through16World queues.
Startup history always verifies every completed canvas; `--verify` remains
accepted and controls comparison in ordinary continuous launches. The command
uses the existing bounded original menu driver and the source-only producer
consumer with `--world-handoff`, launched before the original game. It retains
menu/loading/font/HUD producers, transfers reconstructed native storage into GPU
World drawing, and commits GPU results back to native history. Original drawing
and simulation remain active. There is no raster bypass or original destination
seed. Launches without `--startup-history` retain the interactive World-channel
viewer and original-window menus/input.

The launcher delegates staging, original-manifest checks, menu transactions and
cleanup to `capture-scene-game.py`. It requires a closed contiguous prefix through
World return16, validates the native stream hash and every completion identity,
requires CPU/GPU parity, and compares every RGB565 canvas to separately recorded
original output. Missing records/raw rows, failed native GPU parity, differences,
worker failures and premature closure fail the command. `native-world.json`
retains all completion comparisons, worker metadata, raw startup queues and
actual draw kinds outside the older World-only observer policy. Those diagnostic
kinds are distinct from producer admission failures. The native worker never
reads original destination oracles; comparison is external after completion.

The underlying producer session keeps its180-second deadline,65536record,
128MiBstream budgets. The public startup launcher explicitly requests a3GiB
original-oracle allowance; ordinary research captures retain the1GiB default.
`--producer-oracle-mib 1..3072` is a strict capture-side opt-in, passed as
`MNM_CANVAS_ORACLE_MIB` before hook installation. Invalid, empty or oversized
settings refuse before mutation. Oracle count9999 and fail11 remain enforced;
no checkpoint is discarded to fit the allowance. The public launcher skips the
optional desktop screenshot: exact comparisons read separate full RGB565
completion files and do not depend on ImageMagick or the desktop root window. This is an automatic finite startup
experiment, not interactive native menu ownership, sustained live performance or
replacement. The first successful public run compared1062canvases across16queues.
`tools/test-native-world-launcher.py` records prospective source/scope fingerprints
for the public entry point; current validation is recorded separately below.
`tests/native-world-startup-test.py` covers19completion/refusal cases and two
capture-dispatch cases, without original artifacts or native raster claims.

A manual producer-history attempt in `run-ekeoh6y5` exceeded the oracle budget
before World entry and contained duplicate/noncontiguous record sequences. Its
immutable diagnostic is
`native-world-startup-launcher-manual-refusal-20261008.json`. Mouse/menu activity
coincided with this failure; the responsible writer/thread and synchronization
contract remain unconfirmed. The launcher uses the already bounded automatic
menu route instead. Manual producer-history capture is a separate pending gap.
The earlier World-only manual queue2eight-pixel discrepancy remains preserved in
`native-world-manual-history-gap-20261008.json`; its omitted context/writer has
not been identified.

Concurrent batch-renderer work was preserved. A narrow conversion from the
renderer image's32-bit pixel container to RGB565 wire words fixes compilation in
that batch reply branch. This launcher does not enable batch replies, raster
bypass or proof pipes; the standalone batch codec test passes, while its live
replacement/guard behavior belongs to the separate batch contract.

The final public `--startup-history --verify` run is pinned in
`native-world-startup-launcher-verified-20261008.json`: all1062completed canvases
and474236912pixels match, all16startup queues retain their raw rows, and all176
prospective source dependencies stay unchanged. All74normalized repository
compiler dependencies are declared. Actual legacy World-observer unsupported
kinds16/17/20 each occur twice; the producer pipeline captures their admitted
raster work and retains the original traversal/preparation.2927original files
verify before/after. The earlier normal run remains immutable; later shared
hook/test edits make its full-source proof historical. Public-policy validation
is live observation, not raster replacement or a full recovered dispatcher
comparison promotion. The separate batch codec test and21Python controller
cases pass. Existing hook configuration is `.githooks` with its executable gate.

User-session diagnosis: `run-_g_wjtth` exhausted1GiB before World entry
(1118oracles,1073280000bytes). `run-scltemki` closed16World queues and
938native/original completions all matched, but the mandatory diagnostic
`import -window root` command failed before the child report was written.
The public route now skips that diagnostic and requests the bounded3GiB
allowance; missing child reports point to the existing experiment directory.
Actual hook budget tests exercise old-ceiling crossing, exact new-ceiling
acceptance, rejection above the ceiling and underflow prevention without
writing gigabytes. Live regression prepends a deliberately failing screenshot
utility and checks that it is never invoked. This change does not attribute
extra menu repaint activity to an original engine writer or thread.

The two user failure sessions are pinned without modifying their output reports
in `native-world-startup-launcher-portability-failures-20261008.json`. Budget
fixture evidence is `native-canvas-producers-oracle-budget-20261008.json`; its
original-equivalence and broader producer validation remain pending.

Fresh portability regression `NR.world-startup-launcher.portability-20261008`
completes16World queues and1062matching canvases/474236912pixels
through the public verified command with a deliberately failing ImageMagick
`import` executable first on PATH. That utility is never called. All176
prospective sources stay stable and all74normalized compiler dependencies
are declared;2927original files verify before/after. Exact raw startup rows
and all completion comparisons remain required. The synthetic budget fixture
proves the strict ceilings independently; this live run does not exhaust3GiB
or assert broader menu-route/whole-engine equivalence. Historical source-only
results retain their original scope and hashes.
