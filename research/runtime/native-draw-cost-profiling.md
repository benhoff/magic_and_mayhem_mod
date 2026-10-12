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
