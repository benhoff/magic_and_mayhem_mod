# Multiple native creature presentation (NS12)

This extends NS07/NS08/NS09's diagnostic scene to the NS11 native multi-movement
policy. It does not recover original world scheduling or multi-entity admission.
`NP.presentation` and `NR.world_scene` cover the presentation implementation;
`NP.multi-movement` retains its separate headless scheduling evidence.

The scene admits at most 32 owned same-profile terrain-motion creatures. Each
actor uses its own fine position and current or completed segment's displayed ANI
record. Composition does not advance an animation clock. Stationary occupants,
cleaned creatures and other entity families are not presented by this slice.
Motion drivers without terrain height are refused. Terrain and every displayed
creature join one recovered depth queue, retaining slot/generation identity through
sorting and frame export, including equal depth keys. The draw bound admits up to
three records per 4,096 terrain tiles plus 32 actor bodies; uploads remain sequential
and bounded independently of animation length.

The preview resolves single, stationary or multi-movement checkpoint policy from
its resource fingerprint before constructing the session. Ordinary-map geometry
checks retain the untagged frozen-byte fingerprint separately from the policy
fingerprint, so added policy tags neither bypass immutable resource checking nor
cause spurious geometry mismatch. Existing diagnostic camera/projection, one
shared creature SPR/ANI binding, RGB565 rendering and checkpoint wire format remain
unchanged. Exported creature draws and actor positions now include slot/generation.
The Step and Save actions continue to use the owned simulation session.

`mnm-world-sandbox move-pair-terrain-ani MAP ANI BASE OUTPUT` takes two XYZ
starts/targets followed by ticks. It creates a terrain-motion/ANI checkpoint using
the NS11 policy; the existing scene CLI can open it without a new mode switch.

## Validation

Run `python3 tools/test-multi-world-scene.py SCENE SANDBOX NEW_OUTPUT` with normal
or ASan/UBSan executables. The runner verifies the original manifest before and
after installed SPR/TTD reads, uses synthetic terrace, slope and vertical navigation
and owned synthetic ANI, and checks four camera views. For each actor, exported
fine coordinates/generation agree with the simulation trace, draw anchors/depth
agree with an independent projection calculation, and the complete RGB565 image
agrees with CPU composition of the exported sorted queue and installed SPR masks.
Opposite-facing actors must display distinct frames; both must be visible together.
A verified fixture-only checkpoint rewrite tests generation `0xfedcba98`, beyond
signed 32-bit, with envelope validation and a new checksum; exports must retain
its exact unsigned identity. Current ANI display frames also agree with each
actor's simulation cursor. Split/fresh-process continuation compares frame JSON,
RGB565 bytes, PNG bytes and
complete final checkpoint against an uninterrupted run. These are native diagnostic
comparisons, not comparisons against captured original multi-creature rendering.
The C++ scene suite also checks masked/clipped overlapping actor pixels and handle
identity at equal depth against its CPU oracle and verifies no GPU surface leaks.

The accepted [record](native-multi-world-scene.json) pins sources, reports, binary
and consumed asset hashes. Prior evidence keeps its original fingerprints;
source changes can leave historical scene or sandbox evidence stale. Separate
[history accounting](native-multi-world-scene-history-review.json) records committed
intermediate receipts without asserting a historical gate or new equivalence.

## Remaining boundaries

Installed multi-creature entity initialization/admission, captured original
multi-creature frame equivalence, mixed SPR/ANI profiles, attachments, shaded
terrain/light integration, visibility activation, camera/action mapping, AI/combat,
automatic play and live replacement remain open. Conservative logical conflict
waiting still follows NS11 and does not imply physical collision or deadlock
resolution. No gameplay balance or original files are changed.
