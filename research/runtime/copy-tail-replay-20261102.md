# COPY tail attribution and source-only replay — 2026-11-02

The software/640x480 storage execution completed its commands, viewport and
source history, but its optional COPY observer stopped at 8,192 of 8,805
operations. This batch attributes all 613 missing witnesses before attempting
new acquisition. The old capture and its `success=false`, `truncated=true`
diagnostic remain immutable.

## Exact attribution

`copy-tail-replay-results-20261102/trace01.json` binds each missing command's
sequence and logical handles to a unique original application call interval in
the closed V2 routing census. Its original-module RVAs remain distinct from
wrapper entry addresses. Each missing command belongs to a separate call.

| Original return PC | Recovered wrapper | CPU-prepared | GPU-prepared |
| --- | --- | ---: | ---: |
| `0x58c05d` | whole opaque, entry `0x58bf40` | 43 | 95 |
| `0x58c279` | whole keyed, entry `0x58c140` | 260 | 0 |
| `0x58c488` | rectangle opaque, entry `0x58c360` | 2 | 0 |
| `0x58c5be` | rectangle/BltFast opaque, entry `0x58c4a0` | 56 | 64 |
| `0x58c9af` | rectangle keyed, entry `0x58c8a0` | 93 | 0 |

These wrapper classifications were already recovered in
[route reachability](native-copy-route-reachability.md) and
[GPU-copy validation](native-gpu-copy-live-validation.md). The new evidence
confirms the command/call bindings in this execution, with high confidence
within its finite captured interval. CPU preparation is routing, not proof of
original destination pixels. None of the missing calls uses an attached
clipper or self-alias. Ninety-three use World generations 976–989, while 520
use generations 990–1,041. The final `after-recreate-load` checkpoint records
generation 990. The report assigns a command to the most recent checkpoint
whose generation is less than or equal to its World generation. This convention
puts all generation-990 commands in the later bucket; it cannot establish
whether an individual command occurred before or after the checkpoint itself.
These are generation buckets, not individual menu phase observations.

## Reproducible bounded experiment

The separate CMake project in `tests/copy-tail-replay/` builds canonical command,
World and OpenGL implementations. Only a generated offline observer changes:
its count ceiling is 16,384, its readback ceiling remains 32 GiB, and its
512 MiB/256-job FIFO waits under an explicit 60-second sink timeout. A
900-second process limit bounds the experiment. The production observer,
player deadlines and live queue refusal behavior are unchanged. The link map
must show the offline object and exclude the production observer object.

The executable accepts the retained MNMPROF3 source history, raw V5 commands
and a read-only asset fixture. It has no input for captured COPY destinations,
viewport images, CPU replies or original reference planes. World and facade
rendering have separate surface owners sharing a native OpenGL context group,
as in the player. Expected COPY pixels are computed only after actual native
execution/readback. Every existing captured witness and every complete CPU-read
reply is compared after replay, including full headers. Prefix anchors must
match source, before, actual and expected hashes, arguments and dimensions.

Choose new RAM-backed staging/build paths, set `TMPDIR`, check memory/free
space and use the main checkout's storage guard for each producer:

```bash
cmake -S "$REPO/tests/copy-tail-replay" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --target copy-tail-replay -j4
python3 "$REPO/tools/trace-copy-tail.py" --root "$REPO" \
  --acquisition research/runtime/rendering-normal-results-20261031/storage-worker-software-low09.json \
  --output research/runtime/copy-tail-replay-results-20261102/NEW-trace.json
python3 "$REPO/tools/run-copy-tail-replay.py" --root "$REPO" \
  --build "$BUILD" --stage "$NEW_STAGE" --assets "$ASSET_FIXTURE" \
  --acquisition research/runtime/rendering-normal-results-20261031/storage-worker-software-low09.json \
  --output research/runtime/copy-tail-replay-results-20261102/NEW-replay.json
```

Use absolute arguments because the storage guard runs from the main checkout.
Original-manifest verification surrounds execution. SQLite stores reports,
logs, exact binaries, generated observer/link map, hashes and selected witness
planes. Temporary command/profile files are bounded producer interchange,
already retained by the original acquisition; they are not new durable copies.
Run `python3 tests/test-copy-tail-replay.py` for attribution ambiguity, missing
and duplicate histories, mutated anchors, full CPU-reply identity and observer
preimage refusal controls.

## Retained failed attempts

`replay01.json` reached 408 matching native COPY comparisons, then refused the
64-surface budget because the initial harness combined World and facade owners.
`replay02.json` separated owners but refused the GPU frame because their contexts
did not share. `replay03.json` refused a misspelled asset-fixture path before
launching native code. These are harness failures and earn no completion credit.
Their logs, binary identities and partial outputs are retained independently.

## Acceptance boundary

`replay04.json` passes: 73,615 commands and 1,041 source-only Worlds replay
through normal END and zero surviving surfaces. All 8,805 native COPY
destinations match the independent CPU expectation over 2,151,556,376 pixels.
All 8,192 existing captured witnesses match exactly, as do all 61 full CPU-read
replies. The extra 613 comparisons cover 159,181,280 destination pixels at
command sequences 60,937–73,609 and World generations 976–1,041. There are zero
COPY mismatches or diagnostic truncation. Diagnostic readbacks total
21,392,213,760 bytes, below the unchanged 32 GiB ceiling. The FIFO peak is
536,845,248 bytes, below its 512 MiB bound.

`joined01.json` binds the replay, attribution, 14 refusal/identity controls and
negative attempts to 136 current source files and the narrow headless scenario
`SC.copy-tail-source-replay`. `CT01.source-replay01` records this separate native
policy. Failed attempts have exact source preimages, preserving their recorded
hashes rather than refreshing them to match the successful harness.

Offline native execution can establish deterministic source-history reproduction
and complete independent destination comparisons. It cannot retroactively
witness missing pixels from the old process. It also cannot establish original
COPY equivalence, original-driver policy, whole-operation acceptance, broader
branch reachability, ABI/font/DC coverage or a final player/recovery verdict.
Those remain separate obligations in the EP06 frontier and closure register.

The next COPY task is fresh bounded live observation of the needed original
call/output branches, using the exact caller/World/command list to select work.
Replay has removed the need for another run merely to test these native COPY
destinations. A new acquisition must record its own identity, diagnostic budget
and refusal outcomes; it cannot make the old process complete. Storage admission
remains 8/156 finite requirements and EP06 remains 0/36 whole operations.

## Independent histories agree

The complementary [isolated fixture proof](copy-tail-proof-20261102.md) uses
independent command history and retained original oracle Worlds. This harness
uses source-only native Worlds. After both executions, a separate comparison
checks all 613 tail sequences: source, pre-copy destination, expected and actual
output hashes agree in all 2,452 comparisons. Four deliberate hash mutations
are rejected. The [cross-check](copy-tail-replay-results-20261102/cross-history01.json)
binds 143 current source files and both immutable results. The original oracle
planes remain outside this native harness's input path.

The fixture proof groups generation 990 with the earlier checkpoint bracket;
the attribution here groups it with the later bucket. Seven COPYs share that
boundary generation, explaining 100/513 versus 93/520. Both report the same exact
command/World identities; neither convention establishes individual action time.
The missing historical outputs and all whole-operation/final verdicts remain
pending. Incoming committed file versions receive an exact receipt check; a
binary edge outside the JSON census gains a retrospective custody receipt,
without claiming a past gate pass or new engine validation.
