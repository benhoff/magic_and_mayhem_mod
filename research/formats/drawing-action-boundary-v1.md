# Original action boundary and EGL action witnesses

Opt-in `MNM_DRAW_ACTION_BOUNDARIES=1` enables reference-only observation in the
supported No-CD drawing observer. Neither file below is a native renderer input.
All integers are little-endian u32; callback pointer values are never wire IDs.

`drawing-action-state.bin` is exactly64bytes: ASCII `MNMDACT1`, version1,
size64, then twelve words: epoch, request, action, screen, resource generation,
argument, callback phase, callback status, API entry, primary-plane count,
last completed request, last completed epoch. Zero is the initial state. Callback
entry/return phases allocate monotonically increasing epochs under the journal
critical section. The file is replaced atomically with MoveFileExA
REPLACE_EXISTING|WRITE_THROUGH. This is an observer sidecar, not an engine state
checkpoint or proof of arbitrary memory atomicity. The original API journal
independently retains paired callbacks; offline binding derives allowed states
from that journal and rejects differing sidecars.

`egl-physical-actions-NNNNNN.bin` is exactly128bytes: ASCII `MNMDAF01`, version1,
size128, presentation ordinal, EGL journal call ID, twelve-word state before the
original swap, twelve-word state after it, and two zero reserved words. Ordinals
are contiguous. Both states must match journal-derived epochs and never reverse.
Changing/in-flight action epochs are retained as ambiguous. A stable completed
epoch associates the observed frame with a callback interval, without proving
that callback caused its source pixels. Missing, extra, reordered or modified
witnesses refuse portable admission.

Storage observation uses128byte `MNMEGL02` requests, version2. Existing request
fields retain their V1 offsets; words14/15 are actual EGL surface width/height
from eglQuerySurface, each1..2048. Physical capture requires matching dimensions,
target placement stable before/after the grab, and two equal full RGBA grabs.
Version1 retains its fixed800×600/single-worker contract. Version2 refuses
overlapping handoffs but allows sequential workers after display recreation;
offline storage admission also requires a fresh successful completed Preferences
OK for each worker change. Neither profile establishes all global presentations,
real hardware scanout, unobserved failure paths or GL error-state neutrality.

See [the evidence and limits](../runtime/action-bound-storage-session.md) and
`runtime/drawing/action_boundary.h`, `runtime/display/egl_physical.h`,
`tools/drawing_action_session.py` for exact encoding and refusal rules.
