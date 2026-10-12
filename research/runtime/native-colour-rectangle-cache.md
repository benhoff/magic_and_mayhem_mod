# Native colour rectangle cache

The measured baseline performs189texture uploads per native redraw. Colour
rectangle primitives currently allocate, upload and destroy colour/mask surfaces
on every draw. This experiment retains exact owned RGB565 source pixels in a
64slot LRU atlas,512x512colour plus matching all-one mask. It uses at most two
native surface handles,524288native storage pixels and1MiBCPUowned keys. It
reuses source pixels while rerunning native drawing and destination-dependent
composites; it never caches a World screenshot or advances original simulation.

Existing native copy/composite operations provide clipping, arithmetic and
known-pixel admission. Every slot update flushes pending copies before overwrite;
an upload failure invalidates its key before another admission. Destination/
mode/clip remain draw-time values, not source identity. The feature is opt-in
for generic scenes and enabled by native CanvasWorld; the environment variable
MNM_WORLD_COLOUR_CACHE_BASELINE=1 retains the serial World path for comparisons.

Synthetic independent pixel controls, matched TITAN replay and full current
public gameplay/performance are pending. Original equivalence/full replacement
and historical shared-source validation are separate.

## Headless validation, 2026-10-12

Source-bound independent controls pass7pixel cases of12288words each,143hits,
131uploads/67evictions, zero warm source uploads, copy batching, source mutation,
two refusal/previous-frame checks and zero remaining surfaces. Existing native
prepared-World controls pass40destination-dependent repeats and two source
updates with producer history unchanged. The initial fixture compile warning
is retained; the corrected source was declared before fresh execution.

Paired source-bound NVIDIA TITAN EGL baseline/current replays process the same
128orderedWorlds plus256native redraws, retaining exact final hashes and every
selected World/redraw pixel checkpoint. Warm timestamp-disabled source consume
median8.2663to5.8346ms; extra redraw3.2589to1.4449ms. Timestamped extraGPUtimeline
median2.70344to0.63744ms (includes CPU feeding gaps). Uploadcalls189to12, bytes
571232to508928;75submitted draws perredraw remain unchanged. The remaining
upload bytes largely belong to indexed palette batches, not uncached rectangles.
Original/source/input checks and bounded queries/resource retirement pass.

Full current TITAN public native gameplay/performance is running separately.
This is a matched bounded capacity gain, not public60FPS or original equivalence.
