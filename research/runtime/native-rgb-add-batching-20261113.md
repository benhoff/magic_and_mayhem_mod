# Native RGB-add rectangle batching, 2026-11-13

The user reported a slowdown when the wizard died or exploded, with many
creatures nearby. That fight had ended. The next recording completed 1,107
Worlds and normal original Quit; the user reported **no observed slowdown**.
This is negative manual reproduction evidence, not confirmation of the earlier
fight's cause. Its retained source stream nevertheless contains a useful native
effect burst: World 1019 has 602 RGB-add rectangles, all 2×2, and no indexed
additive sprite particles. The existing sprite batching does not cover these
rectangles.

The warm owner trace measures a 74.113 ms World turn at generation 1019, including
71.657 ms of native submission. This is native wall time, not original AI CPU
time. The general GUI-latency analyzer refuses this trace because several
UI-only export generations differ from the owner's current source generation.
That refusal is retained; these findings use raw owner phase spans and offline
native replay, and make no accepted GUI latency claim.

## Native policy and exactness

Consecutive visible, clipped RGB565 additive rectangles now share the existing
bounded owned source accumulator and GPU source/mask surfaces. Combining their
nonnegative, saturating channel contributions replaces multiple destination
snapshots and composites with one. Nonrectangle operations, hidden draws,
isolated recovery, oversized unions and exhausted caller budgets preserve
ordering barriers and the existing serial path. Generic scenes opt out; native
World enables this policy independently of indexed additive sprite batching.

The recovered rectangle kernel wraps the channel addition to 16 bits **before**
clamping. Combining arbitrary increments would change pixels. Batching therefore
admits only red/blue increments ≤65,504 and green increments ≤65,472: adding any
possible destination channel cannot wrap in that domain. Larger increments stay
serial. This is a native optimization of the existing recovered pixel rules,
not a new original behavior or simulation policy.

Each group retains the 128×128 union and 65,536 visited-pixel bounds, at most
512 rectangles in synchronous native World, and the caller's draw, upload and
time budgets. The accumulator is at most 64 KiB; two reusable 128×128 GPU planes
are shared with sprite batching. No destination CPU readback is added.
`MNM_WORLD_ADDITIVE_RECT_BASELINE=1` selects the prior serial rectangle path for
diagnostics. Opt-in owner profiling separately counts rectangles and sprites;
successful batch phase timings exclude fallback work and final GPU drain.

## Validation scope

On Apple M1, unprofiled replays of the identical Worlds 990–1040, three fresh
repeats per executable and 153 samples each, give:

| Native completion | Previous renderer | Rectangle batching |
| --- | ---: | ---: |
| Median | 42.003 ms | 14.177 ms |
| Mean | 45.761 ms | 13.463 ms |
| p95 | 76.492 ms | 19.028 ms |
| Maximum | 88.308 ms | 21.805 ms |
| Samples above 50 ms | 68 | 0 |

All 1,107 captured World frames match the exact previous native renderer across
two fresh replays. The complete recording includes cold initialization, whose
largest completion remains above 50 ms; the burst table does not claim to remove
all startup or whole-session stalls. The prior 4× indexed-particle stress case
also preserves every selected frame's pixels across two fresh replays. A paired
237-sample comparison gives p95 34.225→29.734 ms and maximum 42.293→41.117 ms,
with no samples above 50 ms in either case.

Results and exact source/binary/capture retention are recorded in the associated
SQLite-backed evidence. Native replay compares the same source input and
independent native canvas history against the exact pre-change renderer;
unprofiled completion timing drains the GPU and excludes decode, I/O, handoff,
readback and hashing. It is not live FPS, GPU busy time or scanout.

The focused controls include clipping, offscreen extreme coordinates, overlap,
saturation, copy/sprite/hidden barriers, retained history, scratch retirement,
union/visited/upload/draw limits, invalid caps, and every RGB565 destination WORD
at the safe/wrapping increment boundaries. Existing sprite controls remain.

Original AI, combat, simulation, clocks, balance, hook protocols and guards are
unchanged. Original pixel equivalence, new-code live crowded-death acceptance,
Qt/HUD/audio latency and universal fight performance remain outside this scope.
The earlier crowded fight's causal attribution remains unresolved.
