# Legacy ANI and SPR loading

Reviewed 2026-10-04. Native loading now covers all 136 installed ANI files
and all 181 installed SPR files. This extends offline input support, not live
animation/rendering replacement. Confidence is high for installed stored layouts
and native output. ANI expansion is supported by hash-pinned original loader
assembly; SPR version-2 evidence is complete installed inspection and independent
row decoding. No original version-2 SPR loader has been identified/executed.

## ANI versions 3 and 4

The 44-byte header and record-index offset table have the same layout as
[version 5](ani-native-loading.md). Record strides differ:

| Source version | Record bytes | Expansion to native 44-byte record |
| --- | --- | --- |
| 3 | 28 | Preserve seven stored DWORDs; append four zero DWORDs |
| 4 | 36 | Preserve nine stored DWORDs; append two zero DWORDs |
| 5 | 44 | Preserve all eleven DWORDs |

`Animation.version` retains the source version. All records use the same native
`AnimationRecord` representation, so consumers need no alternate strides.
Source count, exact file extent, monotonic record starts, final extent and each
sequence's last stop are checked before exposing a result. Existing input,
sequence and record limits apply to every version. Versions 1, 2 and unknown
versions remain unsupported; they have no installed inputs in this inventory.

Installed older files are `Sprites/objects.ani` (v3, 241 records),
`Sprites/cursors.ani` (v4, 168 records) and `Sprites/LordKing.ani`
(v4, 56 records). Total 465 records and 84 sequences. All 136 ANI files now
supply 121,941 normalized records.

At No-CD loader `0x004644d0`, the jump table at `0x00464ab4` selects the
v3 path at `0x00464758` and v4 path at `0x004648af`. Their source strides
are 28/36 bytes and destination stride 44. The v3 path zeroes destination
+28..+43; v4 zeroes +36..+43. After conversion `0x004649fd` sets the
original runtime header version to 5. Native output retains source provenance.

The original copies each old record's name using an unbounded NUL-terminated
string operation. Native input preserves all eight stored name bytes, including
padding after NUL or names with no terminator. It does not reproduce undefined
memory reads or uninitialized name padding. This is intentional safe native
policy, not a claim of byte-identical legacy heap contents. Selected forward
controller oracle traces remain version-5 evidence; no new legacy playback
or original file-reader execution is claimed here.

## SPR version 2

| Field | Version-2 layout |
| --- | --- |
| File header | 20 bytes: `SPR\0`, file size, version, frame count, palette count |
| Embedded palette | Begins at +20; 768 bytes of RGB triples |
| Offset table | After palette; one DWORD per frame, relative to frame-data base |
| Frame header | 32 bytes: extent, width/height, signed origins, eight name bytes, opaque palette word |
| Row table | Begins at frame +32; pairs of relative delta/pixel DWORD offsets |
| Row data | Alternating transparent/opaque byte runs; opaque pixels are byte palette indices |

There is no file flags word at +20 and no frame auxiliary offset pair at +32.
Those positions hold palette and row data. Native `headerFlags` and auxiliary
outputs are zero/empty to represent absent fields. The raw frame +28 word is
preserved as optional `SpriteFrame.legacyPaletteWord`; it is never dereferenced
or treated as a palette index. Installed values resemble serialized addresses,
but their origin is unresolved. The native palette index is zero.

All seven installed files have exactly one palette. Native v2 support explicitly
requires one embedded palette; zero/multiple palettes are rejected as outside
this recovered schema. This policy does not establish what any original older
loader would accept. Palette-free RGB565 input remains supported for v4.

The seven files are `interf64.spr`, `LOGO.spr`, `magico64.spr`,
`magico80.spr`, `overlay.spr`, `scanner.spr` and `Scrollbutt.spr`, all under
`Sprites/`. Their 162 frames contain 278,620 pixel slots. Both magico files
share frame geometry but retain their own palette/opaque words.

The shared native decoder selects 32- or 40-byte frame headers, validates
row extents, run coverage, dimensions and partial overlaps, preserves aliases,
and enforces aggregate scanned/pixel/decoded limits. SFT version 3 still uses
40-byte frames. Transparent slots stay zero with a separate mask; opaque index
zero remains opaque. Owned output survives input destruction.

The modern original loader `0x0057d310` uses +24 palette/table placement and
requires version 4; it provides no v2 compatibility evidence. Existing MMSprite
notes suggest the four-byte file-header adjustment, but their Python path reads
palettes at +24 and cannot be the authoritative v2 palette oracle. The new
reference reads the full palette at +20 and verifies every native byte.

Original function boundaries are also recorded in
[legacy reader contracts](../runtime/legacy-asset-loading.md).

## Evidence and reproduction

```sh
cmake -S assets -B working/build/dat
cmake --build working/build/dat -j4
ctest --test-dir working/build/dat --output-on-failure
python3 tests/test-legacy-asset-loaders.py \
  working/build/dat/mnm-animation-inspect working/build/dat/mnm-sprite-inspect \
  --installation working/game-clean --report working/legacy-asset-comparison.json
python3 tools/export-legacy-asset-support.py --decompile
python3 tools/compare-mmsprite-binary.py \
  --native-inspector working/build/dat/mnm-sprite-inspect
```

The independent reference compares every normalized ANI record hash, source
header/name and start offset; expansion can be reversed to the exact source
record bytes. For all seven v2 SPR files it compares every palette byte, frame
field, stored opaque word, pixel and mask. Its row walker stops at width rather
than using the native adjacent-row extent algorithm. Source hashes are checked
again after comparison. Four synthetic reference cases and C++ fixtures cover
normalization, exact names/bit patterns, source ownership, empty frames, aliases,
opaque zero, malformed ranges/overlaps/runs and resource limits.

All 37 asset CTests and six related ASan/UBSan tests pass. Leak detection is
disabled because LeakSanitizer cannot run under sandbox tracing. Installed
comparisons and original evidence export verify the original manifest before
and after; no files under `original/` are changed. Hashes and counts are retained
in [legacy-ani-spr-loading.json](legacy-ani-spr-loading.json).

The existing sprite oracle workflow also passes with all 181 files decoded:
59,569 frames, 91 original version-4 draw matches and 77 colour-conversion
matches. The animation workflow decodes all 136 inputs and retains 347 selected
version-5 controller trace matches (22,555 states). These regression results
do not promote old file readers or old playback to executed-original evidence.
The 32-bit reference harnesses require execution outside the sandbox syscall
restriction; initial sandbox attempts could not execute them.

Model action/direction semantics, original legacy name-padding behavior,
version-2 palette-word provenance, old loader execution and live display
remain separate boundaries. This completes installed ANI/SPR byte loading;
full save-world decoding and end-to-end movie validation remain outstanding.
