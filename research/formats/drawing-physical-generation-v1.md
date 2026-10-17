# Native physical capture generation witness

The presentation comparator draws a native diagnostic strip immediately below
its game viewport. It is outside the compared rectangle; no original output
is consulted to draw it or decide when to capture. The strip is32rows high and
the same width as the viewport. Row `bit` is white for a set bit of
`0x4d4e0000 | ordinal`, otherwise black, for bits0..31 in ascending order.
Ordinals are1..8192. The current ordinal is installed only after successful
painting of that requested native GPU frame.

The host waits for paint and swap, then reads game rectangle plus strip in ONE
X11 screen capture. It retries only on the native marker, bounded by the same
five-second per-frame deadline. The native GPU frame remains held during this
wait. Screen geometry must contain both the complete game frame and strip.
This adds a diagnostic margin requirement; game viewport dimensions and pixel
comparison regions remain unchanged.

The game image is retained as `frame-NNNNNN.rgba`, with its usual SHA-256.
The strip is `frame-NNNNNN-marker.rgba`. Both use top-down RGBA8888. The frame
receipt adds `physical_ordinal`, `capture_height` (viewport height+32),
`marker_path`, `marker_sha256` and `capture_sha256`. The last hashes concatenated
game bytes followed by strip bytes, exactly the original full screen capture.
The parser checks the complete RGB marker pattern and all identities/hashes;
alpha bytes are retained and hashed but not compared as colour.

The presentation report requires `physical_generation_frames` and
`painted_before_swap_frames` to equal its frame count, with
`diagnostic_marker_rows=32`. Missing, stale, relabelled, corrupted, wrong-size or
separately altered game/marker receipts refuse. The comparator still compares
all game pixels against the same fixed original ordinal at RGB tolerance1.

This witnesses native image delivery to the captured X11 composition. It does
not establish scanout, arbitrary occlusion, all-context completion or original
resource ownership. See [frame admission](../runtime/drawing-physical-frame-admission.md).
