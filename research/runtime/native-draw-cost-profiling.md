# Native drawing cost experiment

This diagnostic measures submitted draws and instances, texture upload calls and
bytes, program switches and framebuffer attachment changes. These are actual GL
submission counts, distinct from logical native copies. Timestamp pairs remain
bounded and nonblocking; their timeline includes CPU feeding gaps and is not GPU
busy time. Existing statuses/evidence stay historical.

Use the same verified source-only captured World prefix for primary processing
and two exact baseline-restored native redraws per source. Timestamped and
timestamp-disabled modes have separate reports. Checkpoint diagnostics compare
every selected World/redraw outside operation timing and are not a throughput
proof. Original failed full-campaign outcomes remain failed. NVIDIA EGL differs
from public zink/GLX and cannot alone establish public60FPS. New execution,
optimization attribution and current full gameplay validation are pending.

## Baseline execution, 2026-10-12

Fresh source-bound NVIDIA TITAN EGL replay passes128orderedWorlds and256native
redraws. Every selected World/redraw pixel check passes; timestamp-enabled and
disabled final hashes match. Ring overflow executes work without waiting,
exceptions/nesting refuse, and all queries/surfaces retire. Original/source/input
checks pass. Historical input capture's later GAP stays failed.

Warm timestamp-disabled native source processing median7.5194ms and extra redraw
median3.0219ms. Timestamped extra GPU timeline median2.5153ms includes CPU feeding
gaps. Each warm extra redraw submits median75draws/1327instances and189texture
uploads/571232bytes. Colour rectangles create/destroy two native source surfaces
for everydraw, including unchanged prepared redraws. A bounded source-pixel cache
is the measured next experiment. Public merged gameplay's8.5375ms redraw mean
uses a different workload andzinkdriver; these observations do not establish
a causal driver slowdown.

Retained preflights: run-7_1w4_a3 refuses six undeclared compiler inputs;
run-93omo32v's diagnostic mode incorrectly required cumulative readbacks to stay
zero after deliberate hashes. The fresh run uses per-operation readback deltas,
keeps diagnostics separate and does not weaken any rendering/pixel/FPS gate.

## Public backend comparison

`tools/profile-native-draw-costs.py --compare-public --scenario
native-public-context-cost-20261012` runs the same frozen compiled renderer,
assets and ordered captured prefix on NVIDIA EGL and the public xcb/zink
backend, plus Qt xcb with NVIDIA EGL. It creates and cleans up an owned private NVIDIA Xorg, verifies the
actual renderer, and refuses platform/backend mismatches or software fallback.
Every primary native pixel hash must agree across backends; each prepared
redraw must equal its corresponding primary output. Timestamp-enabled and
disabled trials remain separate. The existing 5.4 GB corpus is reused by hash.

This test isolates offscreen context/backend capacity. It excludes the public
worker's sharing topology, command UI composition, immutable shared exports,
Qt widget painting and scanout. A backend difference does not establish their
cost or successful gameplay at 60 FPS. Results and next bottleneck attribution
remain pending until the source-bound experiment completes.

The first comparison preflight retained under `run-7o8cey_u` refused the
public zink context because its GL vendor is Mesa. The corrected check requires
Mesa plus zink/NVIDIA_PROPRIETARY/TITAN identity for that route and NVIDIA
vendor plus TITAN for EGL. It keeps the fallback refusal and original/input
checks. The failed record is retained without a performance claim.

The optional `--compare-xcb-egl` route failed context creation in
`run-s_uyhojr` (Desktop OpenGL 3.3 unavailable). Its EGL/zink128World trials
completed before that failure; the overall run remains failed. Default
`--compare-public` compares the two working routes. Extending the ordered
prefix to2423Worlds measures later captured gameplay as well as startup; it
still does not validate the historical failed journey or live sharing/UI cost.

The extended2423World comparison retained in `run-27qs2xvq` fails its
public checkpoint mode: a prepared redraw changes native pixels. EGL checkpoints
pass; the overall experiment and performance interpretation remain negative.
`--diagnostic-public-only` isolates that checkpoint run without a throughput
or cross-backend claim and retains generation/phase, hashes, differing-word
count and first32pixel differences before refusal. No pixel check is relaxed.
