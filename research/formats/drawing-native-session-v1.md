# Native drawing session declaration v1

This is an intentional offline native orchestration policy, not a recovered game
file, runtime wire ABI or DRAW-02 presented-session capture. Confidence is high
within the owned synthetic execution scope. Original transition, action, media,
caller-state and failure equivalence remain open.

`mnm-drawing-session-replay INPUT_DIRECTORY NEW_OUTPUT_DIRECTORY` reads
`drawing-session.json` and bound source files through the contained, read-only
native `AssetStore`. The output directory must not already exist. Build with:

```sh
cmake -S compat/legacy/drawing-scene -B working/build/draw03-session
cmake --build working/build/draw03-session --target mnm-drawing-session-replay
xvfb-run -a working/build/draw03-session/mnm-drawing-session-replay \
  <owned-input-directory> <new-output-directory>
```

The JSON declaration contains exactly these fields:

```json
{
  "schema": 1,
  "session": "1",
  "assets": {"ui.bmp": "<64 lowercase hexadecimal SHA-256 characters>"},
  "segments": [{
    "segment": 1,
    "stage": "menu",
    "producer": {
      "path": "menu.bin",
      "sha256": "<64 lowercase hexadecimal SHA-256 characters>"
    },
    "frames": [{"sequence": 2, "queues": 0, "canvas": 7, "width": 5, "height": 3}]
  }]
}
```

Session identity is a canonical decimal string in 1..18446744073709551615,
avoiding JSON floating-point loss. Stage labels are `scene`, `menu`, `loading`,
`world`, `results`; no game screen numbers or actions are inferred. Segment
ordinals start at 1 and are contiguous. Producer sequences and World queue
identities start afresh in each segment. Frame sequences increase strictly
within each segment, and queue counts cannot decrease. The runner assigns global
presentation ordinals starting at 1 across all segments.

Each producer is an existing closed MNMPRO01/02/03 stream, decoded with the
existing admission rules. No original destination plane or original raster EAX
can supply native output. Raster field 21 must be zero. Bindings contain encoded
source assets only, and every declared binding is checked even when unused.
All installed `.spr` catalogue files also require contained hash bindings, even
when not named by a producer callback. The World catalogue reads these sources
independently; its complete input surface is checked before and after execution.
Empty asset bindings are allowed for fully owned fill histories without SPR files.
Unknown or missing JSON fields and numeric booleans/fractions are refused.

Every nonempty frame list ends at the producer's last record; no hidden tail is
discarded. A frame-less nonterminal segment explicitly cancels its history and
can interrupt an active World queue. The final segment must finish on a complete
declared frame. Native `DrawingScene` still enforces defined pixels, matching
canvas/extent and completed World queues at every publication. An explicit stage
transition starts undefined storage, even with a reused canvas ID.

Admission is bounded to a 4 MiB declaration, 64 segments, 8192 frames, 1048576
records, 2048x2048 frame extents, 512 bindings of at most 32 MiB each (512 MiB
aggregate), 512 MiB per producer and 1 GiB aggregate producer bytes. Existing
wire-version limits can be narrower. These are native policy caps, not original
limits. The aggregate producer bound counts every segment, including repeats.

Successful output contains `frame-000001.565` onward: row-major little-endian
RGB565, exactly width*height*2 bytes. `session-report.json` records each frame's
session/segment/stage/presentation/source identity, dimensions, output path and
SHA-256. It records the input declaration hash, actual counters, finished state,
GPU readiness, driver and zero remaining surfaces after shutdown. These outputs
can be supplied to an external comparator; this runner reads no oracle.

Input preflight failures leave no output directory. Execution failures write a
report with `success: false`, an error, failed state and observed resource count
(null when renderer initialization failed before a count could be observed);
partial frames from earlier publications can remain and must not be admitted as
a passing run. A report is also emitted on stdout and exit status is nonzero.
After execution the declaration, every producer and every source binding are
read again to verify immutability. Hashes and contained path resolution reject
changed sources, traversal and symlink escapes. GPU frame leases are released
before session shutdown. Physical display presentation and live replacement are
not claimed.

`tools/test-native-drawing-session-runner.py` constructs owned producer packets
and an encoded BMP, independently checks six complete planes across six native
segments (including interrupted World history), and checks 29 refusal cases.
The evidence retains execution logs, prospective scope/source claims and binary
hashes; it supplies no original behavioral comparison.
