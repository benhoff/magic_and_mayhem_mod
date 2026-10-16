# EGL completion and final-source presentation diagnostics

## Current scoped correction

The full original-active corrected session completes222observed EGL frames and
normal Quit. `egl-destination-native-corrected-20261016.json` compares every frame
from source-only native inputs: native GPU, original source texture, pre-swap
destination and physical X11 output all agree, with zero differing pixels/words.
Exactly one upload is cropped, at the first640×480 frame (ordinal160 in this run).
The old238frame reference and its remaining126pixel-per-cohort failure are kept
unchanged; this new record is an explicitly modified-driver policy profile.

The unmodified-upload XCB trace has219frames. At its first640×480 swap, EGL reports
640×480 but the software path submits a full832×640 SHM image, the dimensions used
for the preceding800×600 extent. The following swap submits640×512. Buffers are
padded and SHM segment IDs change; this is stale **image geometry**, not evidence
that the same SHM allocation survived. Source and nominal pre-swap destination
already agree. Rebinding or releasing/restoring the same EGL objects does not fix
the physical shift; those experimental modes remain disabled by default.

`--egl-correct-stale-upload` is an explicit capture-profile policy requiring the
storage case and independent source/destination/XCB witnesses. It is separate
from the rebind experiments. For the observed pinned Wine/llvmpipe path, it checks
the known native window/generation, preceding successful logical/image extents,
full GL source/destination projection, exact full-image SHM packet and source
bounds. It crops only the source rectangle, using the600→480logical change to
select `(0,120,640,480)` in this run. No frame number, constant120, pixel colour or
oracle chooses the correction. Destination, current SHM reference, event and
return cookie stay unchanged; no extra swap is issued.

This is intentional driver policy, not recovered universal image-origin behavior.
Another driver could retain oversized storage while already using the new origin;
that case is outside scope. The policy remains opt-in. Source ownership, complete
SHM lifetimes/routes, hardware and full native rendering replacement are separate.

Actual owned SHM controls reproduce a shifted source in an832×640 image: the
uncorrected upload differs in307196pixels and the cropped upload matches every
pixel. Normal, fresh-dimension shrink, grow and repeated uploads remain unchanged;
six unsupported identity/partial-route cases preserve their arguments. Seven
owned EGL modes across seven resize frames preserve pixels, read state, viewport
and GL error checks; the crop mode makes no correction to the already-correct
fresh images. The wire and original/forwarded packet checks are specified in
[the format contract](../formats/egl-presentation-witnesses.md).

A same-current policy attempt exhausted its128MiB transient acquisition budget;
its incomplete result is preserved. The repeated256MiB bounded acquisition closes
normally and still fails physical parity. The final correction changes no pixel
tolerance, frame denominator or generic14journey/21operation gate.

## Historical source-only diagnostic result

After correcting COPY driver order, native replay matches the original final GL
source texture exactly on all228frames of `cursor-source-native-20261016.json`.
The final policy-disabled230frame acquisition/replay also matches every original
source RGB565 word and retains one first-downsize physical failure. The earlier
228frame run differs by126pixels in each physical-reference comparison cohort. It remains a failing physical comparison. This isolates the discrepancy
after source rendering in the observed Wine/EGL presentation path; the specific
Wine/Mesa internal cause remains unproved.

The full-frame first640×480 presentation in a separate completed233frame capture
is exactly the source shifted down120pixels, with black upper rows and cropped
bottom rows. Native source pixels and the original final texture agree. Additional
observations reject several proposed explanations within these captures:

* A same-context completed GPU fence before the physical read does not fix it.
* EGL surface dimensions are already640×480 before and after the real swap.
* The original final GL blit is source `[0,0,640,480]` to destination
  `[0,480,640,0]`, with a640×480 source attachment and default draw framebuffer.
* The actual associated X11 drawable is viewable at root0,0 and640×480 both
  before and after swap. A displaced child/input-window capture is unsupported.
* GL viewport remains800×600. Temporarily setting it to actual drawable size
  only around the original final blit and restoring it does **not** fix the
  shift: the232frame policy experiment still fails one physical presentation,
  while all232source-GPU comparisons match. This opt-in experiment is retained
  for reproduction, not enabled as a rendering fix.
* An owned EGL resize fixture does not reproduce the shift. Seven full-frame
  source/physical comparisons across both resize directions pass with and without
  the temporary viewport policy, with viewport restoration and GL error checks.

The earlier broad readiness policy that discarded resized partial updates was
rejected because it broke the opposite resize direction. It is absent from the
implementation. No frame/colour-specific compensation, skipped comparisons or
relaxed tolerance is used. The original238frame corpus remains unchanged and
retains its one remaining ordinal170 failure after the nine ordering fixes.

## Updated boundary: destination pixels are correct before swap

`egl-destination-native-baseline-20261016.json` compares all236observed frames.
Native GPU output, the original final source texture and the original default
framebuffer's nominal pre-swap rectangle match exactly in RGB565 on every frame.
Only ordinal170 differs after swap:126physical pixels. Destination readback does
not eliminate it. This narrows the demonstrated boundary to after correct
pre-swap destination content and before the physical witness; the exact internal
swap/window/driver mechanism is still unproved. Nominal EGL dimensions do not
establish the underlying default-framebuffer allocation dimensions.

The new opt-in `--egl-rebind-drawable` policy tests that boundary by rebinding the
same current EGL display, draw/read surfaces and context before the first observed
final blit or a nominal extent change. It verifies identical current objects,
read/draw FBO/buffer state and viewport afterward. It does not translate pixels,
replace sources, skip a frame or issue an extra swap. `MNMERB01` records applied
or explicit no-op state, run-local call/PID/TID, first-context/extent-change reason,
prior/new extents and verified success/state preservation in96bytes. No host
pointer is exported. Owned controls require four applications and three no-op
swaps across the seven frames. This is a driver policy experiment, with no
original error-state neutrality or unrestricted equivalence claim.

## Explicit diagnostic modes

`--egl-swap-completion` requires EGL physical acquisition and sets
`MNM_DRAW_EGL_SWAP_COMPLETION=1` only on the owned original launch. Before the real
swap it records EGL surface dimensions and native X11 geometry. After successful
return it creates a same-context OpenGL sync fence, waits with flush for at most
one second, deletes it, records viewport/scissor/framebuffer state and only then
enters the independent physical handshake. The ordinary worker-return fence runs
after this handshake, so it alone cannot establish this pre-read completion.
Missing procedures/context, timeout/failure, changed context and malformed output
refuse. These queries/fences are intentional diagnostic perturbations; no general
GL/EGL error-state neutrality or hardware scanout claim is made.

`--egl-source-gpu-witness` separately enables
`MNM_DRAW_EGL_GPU_SOURCE_WITNESS=1`. After the observed original GL framebuffer blit
it reads the actual source texture as RGB565 into bounded comparator-only files.
Default pack state, no pack buffer, compatible alignment and extents≤2048 are
required; total readback budget is512MiB. The strict external comparator requires
the exact full-source→flipped-default-framebuffer projection above, correct
extent, immutable SHA-256 and call-bound basename. It compares every RGB565 word
with native GPU output. Missing evidence is not success. Original GPU pixels
never enter native processes or choose source reconstruction, copy order or
composition. Physical comparison remains a separate required outcome.

`--egl-refresh-viewport` separately enables the **known ineffective experimental**
`MNM_DRAW_EGL_REFRESH_VIEWPORT=1` policy. It requires completion mode. Around an
observed final blit to drawFBO0 it saves the viewport, sets actual current EGL
drawable dimensions, forwards original arguments exactly once and restores the
original viewport/errno. Every selected frame must retain an applied/restored
witness. This is intentional native driver policy, not recovered baseline behavior
or an original-equivalence milestone. The normal profile leaves it disabled.

GL blit and native EGL window associations cover observed direct/named/procedure
lookup routes, including platform window aliases, with bounded16function aliases.
Unknown/RTLD_NEXT GL routes stay unobserved. Window generations are run-local
observation identities, not stable addresses or complete creation/lifetime counts.
Private X11 geometry queries do not change the selected peer or establish all
other presentation routes. Source ownership/backing aliases remain independent.

## Sidecar wire contracts

All records use little-endian32-bit fields, an8-byte magic, version1 and exact byte
size. Common identity is original EGL journal call/PID/TID. No host pointer crosses
these files. GL object IDs and XIDs are local diagnostic labels.

| Magic | Bytes | Retained payload |
| --- | ---: | --- |
| `MNMESBF1` | 64 | Pre-real-swap EGL width/height; reserved words zero |
| `MNMESGC1` | 128 | Post-return extent, successful client-wait enum, unchanged-context/fence-created/deleted flags, monotonic begin/end, viewport, scissor/enable, draw/read FBO and buffer enums |
| `MNMEGBT1` | 96 | Observed blit count, signed source/destination rectangles, mask/filter, read/draw FBO, read texture and attachment dimensions/read buffer |
| `MNMEW001` | 96 | Before/after phase, explicit known/unknown association, XID/generation, signed root position, width/height and map state |
| `MNMEVP01` | 64 | Applied-policy count, prior viewport, temporary extent and restoration flag |

`tools/drawing_egl_swap_completion.py` strictly decodes records and binds every
selected physical frame by identity, extent and fence end preceding first capture.
Later returned calls beyond interval closure stay explicit. It checks restoration
against independently queried post-swap viewport. GPU source files are immutable
`egl-gl-source-<call>.565` oracles with exact length/hash. Decoder tests cover
malformed sizes/magic/version, failed/incomplete fences, surface bounds, unknown
window associations and missing/unrestored policy state.

Registered source-native, policy-negative and final unmodified-driver acquisition
records retain exact prospective claims, sources and raw report hashes. Source
changes leave earlier results historical; their hashes are never refreshed.
Broader journeys, native Qt layout policy, original caller results/global routes,
source ownership, hardware/performance and live replacement remain outstanding.


## Destination-before-swap diagnostic

`--egl-destination-gpu-witness` separately enables
`MNM_DRAW_EGL_GPU_DESTINATION_WITNESS=1` and requires completion mode. Immediately
before forwarding the real EGL swap, it verifies the current display/draw surface
and context, queries nominal EGL dimensions and reads that rectangle from the
current default draw buffer as RGB565. It saves the source read framebuffer and
its read-buffer selection, binds the default read framebuffer, saves its own
read-buffer selection, selects the actual draw buffer for readback, then restores
both selections/bindings. It verifies original read/draw state and context before
publication. Unsupported draw routes, pack layout, extent, budget or restoration
refuse. Each run is bounded to512MiB of destination readback.

`MNMEDB01` is a128-byte little-endian version1 record: EGL call/PID/TID, nominal
width/height, default draw FBO0, prior read FBO/buffer, prior default read buffer,
selected draw buffer, exact pixel length, verified-restoration flag, pack state,
monotonic begin/end, unchanged-context and before-real-swap flags, with trailing
reserved words zero. Immutable `egl-destination-before-swap-<call>.565` retains
raw GL lower-left rows; only the external comparator flips them to nominal
image coordinates. The matching `.bin` metadata and `.565` SHA-256, identity,
extent and end preceding the post-return completion witness are checked.

The external comparison reports native→destination, source→destination and
destination→post-swap physical differences independently, with explicit witness
counts. Missing source/destination evidence is not a matching comparison. No
original destination/source pixels enter native input preparation or select
rendering. Nominal EGL extent is not proof of underlying allocation dimensions.
Default-framebuffer binding/readback can itself refresh driver drawable state;
changed physical results must be recorded as perturbation evidence. Error-state
neutrality, source ownership, all routes and hardware remain unproved.

Seven owned known-pattern frames across800×600→640×480→800×600→640×480 run in
control, completion-only, destination-readback and drawable-rebind modes. They compare exact
destination words and physical pixels, source/default read-buffer restoration,
viewport preservation and GL error checks. The default framebuffer deliberately
retains `GL_FRONT` for reading while the source FBO reads `GL_COLOR_ATTACHMENT0`
and drawing targets `GL_BACK`, so restoration is checked independently of merely
returning to the original FBO. Original-game findings remain separately recorded.
