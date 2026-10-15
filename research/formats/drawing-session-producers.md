# Bounded session source producers

The opt-in `MNM_CANVAS_SESSION_PRODUCERS=1` policy records source-only drawing
commands until a requested quiescent interval end. It requires original-active trace mode,
owned minimap inputs and the existing 32-sample selector; internally it uses the
explicit v5 envelope with a 120-World-return maximum. Legacy modes and budgets
keep their existing behavior. Whole-update, replacement and publication modes
are separate and incompatible.

The `MNMPRO05` 64-byte header uses little-endian u32 words. Words 2..8 are
version5, header64, build0x40209ca7, maximum1048576records, maximum120queues,
flags2(session), maximum536870912bytes. Words9..15 are zero. The body retains
v3 source-owned payload definitions, four-orientation minimap and colour rectangle
operations. Record sequence numbers and World entry/return identities remain
contiguous. Recorded raster return observations are removed before native input.
Checkpoint records carry identities only; no producer-oracle destination planes
are written in this mode. Source payloads remain distinct from expected pixels.

The ordinary complete-prefix decoder refuses this envelope because a session may
end after UI drawing or cleanup rather than exactly at a World return. Admission
requires `canvas-session-0000.done`, an independent 64-byte `MNMPCS01` marker:
words2..11 are version1, size64, build0x40209ca7, recordcount, completedqueues,
streambytes, checkpointcount, Windowsclosingthread, quiescentclosure1, queuemax120.
Words12..15 are zero. The marker is emitted only with no recorder failures,
active World queue or pending producer frame, and after the stream flush.
The collector requests closure only after both presentation/API journals close.
At the next producer return with no active queue or pending frame, the recorder
flushes, emits the marker and stops recording. The external collector must also
prove actual normal original Quit; a closure marker is not an exit-success claim.

`decode_session` checks exact marker counts against the complete raw body. Native
replay may receive a separately hashed prefix ending at the last declared frame;
unpresented cleanup remains outside that prefix. Both the full captured stream
and its source-prefix derivation remain retained. A budget or closure failure is
an incomplete original result, never a partial session relabeled complete.
