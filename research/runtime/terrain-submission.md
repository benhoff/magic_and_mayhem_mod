# Selected terrain sprite submission

Offline evidence for No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The original `0x004f8960` function runs unchanged in a private PE mapping.
Fixtures disable object, overlay and creature branches, keep the picking point
outside sprite bounds, and initialize scratch globals. No Win32 calls, Wine,
game sessions or executable instruction patches are involved.

## Confirmed selected contract

A 12-byte terrain owner contains definition WORD +0, object references +2/+4/+6
and flag WORDs +8/+10. Selected ordinary terrain uses definition stride 356;
view-indexed DWORD arrays at +0x84, +0xd0 and +0xe0 select body, first and second
SPR roles. Scene +0x31 supplies raw view 0..3. Native reconstruction accepts
owned catalog bytes and returns numeric frames/roles, not original pointers.

World X is column*32+16, Y is row*32+16 and height is level*16. The producer
uses explicit scene pixel anchors +0x89/+0x8d and priority +0x9d for every role;
it does not derive those anchors inside the selected path. All arithmetic in
coordinate/key production preserves original DWORD wrap. Light is a signed
map byte minus scene +0xb1, clamped only below at -127. The queue stores its low
signed WORD; original shade quantization is fixed to 1 in these fixtures.

Admission checks orientation/player concealment bits or the combined body and
both-layer visibility flags, followed by an unsigned level/cut-level test.
Mode 1 rejects a tile when (flags8 & 3) exceeds the player selector. Body ID zero
skips all roles. Nonzero frame IDs above the supplied collection bound resolve
to frame zero. The original descriptor comparison is inclusive; native callers
supply the last owned frame as their safe bound. This does not establish safe
original behavior for a malformed one-past-end collection reference.

Body draw kind is 33, or 31 when flags10 bit 0x0008 is set. Children use kind 33.
All roles share the owner pointer in original records; kind 33 contributes to
coverage, while kind 31 only tests it. A preexisting flags8 bit 0x0004 skips
both children. An absent first frame sets flags8 bits 0x8004 and also suppresses
the second. Otherwise the first is submitted; bit 0x8000 gates the second, and
an absent second sets that bit. This differs from independently submitting
three present frames. Owner role identity subsequently follows the recovered
[sprite visibility](sprite-visibility.md) body/first/second precedence.

## Evidence and confidence

`python3 tools/test-terrain-submission.py` verifies original manifests before
and after and hashes the PE, sources, installed TTD/SPR and compiled helper.
[Comparison report](terrain-submission.json) records 15,204 matching tile cases:
8,192 synthetic states plus every one of the 1,753 Celtic Forest definitions
in all four views. It compares 15,317 queue records, frame resolution, draw
anchors, key, kind, shade, owner identity, both sentinel WORDs and owner flags.
Synthetic states include absent/out-of-bound roles, concealment, mode/player
admission, signed priorities, height and light variation. The scratch owner
has no object references or creature-presence flag; excluded branches are not
claimed by these comparisons.

The same frozen helper produces 32 installed nine-tile queue/owner fixtures,
including original sorting and optional reverse visibility. Selected definition
IDs 5,9,...,37 exercise orientation; repeated ID 1745 exercises real coverage.
Artifacts: `working/tests/terrain-submission/run-v_f_t_ex/`. Confidence is high
for this selected ordinary producer. TTD consumption at these offsets is
confirmed; unrelated record meanings and full original catalog loading remain
outside scope.

The [owned MAP reader and installed slice preview](native-map-terrain-preview.md)
now supply actual selected grid cells. Remaining boundaries: full world
traversal, upstream anchor/base-priority production, picking side effects,
water/overlays/objects/creatures, lighting and
palette chains, original whole-world rendering, visibility activation and live
replacement. See the [bounded native preview](native-terrain-preview.md).
