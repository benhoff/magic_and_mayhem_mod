# Native scene camera, entity rotation and spectator lighting

This packet connects existing recovered algorithms and native owners through
`SceneSession`. It changes no recovered numerical algorithm or gameplay policy.
Evidence belongs to `NR.scene-frame-camera`; the original camera/entity/lighting
parent milestones remain independent.

The accepted scene owns its camera lifecycle, creature/effect view arrays and
minimap invalidation. View metadata is copied from each admitted source, while
positions come from that source's current semantic entity cells. Recomposition
retains the resulting generation-bound caches and directions. Camera consumers require owned source layers, and two generations of the same
family/slot cannot coexist in an accepted snapshot. A new source
reimports authoritative animation state and refreshes it into the current view.
A catalog rebind uses the actual animation receiver resource in the draw and
frame plan, with its current resource revision. Catalog assets are loaded before
consumer execution. Generation/epoch retirement clears receivers and pending
camera work; decoded resources and GPU cache entries retire with the session.

`queueCameraTurn` sets the recovered boolean requests. `render` stages the
post-frame view consumers before GPU submission and publishes them only after
successful bounded frame delivery. Clockwise takes priority; a simultaneous
counterclockwise request waits for another completed plan. A repeated submission
of one frozen plan does not consume another request or decrement redraw frames.
The completed image retains its original projection; `recompose` builds the next
image using the resulting camera. A direct `setCamera` invalidates plans prepared
under the preceding projection. Rejected submission/consumer input preserves
camera requests, accepted view state and animation bindings. GPU pixels are not
a rollback mechanism if a driver fails during submission.

Lighting retains its admitted source-tick schedule. Frozen redraws and camera
turns do not advance simulation, animation or lighting clocks. The next source
tick projects the current camera into the spectator anchor and static admission
view. The original light-cycle publication latency remains explicit; no forced
publication is added for a redraw. Initial autofocus still precedes the first
light update. Creature slot generations and relation/enable controls participate
in owned recount detection, even when the visible light fields are identical.

`CAMERA.scene-frame-native-20261026` passes the current native64 source matrix:
eight exact CPU/software-OpenGL images, actual changing World health/cell input,
queued priority/idempotence, stale linked-generation failure before drawing,
special resource rebinding, layer ownership and duplicate-generation refusal.
`CAMERA.scene-frame-sanitized-20261026` independently passes the same frozen
source matrix under Clang ASan/UBSan. The first added World assertion incorrectly assumed a dense one-entry light
pool; its failed report is preserved. The corrected assertion counts active
entries in the bounded pool. An earlier sanitizer run passed all assertions but
correctly rejected changed test-source fingerprints; that report is preserved.
New results receive new evidence IDs; older hashes are unchanged.

The committed first-parent range `adbf8029..8c0c4ea4` was inspected independently:
489 exact file transitions, 147 verified SQLite payloads and zero missing exact
receipts, retained in `scene-camera-committed-history-20261026.json`. This is
retrospective accounting, with no historical gate or engine validation claim.
The finite native tests use generated ANI/SPR/TTD/map inputs and software Mesa
OpenGL. They do not establish installed whole-session equivalence, physical GPU
acceptance, original entity producer provenance or live replacement. Original
files are not consumed by this packet's synthetic checks.

`CAMERA.scene-frame-installed-20261026` independently checks read-only installed
Forest 40x40x19 MAP/MPS/terrain ANI/TTD/SPR, creature ANI/SPR and configuration
acquisition: 77 retained placements and 766 draws, plus exact native executable /
checkpoint image agreement. Original manifest verification before/after and
installed source-hash stability pass. This checks installed resource loading;
branch-specific camera/linked/spectator cases use controlled native sources.

Intermediate immutable results remain at `scene-camera-frame-native-dev01-20261026.json`,
`scene-camera-frame-native-20261026.json`, `scene-camera-frame-native02-20261026.json`,
`scene-camera-frame-sanitized-20261026.json` and
`scene-camera-frame-sanitized02-20261026.json`. They do not supply current-code
validation. Durable payloads and exact review bookkeeping live in SQLite;
retained `working/camera-*-final*` logs/builds are execution inputs/debug output.
