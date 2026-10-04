# Audio listener camera projection

Reviewed 2026-10-04. Complete selected integer projection and origin helper;
static reconstruction and offline fixtures, no live object mapping or injection.

## Evidence

`python3 tools/export-audio-camera.py` exports `0x004f7d00..0x004f7e54`, origin
helper `0x004f8230..0x004f82fb`, normalization wrappers and the projection jump
table at `0x004f7e58`. Evidence:
`working/decompiled/audio-support-05kmhu0c/manifest.json`, assembly files and
`camera-table.json`. No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All 2927 original files verified before and after. No executable changed or ran.

The projection table selects `0x004f7d51`, `0x004f7d82`, `0x004f7db0`,
`0x004f7de0` for orientations 0..3. The origin table at `0x004f8300`, included
in the origin assembly export, selects `0x004f8274`, `0x004f8295`, `0x004f82b8`,
`0x004f82d9`. These are different functions, not interchangeable tables.

## Recovered fields and formulas

`CameraState` is a decoded host annotation, not the packed engine layout.
Fields named f11, f15, etc. identify camera offsets where physical semantics
are not yet established. The fixed camera reference at this call site is
`0x00689930`; its orientation +0x31 equals the positional routine's global
`0x00689961`. This is static build evidence, not a stable live-pointer promise.
Map extents come from the object at camera +0x25, fields +4 and +8. Supplying
consistent dimensions to both camera and positional models is a backend duty.

| Camera field | Use in these blocks |
| --- | --- |
| +0x11/+0x15 | Base screen-origin terms |
| +0x31 | Orientation 0..3 |
| +0x39/+0x3d | Map-coordinate anchor terms |
| +0x41 | Vertical origin term, multiplied by 16 after subtracting 2*field5d |
| +0x45/+0x49 | Orientation-dependent origin offsets |
| +0x4d | Additional vertical origin term |
| +0x51/+0x55 | Signed halves added to origin x/y |
| +0x5d | Shared diagonal/map-anchor offset |
| +0x61 | Vertical projection correction, multiplied by 16 |

Let `hx=trunc(field51/2)`, `hy=trunc(field55/2)`,
`S=field45+field49`, `D=field45-field49`, and
`H=uint32(S)>>1`. Origin base is:

```
Ox = field11 + hx
Oy = field15 + 16*(field41 - 2*field5d) + hy + field4d
```

Apply orientation offsets to that base:

| Orientation | Origin x adjustment | Origin y adjustment |
| --- | --- | --- |
| 0 | -D | -H |
| 1 | -S | trunc(D/2) |
| 2 | D | H |
| 3 | S | -trunc(D/2) |

H deliberately uses logical shift, including negative S. Replacing it with
signed division changes the original behavior. For a requested screen point:

```
a = screenX - originX
b = 2*(screenY - (originY + 24 - 16*field61)) + 56
u = trunc((b+a)/64)
v = trunc((b-a)/64)
```

The projected map coordinates before wrapping are:

| Orientation | x | y |
| --- | --- | --- |
| 0 | field39 - field5d + u | field3d - field5d + v |
| 1 | field39 + field5d - v | field3d - field5d + u |
| 2 | field39 + field5d - u | field3d + field5d - v |
| 3 | field39 - field5d + v | field3d + field5d - u |

Each coordinate normalizes to `[0, extent)`. Signed divisions truncate toward
zero, matching the inspected sign correction before arithmetic shifts. Audio
passes screen center (320,240) when `0x006de6d5` is zero and (400,300) otherwise.
This recovers the two original call-site modes, not arbitrary native display
resolution support. `screenToMap` itself accepts explicit screen coordinates.

## Integration and validation

`reconstruction/audio/camera_projection.*` models origin, screen-to-map,
audio-center selection and `cameraPositionalControls`. The latter feeds decoded
listener coordinates, dimensions and orientation into the existing positional
model; that model then applies its separate +/-6 listener bias. There is no
Qt/widget dependency or process-pointer access. Native audio services do not
depend on the reconstruction.

```bash
python3 tools/test-audio-camera.py
python3 tools/export-audio-camera.py # guarded static executable input
```

Fixture evidence: `working/tests/audio-camera/run-_41t8ns5/report.json`.
All 17 audio/asset CTests passed. A separate u/v rotation oracle and iterative
wrap oracle check 1152 screen/map cases: four rotations, odd/even spans, both
screen centers, negative origins, diagonal and vertical corrections, rectangular
small/large maps and points outside the screen. Another 68644 cases exercise
positive/negative 64-pixel division boundaries. Fixtures cover logical-shift
origin semantics, invalid orientation/dimensions and overflow rejection.
Four-orientation integration places a source at the recovered biased listener
center (volume zero, pan zero), then verifies distance cutoff.

`working/build/audio-output/audio-camera-sanitize` passed AddressSanitizer,
UndefinedBehaviorSanitizer and LeakSanitizer outside sandbox tracing. Tests use
no game assets, audio device, Wine or live game. Confidence is high for the
exported valid integer contracts and fixture integration. Invalid rotations,
nonpositive map extents and signed overflow are rejected explicitly; original
fallback/fault/overflow behavior is not claimed. Live camera-field capture,
viewport/object lifecycle and actual listener agreement remain unvalidated.
Map-byte lookup and ownership are the next separate reconstruction chunk.
