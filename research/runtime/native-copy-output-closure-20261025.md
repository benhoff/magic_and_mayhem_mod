# Caller-bound native copy output closure

The missing proof was CPU-backed keyed-copy output. The existing live observer
selected only copies with World ancestry. It counted application keyed calls,
but none entered that selection; routing counts could not establish their pixels.
The new opt-in `all` selection checks every native COPY in a bounded session.
Version2 application metadata binds each emitted sequence/resource interval to
its actual pre-call geometry, key, history state and return PC. Default rendering
and the existing World selector keep their separate boundaries.

The native observer captures the source and prior destination, executes the GPU
command, captures the actual destination, then applies an independent CPU
transform and compares every destination word, including unchanged pixels.
A separate Python implementation replays retained source/prior/output words.
Captured or expected pixels never feed rendering, World providers or CPU-history
restoration. Command geometry/key checks also bind GPU pieces to the original
application arguments; internally consistent but wrongly bound pixels are refused.
Both CPU-keyed and GPU calls must include original-module callers.

## Current execution

All results below retain prospective scope/source declarations. Compiler inputs
are a reviewed superset; sources stay stable through execution. Original artifact
consumers verify the original manifest before and after. FPS is diagnostic.

| Check | Result and boundary |
| --- | --- |
| Python and C controls | 55 Python methods pass, including altered keys/geometry/resources/callers, absent draws/witnesses, mixed/incomplete census and unused-tail refusals. ASan/UBSan version1/version2 controls pass at65536 rows and counter exhaustion. |
| Actual OpenGL | No-World CPU-keyed and opaque destinations pass; deliberately omitted drawing refuses. The legacy selector still skips non-World readbacks and checks its two World copies. Cleanup and overwrite refusal pass. |
| Owned facade/production bridge/GPU | All23 clipping/self/keyed/bootstrap/empty/late-failure/integer-extreme/capacity cases pass;27 prepublication refusals and7 publication failures pass. The256-piece cases, single frozen alias snapshot, private CPU restoration and legacy compatibility pass. No original EXE is executed in these controls. |
| Actual ordinary all-copy game | `run-0o5z4v73`:3664 emitted COPY commands checked and caller-bound:2943 CPU copies, including1260 keyed calls, and721 GPU copies. Four CPU-keyed pixel witnesses retained;12 retained cases independently replay3873360 destination words. No failed/unprepared active call. |
| Ownership and history in that game | 65 completed/imported/returned Worlds, no pending World,49 explicit typed-Lock CPU restorations, no ordinary World/native/RGBA readback, Mini/Cancel, QuitNo/resume and original normal Quit.25 fills use UPDATE and receive no COPY-output credit;7 terminal-tail callbacks are counted exclusions. |
| Diagnostics-disabled storage | `run-ch81m6zk`:568 balanced Worlds,40 explicit CPU restorations,Save/Load/resume,two display rebuilds,Main Load/resume,normal Quit. Four fresh source-generation checkpoints retained. This functional pass is separate from all-copy proof. |

Positive evidence IDs are `COPYOUTPUT.controls03-20261025`,
`COPYOUTPUT.native-gl04-20261025`, `COPYOUTPUT.gpu-plan04-20261025`,
`COPYOUTPUT.live-copy02-20261025` and `COPYOUTPUT.live-storage02-20261025`.
Their [immutable SQLite-backed reports](native-copy-output-results-20261025/)
retain exact sources, declarations, actual results and raw-run paths.

## Original World raster checkpoints

`COPYOUTPUT.storage-original01-20261025` compares before-Save,after-Save,
after-Load and after-rebuild/MainLoad source generations39,155,323,481.
Each480000-word RGB565 destination matches unchanged original raster code;
all1920000 words have zero mismatches. The original reference receives a
diagnostic native entry canvas. Two additional authored entry baselines also
agree for all words in these four draw lists. That is evidence for those two
histories,not general baseline independence or complete presented-session/save
state equivalence. Native rendering never consumes the original reference.
Nine GPU-reader family controls,sparse writes,keyed copies,unknown-history
refusal and zero remaining native surfaces pass. Sources and inputs stay stable;
original manifest bookends pass. The new all-copy policy keeps original
comparison none: this is a separate,existing World raster contract.

## Preserved failures and limits

The earlier all-copy storage attempt `run-c3jb38xf` checks8137 copies before
exhausting the unchanged32GiB readback budget. It sees11628 copies, marks the
comparison truncated and never earns completion credit. The resident producer
later cancels at Save. Budget exhaustion and producer cancellation are both
observations; their causal relationship is not established. The successful
separate diagnostics-disabled journey does not make the failed run pass.

An initial Python corruption control exposed a missing source/destination extent
check; the validator now refuses that mismatch and the controls were rerun.
Private RAM build visibility, host compiler-temp quota, a cold Wine bootstrap,
missing compatibility binaries, a failed Xvfb wrapper cleanup and an automatic
compiler-input declaration refusal are retained as setup/control failures.
Unique intermediate source bytes are retained in the task's source-variants
SQLite database. Old report hashes are never refreshed.

Both selectors retain8192-copy/32GiB readback limits. Hitting either is incomplete
proof. The all-copy option is diagnostic, not a default runtime policy or a
sustained performance benchmark. Full-session output and physical scanout are
not established by these observations.

## Remaining native rendering boundaries

- Natural clipped, self-alias and keyed-GPU-source calls are zero in the ordinary
  live run. The23 owned cases validate their implemented native policy, not their
  natural reachability or original DirectDraw-driver equivalence.
- All43 optional handlers already exist. PCX words(0x54b5f0),brighten(0x58d770),
  row copy(0x58d3b0),raster15(0x57e1b0),camera3(0x5532f0) still have no confirmed
  natural callers. Their provisional unused-candidate classification is not
  proof of dead code or permission to remove them.
- Typed-Lock restoration is observed. Arbitrary raw readers, complete original
  ownership, unsupported storage histories, restored simulation equivalence,
  all-game routes and physical driver/context-loss parity remain open.
- Independent native-policy CPU expectations are not original-driver comparison.
  The new policy has scoped implementation/live observation; its original
  comparison and live replacement statuses remain none.

Reproduce the bounded live proof with `tools/test-native-campaign.py` using
`--resident-world --other-pixel-producers --no-pixel-forwarding --normal-quit
--gpu-copy-diagnostics --copy-diagnostic-scope all --legacy-audio
--stress-seconds 10 --fps-policy diagnostic --difficulty 1 --menu-cycles 1
--skip-movies`, an admitted idle prefix template, owned installed-game inputs and
prospective claims expanded with `rendering_seventeenth_controls.source_closure`.
The retained task orchestration records the private namespaces, Mesa driver,
build settings and exact commands. Storage runs use a separately declared
Save/Load scenario and disable copy diagnostics.
