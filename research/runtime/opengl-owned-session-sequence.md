# Bounded successive native primary presentations

2026-10-06. Native policy `NR.owned-session-sequence` extends the opt-in ordered
shadow session beyond the [first-frame sample](opengl-owned-session-startup.md).
The consumer already accepts repeated PRESENTs and keeps native GPU surface
history. This change keeps the producer open for a requested number of frames.

`MNM_RENDER_OWNED_SESSION=1` and `MNM_RENDER_SESSION_PRESENTATIONS=N` request
1 through32 eligible primary presentations. Missing, malformed, oversized or
out-of-range counts preserve the ordinary16-operation sample and64-operation
startup extension. Explicit sequences finish immediately on the requested
PRESENT. First PRESENT must arrive by64 successful operations; the sequence must
complete by256 successful operations. Failed original calls do not consume slots
or presentations. Early producer exit and an operation ceiling before the target
refuse with existing GAP6, even if some native frames have already appeared.
Invalidation, held locks/DCs and resource limits retain their existing refusals.
Completed streams do not reopen when original drawing continues.

This is one bounded append-only sequence, not unbounded transport. The unchanged
64MiB command/channel ceiling remains unchanged; operational
records still stop at4062 with cleanup reserved through4096. Ownership caps,
surface/pixel budgets, native command wire, and original drawing stay unchanged.
No independent original-driver pixel equivalence or live replacement is claimed.
A reusable transport with acknowledgement/backpressure and explicit recovery
will be needed for sustained presentation beyond these bounds.

## Validation protocol

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-sequence.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-session-startup.py working/build/live-render-channel
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-live-render-game.py working/build/live-render-channel --presentations 3
```

The PE32 fixture independently draws changing native pixels and preserves
original return/LastError/call-count checks and poisoned Unlock backing memory.
Three frames,32 frames (beyond64 successful operations), failed original draws,
operation exhaustion after one frame, early exit after two, invalidation after
one, the ordinary sample and an out-of-range setting are exercised. Whole live
GPU frames compare to independently saved original fixture frames; explicit CPU
and GPU replay validates final native storage on complete streams.
Refused streams release native surfaces; ordinary native/RGBA frame readbacks
and viewport image uploads stay zero.

## Synthetic execution

[Eight sequence fixtures](opengl-owned-session-sequence.json) pass: five complete
streams and three deliberate refusals. Forty complete-stream frames and four
frames preceding refusal compare against independent original fixture pixels.
Both native incremental-consumer and live-command CTests pass. All terminal
native surfaces are released, with zero ordinary native/RGBA readbacks and
viewport image uploads. The first budget-fixture attempt exceeded the unchanged
30-second client timeout because of artificial30ms event pacing; its fresh
successful run uses2ms pacing for that fixture only, leaving production behavior
and the client deadline unchanged.

[Eight ordinary startup regression fixtures](opengl-owned-session-startup-sequence-regression.json)
pass on the sequence-capable sources, preserving the ordinary sample and both
startup boundaries. Historical evidence remains unchanged; shared-source edits
leave older transport/fill/DC/unlock evidence stale rather than asserting whole
contract revalidation. Sequence coverage uses opaque RGB copies; longer real
menu/gameplay coverage, repeated palette/Flip/DC sequences and sustained channel
reuse remain separate validation milestones.

## Diagnostic traffic and capacity boundary

The [first real multi-frame attempt](opengl-real-game-shadow-sequence-capacity.json)
published67,069,888 bytes and then refused capacity (GAP2), retaining original
drawing. It contained49 full-surface CHECK records,30 UPDATEs,7 CREATEs,6 copies
and one PRESENT before refusal; the native consumer observed no frame because it
opened after the producer was already failed. Historical source hashes remain
unchanged. This observation does not assert a clean multi-frame native result.

The opt-in multi-frame mode now omits CHECK records, which contain expected
output used only for diagnostics and never renderer input. Ordinary comparison
samples retain CHECKs. Ownership/DC guards still run, and every admitted
CREATE/UPDATE/COPY/PALETTE/SWAP and PRESENT is preserved; expected output is not
substituted for native drawing. Whole-frame synthetic comparisons and explicit
final native-byte replay remain independent checks. There is no per-operation
CHECK comparison in this multi-frame stream. Byte/record exhaustion still refuses
rather than dropping render inputs or recycling the append-only channel.

[Current input-only sequence execution](opengl-owned-session-sequence-inputs.json)
passes the same eight cases and44 independent complete-frame comparisons. Explicit
final CPU/native GPU bytes agree on complete streams; new sequences contain zero
CHECK records, while ordinary and out-of-range sample cases retain CHECKs. This
new evidence preserves the earlier CHECK-carrying execution fingerprints rather
than refreshing them after the implementation changed.

The [fresh ordinary startup rerun](opengl-owned-session-startup-sequence-regression-v2.json)
passes all eight boundaries on the final input-only implementation; default
CHECK emission and diagnostic replay remain active. Earlier regression records
retain their original source hashes.

## Original-game sequence result

[The fresh input-only game rerun](opengl-real-game-shadow-sequence.json) reaches
three successive native primary PRESENTs before producer exit and clean END at
57 successful operations. It publishes39,363,820 bytes/74 records:7 CREATE,
42 UPDATE,14 COPY,3 PRESENT,7 DESTROY and1 END; no CHECK output traffic. Mirror
and mapped channel match. Native surfaces are released, with zero ordinary
native/RGBA readbacks and viewport image uploads. The harness verifies all2,927
immutable original files before and after the experiment.

All three frames share early-startup QRgb SHA-256
`f4e2db9a02fa70eac1e8f0ffa0868ca5dfb158575ff000903d69a6070af382bf`.
This proves successive live native presentations and clean sequence completion;
it does not prove visible animation or full menu/gameplay coverage. Changing
native pixels are verified in the independent synthetic sequences. Original
drawing remains active; original-driver pixel comparison and live replacement
remain pending. Sustained presentation needs reusable transport/acknowledgement
with bounded storage and explicit gap handling, plus longer scenario coverage.
