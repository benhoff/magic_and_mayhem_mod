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

## Full public result after boundary correction, 2026-10-12

The unchanged difficulty1full test passes successful casting/combat/spells,
portraits/movement/native menus/resume and normal original Quit.4612source/native/
import/verified returns balance; source/output high-water2, outstanding0 and
zero ordinary readbacks/pixel replies/fallback. Source/input/original/private
cleanup pass. The19FPS floor passes at19.5966Hz.

60FPS remains failed: nativeWorld48.9985FPS, visiblepaint63.3835/57.3311FPS
(worst34.5769/40FPS). Native source processing mean13.9877ms; native submission
6.8951ms and repeatedWorld7.0775ms, plus UI/command/export costs. The successful
run haszero pending-import/incompleteUI blocked opportunities; attributing its
remaining cost to an originalUI wait is unsupported. Livezink and EGL replay
have different work/context paths and are not a causal driver comparison. A
read-only directNVIDIA_GLX private-display probe times out20seconds; the private
display is stopped. No backend policy is changed from that failed probe.

The preceding corrected run has no renderer error but loses the player wizard
before required combat completion. Its partial49.7090World/59.5056paintFPS and
19.5516Hzsource are retained as a failed gameplay observation, not a full proof.
The final identical-input EGL comparison is being refreshed for the final code.

## Final merged-code same-input replay

Fresh finalcode128World/256redraw TITAN EGL comparison passes every selected
native checkpoint and timestamp/throughput output hash. Baseline extraCPUmedian
3.0856ms becomes1.4300ms (53.7%less); GPUtimeline2.551264becomes0.64088ms.
Uploadcalls189become12;75native draw submissions remain unchanged. Warm source
processing median7.5931becomes5.7350ms. Zero query drops/ordinaryreadbacks/
remaining resources and original/input/source integrity pass. This result
supersedes current-code capacity validation without rewriting earlier hashes.

60FPS still needs public-path cost attribution: the complete successful live
run's World49FPS and repeatedWorld7.08ms include a different driver/context
path from EGL. Pending imports and incompleteUI blocked opportunities arezero
in that successful run. Measure current public render-context/driver/submission
work on identical input before assuming simulation or an originalUI roundtrip
is the limiting work. Native drawing stays enabled throughout.
