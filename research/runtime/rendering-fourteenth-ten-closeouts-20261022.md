# Fourteenth rendering batch: startup and finite World history

This batch retains the existing scope of ten registered implementations:

| Behavior | Completion work |
| --- | --- |
| RS.canvas-create-lock | Creation, descriptor/lifetime bookkeeping, null/lost/restore/retry and conditional unlock fixtures |
| RS.canvas-bind-clip | Ordinary/padded bindings, stride/height and separate clip state |
| RS.canvas-startup-clear | Full-fill low16 colour and scripted lost-surface retry/fatal outcomes |
| RS.world-startup-queues | Exact finite prefix, raw row/kind retention, malformed and effective/default admission |
| RS.world-startup-wave | Cold/warm lazy initializer and complete opaque/split-run clipping canvases |
| RS.world-startup-palette | Real shade-minus127 fallback, shade/shift/neutral palette matrix |
| RS.world-dispatch-targets | Pinned table and source-token primary/auxiliary/software routing matrix |
| RS.world-kind8-virtual | Static class/method pairs, read-only diagnostics and owned wrapper routing |
| NR.world-history-channel | One-use slots and strict acknowledgement, reset/sequence/identity and PE32 journal admission |
| NR.world-live-history | Pending terminal-state refusal, asynchronous prepare/upload, ordered ENDED drain and cleanup |

Two concrete native history defects were found by inspection. A consumed journal
slot could be republished with a new contiguous sequence although v2 says each
slot is one-use. The host now tracks consumed slots and cancels on reuse in
WRITING, READY or READING state. A producer failure, cancellation or changed
session during owned background preparation previously went unchecked until a
later channel poll. Each session tick now validates the channel before adoption;
controlled blocked-worker tests require immediate refusal, no presentation and
complete cleanup. Nonzero acknowledgements must name the latest consumed frame
and advance exactly once. This is native policy, not recovered driver behavior.

The new runner declares source/contract hashes prospectively, verifies compiler
dependencies, leases an idle marked fixture prefix exclusively and retires all
Wine clients. It composes existing native fixtures with assertions enabled in
Release, consumes original bytes only in mapped reference processes, and compares
complete independently initialized wave canvases externally. Direct report writers
store generated results in SQLite; bounded physical claims/JSONL outputs are
explicit tool interchange. No original destination pixels enter native rendering.

Original creation/Lock/fill APIs are scripted reference boundaries. No real
DirectDraw allocation/alias lifetime, unobserved writer closure, physical loss or
restore, other startup seeds, arbitrary queue/object/thread semantics, installed
live-game continuity, whole-game parity, performance or replacement is inferred.
Historical comparisons and failures keep their exact source hashes. Other stage
statuses remain independent. Final current execution will be linked below.

[Current execution](rendering-fourteenth-ten-results-20261022/attempt-03/controls.json) passes 4 Python controls, 39 recorded commands and 2646 stable prospective inputs. All768 independently initialized complete wave canvases match unchanged original WORDs on CPU, software OpenGL and actual `NVIDIA TITAN RTX/PCIe/SSE2`; each backend compares 1572864 words. Selected creation/Lock/bind/clear branches, 5472 dispatch cases/6084 calls, 1440 class-method cases and 480 empty-method cases pass. The ABI callback verifies register/flags/floating-point/LastError preservation and real shade-minus127 capture. Ten PE32 history modes retain exact full-prefix/gap/missing/failure/changed outcomes. Native histories test queued ENDED, pending drain, retain/reset/resize, normal no-readback presentation, strict mismatches, bounded refusal, blocked-worker terminal cancellation, cleanup and GUI responsiveness.

`NT14.native-controls-20261022` supports exactly ten implementation completions, reducing registered rendering partials36→26. The other four status dimensions and all historical source hashes remain unchanged. Original manifest passes before/after; no installed game or real driver lease is executed. Exact committed-history review covers `6da5dc7b..4cd9121f`:48 parent/current transitions, one evidence record, zero missing/rewritten receipts. Incoming actor-emitter range `4cd9121f..8bf5e736` adds89 checked transitions across two commits with no receipt gaps/rewrites; owned sources/contracts were preserved on integration and the full batch rerun at that head. SQLite holds generated reports and review bookkeeping; raw controls and verified local backups remain pinned on the second drive.
