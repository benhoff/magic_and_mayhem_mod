# Campaign cadence and diagnostic overhead

Current merged-source acceptance passes the unchanged 5 FPS whole-journey gate
for the matched GUI pair and dedicated World owner. The source-bound
[full report](native-campaign-cadence-bound-20261017.json) and
[provenance](native-campaign-cadence-provenance-20261017.json) retain the exact
sources, build, mode, original callbacks, independent pixel checks and all failed
attempts. These are bounded software results; sustained hardware/60 FPS and
whole-session original pixel equivalence remain open.

## Diagnosis

The unchanged `96d3cb46` pair passes at 15.9219 World FPS with copy diagnostics
off and 5.0024 with them. It uses the same shell hash and common source hashes,
30-second pre/post Mini Cancel workload, resident profiling/source capture,
supported installation, software driver and four renderer threads. Earlier
4.9904/4.7642/4.7611 FPS failures remain failures. Diagnostic-on barely crossing
the threshold does not invalidate those negatives or establish a robust pass.

Per-World observations locate the main difference inside the producer's wait
for native acknowledgement. Mean intervals and counts are workload observations,
not independent simulation CPU time. The observer serializes/hashes complete
source, before, actual and expected images after already comparing every output
word. New stage timings confirm encoding/hashing is its largest host cost:
45.590 seconds, versus
6.444 seconds in explicit readbacks,
3.795 in reference construction/comparison and
0.283 in record/witness writes for the selected GUI
run. These are host elapsed spans including driver waits, not GPU busy time.

## Repair and measurement

The observer still computes the expected destination independently, only after
GPU execution and output capture, and compares **every word**. After exact
word equality is established it shares the canonical actual/expected encoding
and digest. Mismatches retain separate encodings and hashes. On little-endian
hosts canonical encoding borrows immutable owned pixel storage through synchronous
hash/file writes; the previous conversion loop remains for big-endian hosts.
All four digest fields, three readbacks per checked copy, all destination checks,
16 retained Python-recomputed cases, capacity guards and wrong-output/overwrite
refusals remain. Default rendering has no copy observer. No game state, producer
pacing, simulation or native copy operation changes.

Stress rate samples now expose `world_imports_begin`, `world_imports_end` and
`world_imports` separately from command frames and paints. Helpers publish a
Linux monotonic start and bounded readiness intervals. `campaign_cadence.py`
checks clock identity, interval order/extent and overhead sum, contiguous time
windows, counter continuity and sufficient active duration. It selects only
complete windows inside physical stress, excluding explicit readiness intervals
and helper boundaries. Long stalls stay in its denominator. Other modal time
remains unclassified. World imports do not imply advancing simulation.

Whole-journey `engine_fps`, its warmup, original timestamps, between-phase time
and 5 FPS assertion are unchanged. The new phase view cannot turn a failed
whole-journey gate into a pass. Synthetic controls reject cached-paint-only
progress, wrong clocks, NaN/bool values, gapped counters/windows, forged overhead,
outside/overlapping intervals and insufficient active sampling. An extended
active stall must lower the measured rate rather than disappear.

## Current merged-source acceptance

| Run | World owner | Whole-journey World FPS | Active World imports FPS before / after Mini | Checked destinations |
| --- | --- | ---: | ---: | ---: |
| merged-off-09 | gui-resident | 15.0681 | 14.77 / 15.56 | off |
| merged-on-10 | gui-resident | 6.9920 | 7.83 / 6.68 | 6010 |
| merged-worker-11 | dedicated-worker | 6.3246 | 7.29 / 5.88 | 5755 |

All three full runs pass camera0/1/2 coverage over at least four fresh settled
turns per phase, at least 30 seconds of active input, original Mini Cancel,
Quit No/resume/Yes/report Continue/Main Quit, balanced World retirement, zero
ordinary World/native/RGBA readbacks, zero producer forwards/history refusal,
unchanged declared sources, private Wine stop/wait and all 2,927 original-file
bookends. Diagnostic-on cases additionally close caller censuses and compare
complete GPU destinations with zero mismatches plus independent retained Python
recomputation. Clipped/self/keyed GPU-source live branches and independent
CPU-keyed output retain their existing boundaries.

Forty-nine Python test methods and native GL controls pass. Native controls
verify canonical retained hashes after source buffers retire, preserved failure
hashes for deliberately omitted GPU work, zero non-World readbacks and historical
output refusal. No performance threshold or camera requirement was relaxed.

## Preserved boundaries and failed attempts

Before the upstream merge, the repaired GUI pair passes at 15.6162 FPS off and
7.1038 on; the dedicated worker passes at 6.5344. These remain pre-merge evidence.
The first repaired off run encounters multiple separate introductory Hermes
cards, exhausts the unchanged eight-turn camera budget and refuses coverage.
It remains a failed acquisition; arbitrary intro ordering/readiness is not
closed by later passes.

Host `/tmp` exhaustion prevents Xvfb keymap creation in initial native-control
setup. A child-only mount namespace supplies a private `/tmp` on the second
drive, with normal device access. No host files are deleted. The native control
then passes with a verified owned X server and cleanup. A source-sync assertion
is followed accidentally by a shell starting an acquisition; its owned processes
and private Wine prefix are stopped, immutable originals verify, and it receives
no validation credit. All setup logs and cleanup records remain in provenance.

Master's concurrent DRAW-02 merge changes producer sources although the host
renderer and these five contracts are unchanged. The final trio reruns the
merged code; earlier positives are not substituted. Exact history reviews cover
`9a24c154..96d3cb46` (zero missing receipts) and `96d3cb46..6cada041` (65 commits,
68 parent transitions). The latter initially finds 408 missing intermediate
hash receipts in three versions; three append-only retrospective reviews account
for them. The original report remains unchanged. Retrospective accounting does
not establish past gate passes, new engine validation or storage permission.
