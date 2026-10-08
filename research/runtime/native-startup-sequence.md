# Native startup World sequence comparison

This experiment replays every queue in a bounded original Quick Battle startup
prefix as one native RGB565 canvas, then compares every completed canvas. Native
storage starts with word zero, binds the captured dimensions/clip, and retains
only completed native pixels between queues. The original menu/loading canvas
and between-queue HUD writes remain separately measured producer gaps. Original
destination pixels never initialize, correct or reset the native renderer.

## Newly recovered startup wave routes

Pinned No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The first consumer jump at `0x5003fc` indexes `0x501184`. Its actual table
entries and branch calls establish:

| Queue kind | Table entry | Branch | Primitive argument |
| --- | --- | --- | --- |
| 16 (`0x10`) | `0x5011c4` → `0x5006fb` | call at `0x500707` | `0x5806f0`, amplitude 2 |
| 17 (`0x11`) | `0x5011c8` → `0x500711` | call at `0x50071d` | `0x5806f0`, amplitude 3 |
| 20 (`0x14`) | `0x5011d4` → `0x500753` | call at `0x50075f` | `0x5806f0`, amplitude 6 |

These use the native renderer's existing displacement operation. Earlier
observer refusals were conservative admission gaps, not absent original draw
support. Other table values/default branches remain incomplete.

At `0x580718`, a sentinel of `0x7d00` at `0x5f14d0` triggers the original lazy
table initializer. The loop at `0x58072d..0x58076a` calculates 16 entries for
each amplitude 0..15 with `fsin` and integer truncation helper `0x59bee0`.
Nonclipped row selection at `0x58091e..0x580944` combines absolute row and phase,
takes modulo twice the amplitude and indexes the amplitude's 16-entry table.
The observer now defers ownership of this first request's offsets until the
original primitive returns and the original table has initialized. It neither
calls nor predicts the initializer inside the entry observation callback.

Admission of kinds 16/17/20 and deferred lazy-wave ownership requires finite
`--startup-replay`. Default capability refusal behavior is retained. The added
return trampoline preserves registers, EFLAGS, x87/SSE and LastError. Its active
synthetic fixture validates copying phase-rotated offsets and return state.
Concurrent/reentrant wave callbacks remain outside this scope.

## Comparison boundaries

For every completed native queue the driver compares:

1. Native retained history against an independent unmodified original raster
   chain with the same zero initial state. This tests the recovered request
   operations and native retention without importing live destination pixels.
2. Native completed pixels against the actual original post-consumer canvas.
3. A separate original replay initialized with its diagnostic before-canvas
   against actual post-consumer pixels. This tests completeness of the captured
   queue independently of unknown prior producers.
4. Native prior state against actual original before-state, and prior original
   completion against the next original entry. These retain initial/menu and
   between-queue producer differences instead of hiding them with resets.

Every comparison records mismatch counts, bounds, sample coordinates/words and
SHA-256 hashes. Native input directories contain only closed request records
and a pinned timeline; original before/after pixel files are separate reference
oracles. A raw queue gap, changing canvas identity/extent, incomplete owned
request or changed input hash refuses the experiment. Negative live comparisons
are completed measurements, not whole-startup equivalence claims.

```sh
xvfb-run -a -s '-screen 0 1600x1024x24' \
  python3 tools/capture-scene-game.py --canvas-startup --world-frames \
  --world-lifetime --startup-replay --startup-queues 16 --samples 16
xvfb-run -a -s '-screen 0 1600x1024x24' \
  python3 tools/test-native-startup-sequence.py --capture-report CAPTURE_REPORT
```

`RS.world-startup-wave` tracks original route and lazy-initializer observation.
`NR.world-startup-sequence` tracks this explicit native initialization/history
policy and comparison workflow. Original menu/HUD producer reconstruction, live
history delivery, whole-scene visual equivalence and bypass remain pending.

## Palette capture correction

The initial replay exposed 4,138 queue1 pixels that differed even when the
private original replay started from the actual before-canvas. Kind33 first
selects explicit black `0x595677` for shade -127. That fast primitive refuses
horizontal clipping; the consumer then calls slow `0x57de00` with the same
shade. Unlike the explicit black producer, the fallback selects the real
shade/shift/neutral palette (`0x57de28..0x57de5f`). Half blend `0x57ec90`
also selects a real table (`0x57ecd1..0x57ed0e`). The previous global
shade-minus127-to-black assumption lost near-black nonzero words such as 33
and 2113. Capture now zeros only explicit black tags 0/10. Synthetic actual
observer callbacks check real palettes at shade -127 for tags 1..6 and zero
palettes for tags 0/10. `RS.world-startup-palette` owns this narrow branch.
The first negative run remains historical evidence and is not overwritten.

The queue decoder reports actual unsupported kinds for the effective mode and
retains `default_policy_unsupported_kind_counts` separately. In finite startup
replay,16/17/20 are admitted; their default-policy refusal remains documented.
The capture harness stops the original process/Wine server before reading and
hashing lifetime diagnostics, preventing a post-report append race. A prior
comparison refused that changed trace (`run-gnjau62i`), as intended.

## Completed comparison (2026-10-08)

Final capture `run-c08xurpo` retains every first 16 queue and all 22,742 raw
rows. Original draw kinds 20 in queues 3/4,17 in 5/6 and16 in 7/8 are admitted
by finite startup-replay; default-policy refusal counts remain 2 each. Final
native experiment `run-sitf324r` owns 18,893 effective raster requests on one
800×600 canvas, starts at word zero and retains only its own completed pixels.
No captured original destination pixel initializes or repairs the native canvas.

| Completed queue | Effective draws | Native vs same-zero original | Native vs actual original | Original from actual before vs actual after |
| --- | ---: | ---: | ---: | ---: |
| 1 | 1178 | 0 | 0 | 0 |
| 2 | 1181 | 0 | 0 | 0 |
| 3 | 1181 | 0 | 0 | 0 |
| 4 | 1181 | 0 | 0 | 0 |
| 5 | 1181 | 0 | 0 | 0 |
| 6 | 1181 | 0 | 0 | 0 |
| 7 | 1181 | 0 | 0 | 0 |
| 8 | 1181 | 0 | 0 | 0 |
| 9 | 1181 | 0 | 0 | 0 |
| 10 | 1181 | 0 | 0 | 0 |
| 11 | 1181 | 0 | 0 | 0 |
| 12 | 1181 | 0 | 0 | 0 |
| 13 | 1181 | 0 | 0 | 0 |
| 14 | 1181 | 0 | 0 | 0 |
| 15 | 1181 | 0 | 0 | 0 |
| 16 | 1181 | 0 | 0 | 0 |

All 16 completed canvases match both original references across 7,680,000 pixels.
The captured original entry-canvas still differs from native zero at 365,338 pixels;
later entry states differ by 27,473..27,537 pixels from intervening HUD writes.
Those producers were not reconstructed. Their pixels leave no mismatch in these
completed World canvases; that selected outcome does not prove arbitrary maps,
other startup stages, all menu/loading writers or live native history delivery.
The first historical capture's 41 retained-pixel differences and incomplete
shade-minus127 capture remain negative evidence, not overwritten by this run.

Immutable reports: `native-startup-sequence-live-20261008.json`,
`native-startup-sequence-comparison-20261008.json`,
`native-startup-sequence-negative-20261008.json` and
`native-startup-sequence-queue-policy-20261008.json`. Detailed raw/native outputs,
per-canvas hashes and frozen source/build artifacts are retained under
`working/tests/native-startup-sequence/run-sitf324r/`. Active return-state and
palette-capture fixture and both native World/history CTests pass.

The history poison/reset regression now fails actual resource IO after admission
by temporarily removing its synthetic SPR file, instead of assuming an occupied
GPU surface budget cannot be relieved by cache eviction. Poisoned continuation,
explicit-reset recovery and cleanup assertions remain exercised.

Committed-history review: `coverage/committed-history-native-startup-sequence-20261008.json`
checks 8e43ed5..63f32b1 against exact receipts with no unresolved gaps. It excludes
concurrent uncommitted changes and asserts no retrospective validation.
