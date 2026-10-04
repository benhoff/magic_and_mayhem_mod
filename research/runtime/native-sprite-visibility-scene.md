# Native SPR visibility preview integration

The Qt sprite scene has an explicit `--visibility` option applying the
[recovered SPR visibility pass](sprite-visibility.md) after original queue
sorting. The native asset loader retains opaque auxiliary bytes, the
reconstruction interprets/tests them, and application orchestration decides
when to apply the pass. Renderer surfaces and widgets retain their existing
boundaries. No game launch, Wine session or live hook is required.

Each query starts with an empty owned 14,577-byte coverage grid. Visible body
and configured child entries receive normal draw kind 0; the original reverse
pass visits last through index 1, retains first-entry behavior, and marks fully
covered draws as -2. Hidden records remain in exported diagnostics and are
skipped by the native draw loop. SPR pixel masks/palettes/origins, cache ownership
and per-frame background clearing continue independently of visibility masks.

Default preview admission remains disabled, matching the preceding evidence.
Original activation depends on globals and world state that the preview does
not have. `--visibility-expanded` selects original expanded viewport projection/
bounds and requires visibility admission; the 512x256 canvas is unchanged.
Creature/example-layer previews have no terrain-owner links. Resolved owner
flags are tested in the original/native model fixtures, not synthesized into
creature state. No claim is made that original world submissions use these
preview draw-kind, role, priority or activation policies.

## Repeatable offline comparison

After building `apps/sprite-scene`, obtain reports from the original queue and
visibility comparisons, then run:

```bash
python3 tools/test-sprite-visibility-scene.py \
  --queue-reference working/tests/sprite-queue/<queue-run>/report.json \
  --visibility-reference working/tests/sprite-visibility/<visibility-run>/report.json
```

The runner verifies original executable/reference/helper/source/input hashes and
immutable manifests before/after. Twenty-four original-built/sorted queue
fixtures are exercised in both viewport modes. Installed RedCap bodies (0/4)
and explicit Bat/Eye child layers (0) overlap at one pixel anchor. These example
layers remain unrelated to recovered creature attachment recipes.

For each fixture, original queue orders and raw ANI records supply frame IDs
and anchors. A bounded fixture file stores complete raw SPR frame records;
the unchanged original `0x005015f0` pass returns draw kinds and is independently
compared to the native model's complete coverage grid. Native sorted identities,
keys and kinds must match. An independent SPR decoder/CPU compositor then draws
only original-surviving entries and checks the complete RGB565 frame and RGBA
presentation hash. It does not use the native scene's anchors to build expected
pixels and does not infer coverage from transparency.

[Evidence](native-sprite-visibility-scene.json) records 50 complete installed
frames per normal/sanitized build, including original hidden-draw decisions,
zero final native surface ownership, source/input hashes and artifact directories.
The 48 creature/example-layer cases remain visible in the original pass; they
validate mask/grid transformations without inventing hiding. Two additional
cases use installed Celtic Forest Terrain.spr frame 1745 in unchanged encoded
form, inside a one-frame fixture SPR and four synthetic ANI sequences. Both
viewport modes produce original kinds `[0,-2,-2,0]`; the native decisions and
complete surviving-frame/presentation bytes match. Fixture containers are written
by the test script; original assets and frame encoding remain unchanged.

All five targeted CTests pass normally and with ASan/UBSan; leak checking is
disabled for this environment. A synthetic scene additionally pins the exact
three-draw result `[0,-2,0]`, checks full masked pixels including opaque zero,
and verifies background clearing when all players stop.

Confidence is high for selected bitmask/pass behavior and these native fixtures.
This is original ordering/visibility plus independent CPU rendering, not original
whole-scene rendering. Complete terrain producers, mask generation, world role
orientation/ownership, original admission/lifecycle, lighting and live rendering
remain unverified and unconnected.
