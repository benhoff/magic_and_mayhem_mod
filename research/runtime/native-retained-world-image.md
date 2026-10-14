# Retained completed native World image

Experimental native policy, selected by `MNM_WORLD_IMAGE_REUSE=1` in the
independent resident path (`test-native-campaign.py --reuse-world-image`).
The default retains prepared draw replay for comparison and drawing stress.

`CanvasWorld::end` freezes the successfully completed World into one renderer-owned
native surface before subsequent UI writes. Extra presentations restore that
image, then use the existing native sealed UI composition and immutable bounded
cross-thread export. Both the capture and restore are GPU copies, including
native word validity, palette and clipper metadata; there is no original image
seed or ordinary readback. The slot is refreshed once per accepted source
generation and cannot be presented during an active/incomplete World. Failures
remain terminal in the rolling session. No simulation, ANI, RNG, original
return, source FIFO, import identity, two-credit or GUI lease rule changes.

The comparison path freezes the pre-drawing baseline and replays owned native
draws. Destination-dependent half composites, shadows and displacement must not
accumulate across repeated presentations. The retained path instead restores
the already completed result, avoiding repeated draw traversal, sprite cache
lookups, destination snapshots and submissions. It keeps the full native drawing
pass for every new source. Repeated images are not distinct simulation updates
or interpolation, and native presentation counts do not establish scanout.

Validation is pending: authored overlapping composites with interposed UI writes,
new generation replacement, incomplete/invalid-mode refusal, captured public
complete owned source prefix full-frame comparison with prepared replay, native worker/UI
ownership controls and TITAN public gameplay/performance. Captured-input timing
includes a final diagnostic synchronization and is distinct from live FPS.
This policy does not recover arbitrary original driver or destination allocation
behavior. Historical evidence for changed shared sources remains historical.

## TITAN controls, 2026-10-13

`run-6yocwcgb` passes80 authored repeats over two generations, eight captured
full800x600 comparisons against prepared draw replay, invalid-mode/incomplete
refusals and zero ordinary readbacks/surface leaks. The complete initial owned
source prefix is pinned by source/request hashes from `run-d5jy7cxs`, with its
installed asset manifest checked before execution. A later generation2354 ring
slot (`run-k4uirl9j`) correctly refuses absent prior native history and is retained
separately; it cannot initialize a fresh session. The earlier `run-8bdd1yt7`
assertion incorrectly counted the required native presentation resolve as sprite
drawing; corrected controls require exactly that one resolve, one native restore
and no sprite batch/effect snapshot. Both initial failures remain recorded.

Paired200-repeat timings on NVIDIA TITAN RTX/Zink, after20 warmup repetitions
each and including a final diagnostic synchronization, are2.24324ms prepared
replay and0.178185ms retained image (92.06% lower). This is one captured input,
not a live-FPS or GPU-busy claim. Worker controls independently pass172 native
presentations,25 full-frame pixel checks, two immutable export leases, inherited
UI generation refusal, movie pause and joined teardown; GPU presentation controls
pass. Public gameplay throughput and default adoption remain pending.

## First public run: failure retained

`run-l57e71fc`/experiment `run-09bsid6w` balances3837 accepted/imported/verified
returned Worlds and3837 retained-image captures, with zero ordinary readbacks.
It reaches the first campaign menu then refuses drawing-history GAP reason2.
The unchanged50ms mutation budget observes65ms of BltFast ownership on thread
0x1cc at bridge site257, phase4 (`game_blit_after`), while thread0x190 waits
for Lock at site295. Native statusstage0 is sampled, unlike the earlier status
publication failure. This identifies a real missed-mutation boundary; it does
not locate the phase4 suboperation or prove an OS/driver/policy cause. No guard
is weakened and the public result receives no current live validation credit.

Partial engine18.96428FPS/native49.93316FPS do not improve the target. Mean
extra World work is0.61283ms and UI0.95815ms, but new-source native work remains
12.87587ms (producer3.87386/prepare2.69864/submit3.90228ms, plus other costs).
The host trace counts9936 visible native frames over198.698s (50.01054FPS),
with48.344s World,24.858s commands,9.658s extra World/UI,1.763s export,
3.144s status and110.666s wait. Cheap repeats alone have not proved a live
throughput gain; source/UI eligibility and owner deadline scheduling remain
separate candidates. The trace does not attribute original simulation CPU or
GPU busy time. Paired current baseline and complete successful reuse are pending.

## Complete paired public journeys

Prepared replay `run-5r0ezlxe` and retained image `run-v71bzxl6` both pass the
unchanged 19 FPS campaign gate on TITAN: casting, combat, movement, spell cases,
portrait checks, three menu/resume cycles, Quit-No resume and normal Quit. Both
have stable sources, identical shell binaries, matching common source hashes,
the same accelerated renderer, zero ordinary delivery readbacks and private Wine
cleanup. They use the same scripted objectives but different live game states;
observed FPS differences are not a controlled causal result.

| Metric | Prepared replay | Retained image |
| --- | ---: | ---: |
| Balanced source/import/verified-return Worlds | 5,353 | 5,230 |
| Retained World image captures | 0 | 5,230 |
| Extra World CPU wall time | 4.57529 ms | 0.59140 ms |
| Engine FPS | 19.45945 | 19.33535 |
| World-specific native FPS | 49.97270 | 50.80452 |
| Gameplay native presentation FPS | 62.64872 | 65.06677 |
| Gameplay GUI paint FPS | 62.64872 | 65.07059 |
| Worst gameplay presentation/paint window | 39.12648 FPS | 40.90909 FPS |
| New-source native processing | 12.80773 ms | 12.71408 ms |

Extra World work is 87.07% lower in these live journeys, while observed World
throughput rises 1.66% and mean GUI paint rate 3.87%. The identical captured-input
200-repeat comparison remains the stronger causal cost measurement (92.06%
lower). World rates count source plus repeat work separately from UI-only
presentations; neither averages above 60 nor Qt paints establish physical
scanout or sustained 60 FPS. The retained run has no copy-order timeout/GAP in
its lifecycle log, but the first failed attempt remains a reliability limitation.

The new policy now has current bounded live-observation support, independently
of original comparison and replacement (both none). It remains opt-in, keeping
prepared native drawing available by default for rendering stress. The next
render-side target is the roughly 13 ms new-source work and its scheduling on
the presentation owner. Exact original simulation CPU, phase4 stall attribution,
broad auxiliary input coverage, interpolation and sustained 60 FPS are pending.
The paired results, raw report hashes and rate formulae are retained in
`native-retained-world-paired-public-20261013.json`.
