# EGL completion and final-source presentation diagnostics

## Confirmed result

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
