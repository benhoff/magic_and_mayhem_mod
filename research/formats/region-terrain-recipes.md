# Region terrain recipes and section metadata

The three installed realm CFGs contain 40 numeric REGION sections: Celtic
REGION0..9, Greek REGION0..12 and Medieval REGION0..16. Their original plain
Latin-1 text supplies these terrain selection fields:

| Key | Selected meaning |
|---|---|
| Name | Owned display name |
| Path | Relative installed section directory, commonly using backslashes |
| SpritePath | Terrain TTD/SPR directory; may differ from Path |
| SectionPreFix | Filename prefix; section ID is decimal with at least two digits |
| MapSize | `(columns,rows)` in section blocks |
| Specific | Ordered `(section,rotation,column,row)` tuples |
| Random | Ordered `(section_occurrences,...)` candidates |

`-1` in Specific fields expresses unresolved rotation/placement. Random
occurrences are not a simple request to place exactly that many files: the
original generator combines candidates with edge constraints and retries.
Native decoding preserves these values and their list order without running
that generator or interpreting balance/entity settings.

`assets/region_recipe.hpp/.cpp` returns independent owned strings and lists.
It reads only the above keys; unrelated repeated fields such as shipped
MaxWanderingCreatures settings do not cause a recipe failure. Duplicate
recognized keys, repeated numeric region sections, malformed tuples, missing
required fields, NULs, unsupported native capacities and invalid coordinates
are rejected. Names/keys are ASCII case-insensitive; semicolon comments are
removed before parsing, including comments after section headings.

The plain decoder is separate from packed-container handling.
`loadRegionRecipes(file, packed, limits)` delegates explicit packed inputs to
existing checksum-validated native unpacking, then decodes the resulting text.
The three installed realm CFGs are plain text and use the default `packed=false`.
The reader does not guess packing from the first byte. SpritePath falling back
to Path when absent is a native selection policy for the owned recipe service.
Original CFG parser equivalence is not claimed. Original field interpretations
are supported by installed comments and selected generator/caller assembly.

Native recipe limits match the selected 5x5 placement capacity: dimensions
1..5, section IDs 0..99, rotation -1..3, coordinates -1 or in bounds, and at most
50 Specific or Random entries each. Random occurrence counts are bounded to
1..999. Input/decoded bytes and line counts retain PersistenceLimits bounds.

```sh
working/build/terrain-region-frozen/sprites/assets/mnm-region-recipe-inspect \
  working/game-clean Realms/Celtic/Celtic.cfg
```

## Selected version-6 MAP header fields

The native MAP reader continues retaining all header metadata without applying
engine policy. Region reconstruction now interprets:

| Decoded header offset | Selected meaning |
|---|---|
| +04, +08, +0c | Width, height and layer count |
| +18, +1c | Section block columns and rows |
| +20, +24 | Two outer north edge labels |
| +28, +2c | Two outer east edge labels |
| +30, +34 | Two outer south edge labels |
| +38, +3c | Two outer west edge labels |

The placement descriptor builder `0052edf0` reads +18/+1c as block dimensions
and +0c as source layers, and extracts outer edge labels while creating unique
internal connectors for 2x2/2x1/1x2 sections. These selected interpretations are
confirmed by unchanged original calls on the two authored-region fixtures.
The complete version-6 reader/683-file payload evidence remains in
[MAP loading](map-native-loading.md) and
[the section copy evidence](../runtime/terrain-section-assembly.md).

The preceding section inventory contains 616 1x1, 59 2x2, five 2x1 and three
1x2 installed MAPs. All use a 20x20 tile block side. This inventory was read
from the independently decoded fixtures in
`working/tests/terrain-sections/run-4ih7usyv/map-*.bin`; it does not broaden
original descriptor execution to every installed MAP. Remaining metadata
fields are retained as opaque values.

Confidence: high for documented recipe syntax, owned decoding of all 40
installed entries and selected original descriptor operations; no original
whole-CFG parser or random generator equivalence. See
[authored-region integration](../runtime/terrain-authored-regions.md).

Selected single-block edge classification and occurrence admission now have
[offline native/original helper comparisons](../runtime/terrain-region-selection.md).
Random counts are admission maxima rather than selection weights; this does not
claim whole-generator or CFG-parser equivalence.

Descriptor expansion for all installed block shapes, internal connector pairs
and counter progression now have
[offline native/original comparisons](../runtime/terrain-region-constraints.md).
Connector threshold initialization and whole-region generation remain separate.
