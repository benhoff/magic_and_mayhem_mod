# Native additive particle spikes, 2026-11-13

The retained [combat replay](deterministic-render-replay-20261113.md) identifies
the late-frame spikes without another original-game run. Existing opt-in owner
timings place about 80–102 ms in scene drawing, versus roughly 1 ms in resource
binding/preparation. Slow frames add 648–803 operation-8/backend-13 additive SPR
draws; nearby fast frames have none. The old renderer freezes destination pixels
and submits a separate composite for every particle. This is native rendering
work, independent of original AI or simulation.

The native SceneRenderer now aggregates consecutive indexed additive sprites'
owned source contributions. RGB565 addition saturates each nonnegative 5/6/5-bit
channel, so `min(limit, d + a + b)` equals the ordered serial saturation. Each
batch preserves source masks, opaque index zero, selected palettes, signed frame
origins and per-draw clipping. One destination snapshot/composite applies the
combined source; no destination pixels are read to the CPU.

Bounds are 32 sprites, a 128×128 union, 65,536 visited source pixels and the
caller's draw/upload/time budgets. Time is checked between admitted sprites; the
bounded batch executes atomically. Two reusable 128×128 source/mask GPU surfaces
add 32,768 storage pixels (128 KiB of native planes), plus a temporary CPU
accumulator of at most 64 KiB and at most 32 source descriptors. Initial mask
upload and each source patch consume the caller's upload budget. Cache
retirement and scene destruction release both surfaces. ResourceManager's
thread-confined immutable loaded references remain valid until explicit unload;
no unload occurs while a batch holds them.

Hidden draws, other modes, missing selected palettes, oversized unions/work,
singletons and exhausted budgets retain the serial path or form barriers.
Isolated device-recovery rendering stays serial. Generic scenes retain their
default; native CanvasWorld opts in. Set
`MNM_WORLD_ADDITIVE_BATCH_BASELINE=1` for a diagnostic serial comparison.
No original engine, AI, animation, clock, balance, wire, caller guard or source
generation policy changes.

Same-case Apple M1 hardware comparison, frames 700–820, three fresh repetitions
per executable (363 samples each), no simultaneous game/compiler workload:

| Native completion | Old executable | Batched executable |
| --- | ---: | ---: |
| Median | 11.624 ms | 11.875 ms |
| Mean | 19.418 ms | 14.187 ms |
| p95 | 78.918 ms | 26.665 ms |
| Maximum | 109.377 ms | 33.001 ms |
| Samples above 50 ms | 30 | 0 |

p95 decreases 66.2%; maximum decreases 69.8%. Median increases by 0.251 ms;
the improvement is in the tail cost. Frame 817's
draw calls fall from 943 to 199, frame 804 from 785 to 188, and frame 820 from
891 to 214. Both executables use exactly the same immutable case and asset
hashes. Benchmarks use one final diagnostic read per repetition; ordinary World
readbacks remain zero. Source/request decoding, handoff, I/O and diagnostic
reads/hashes are outside these serial CPU+GPU completion timings.

Validation is separate from performance:

- All 868 captured frames match the old renderer's pixel hashes across two
  fresh full replays. This proves native regression equivalence for this case,
  not original-game pixel equivalence.
- Sixteen focused serial/batched cases pass against an independent sequential
  scalar compositor on llvmpipe and M1. They cover overlap/saturation, palette
  changes, opaque index zero, holes, signed origins, clipping/offscreen input,
  copy/hidden barriers, oversized/direct-word fallback, one-draw/8 KiB caller
  budgets, retained canvases, cache retirement and surface lifetime. The dense
  control saves 93 drawing calls.
- The final 12-test replay suite passes on llvmpipe and M1, including data
  corruption refusal, earlier history, selected ranges, viewer rewind,
  deliberately damaged baseline and opt-in timing/readback counters.

The initial full-replay frame still costs about 160–181 ms in diagnostic
verification, including cold native setup. That is outside the measured combat
window and is not optimized here. Verification timings were not used as an
additional performance experiment. This does not establish live FPS, full
Qt/HUD/audio/scanout behavior, long-session acceptance or every-map throughput.
The original acquisition's World-869 guarded caller `0x54a563` failure remains
unchanged and outside the imported complete prefix.

Evidence `NATIVE13.additive-batching01` binds the scoped native policies and
current replay controls. Exact baseline/candidate ELF bytes, unique sources,
stage diagnostics, negative build results, scalar controls, per-frame timings
and hashes are retained in `working/native-replay-spikes-20261113/*.sqlite3`
with verified local backups before finish. Original capture/assets remain shared
read-only. Compiler intermediates and exported tapes are reconstructible
temporary output; earlier evidence keeps its original hashes and applicability
limitations.
