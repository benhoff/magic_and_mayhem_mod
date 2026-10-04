# Saved battle/world structural loading

Reviewed 2026-10-04. The version-20 No-CD save envelope's `0x17` branch
contains 24 bytes of globals followed by this world stream. All integers are
little-endian. This is a recovered **physical byte grammar**, not a portable
simulation snapshot schema. Gameplay field meanings, resource recreation,
reference validity and simulation restoration remain incomplete.

Evidence: paired writers/readers from executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`;
[function inventory and experiment](../runtime/save-world-serialization.md).
Confidence is high for reviewed write/read sizes and order, with selected
original writer execution supporting entity extents. No original-generated
complete save has been tested.

## Native interface and policy

`assets/save_world.hpp` adds `decodeWorldState`, `loadWorldState` and
`decodeSavedWorld` to `mnm-persistence-loader`. The last requires world marker
`0x17`; a campaign-only save has no world to decode. Results own exact input
bytes, map path, four globals, counter and named offset/size ranges with parent
indices. Offsets refer to the owned world bytes, excluding the outer 24 globals.
Parents encompass their children. Serialized references remain raw values;
no original host pointer is dereferenced or recreated.

The existing envelope reader still accepts opaque world tails. Structural
validation is explicit, so callers can inspect campaign state even when a
world record is unsupported or malformed. `mnm-persistence-inspect` adds:

```bash
working/build/dat/mnm-persistence-inspect ROOT sav-world Save/example.sav
working/build/dat/mnm-persistence-inspect ROOT vas-world Save/__temp.vas
working/build/dat/mnm-persistence-inspect ROOT world extracted-world.bin
```

JSON has a `world` object with `map_path`, `globals`, `counter`, `source_bytes`
and `blocks`. Numeric/string/entity payloads not interpreted by this structural
reader are preserved in owned raw bytes. This does not implement native entity
models, a save writer or live load integration.

Defaults: 64 MiB input, 96 MiB owned decoded-data accounting, 65,536 records per
count and 200,000 named blocks. Nested word lists have a 30-element storage
bound and the final resource array has three slots. Counts, products, every
extent and trailing bytes are checked. Resource budgets and strict rejection
are native policy; original malformed-input behavior is not reproduced.
Decoded accounting charges raw bytes and logical metadata/string storage, not
allocator capacity or transient file/input copies. Strings preserve original
bytes; map-path text is bounded to its 256-byte field even without a terminator.

## World stream order

Notation: `C` is a DWORD count; `P` is a DWORD presence flag (nonzero means
present); `A` is an animation state (below). Named list bodies retain raw records.

| Order | Physical bytes / sequence |
| --- | --- |
| Map path and globals | 256 + four DWORDs |
| Resource registry | C + C * 264 |
| Counter | DWORD |
| Map | Descriptor 76 bytes, cell count at descriptor +20; count *12 cells; DWORD; two lists each 16-byte header + C + C*40; 1,864 array bytes; C + C byte state; C + C*(DWORD cell reference + A); five DWORDs; list 16-byte header + C + C*72 |
| Secondary map | Two DWORDs |
| Queued cells | C + C DWORD references |
| Records-40 | C + C*40 |
| Eight state slots | Eight times P + (if present 336 bytes); final DWORD |
| Creature pool | Two DWORDs; C + C*24 header records; C variable creatures; 112 bytes; C + C DWORDs; 64 bytes |
| Globals | Three DWORDs |
| Missile pool | C, DWORD, C variable missiles, three DWORDs, four lists each C + C DWORDs |
| Effect pool | C, C variable effects, two DWORDs |
| Path slots | 104 * 256 bytes |
| Optional records-660 | P; if present C + C*660 + 26,744 fixed bytes |
| Optional records-6 | P; if present C + C*6 + four DWORDs |
| Conditional slots | 100 times P + (if present three DWORDs) |
| Records-1148 | DWORD state + C + C*1148 + 412 bytes |
| Controls | DWORD + eight byte flags + two DWORDs |
| Resource slots | C (at most three); each DWORD length + length+1 string bytes + two DWORDs + byte flag |
| Final globals | Two DWORDs |

Animation `A`: DWORD sequence; `0xffffffff` ends the record immediately,
otherwise three DWORDs follow (16 bytes total). Original conversion is tied to
loaded animation resources; the native reader only identifies the extent.

Missile: ID DWORD + active DWORD. Inactive records end there. Active records
contain eight DWORDs, 252 bytes, 56 bytes, presence DWORD, optional DWORD
reference + A, then 154 bytes. Effect: ID + active; active records contain seven
DWORDs + WORD + DWORD type; A unless type is 50 or 61; seven final DWORDs.

## Creature record order

All records start with ID + active DWORD; inactive records end at eight bytes.
Active records use this sequence:

1. 140 bytes, DWORD type. Types 0/24/25/26 add C name bytes (writer includes
   the NUL when a name exists; zero count is allowed).
2. A; DWORD; DWORD mode; two DWORDs; three additional DWORDs when mode is zero.
3. 48 bytes; A; 28 bytes; 42 *24-byte entries.
4. 469 bytes; byte presence; optional A.
5. DWORD kind; DWORD reference. Kinds other than 0/2 add DWORD reference + A.
6. 25 bytes; three successive DWORD presence flags each followed by optional A.
7. 56 bytes; C commands, each 36 bytes; selected-command DWORD if C is nonzero.
8. 737 bytes; two DWORDs; C (at most 30) + C DWORDs;
   C (at most 30) + C DWORDs; final DWORD.
9. DWORD; DWORD presence; optional 204 bytes; final 129 bytes.

The 36-byte command contains five base DWORDs plus four subclass DWORDs.
Byte flags and packed unaligned fields make a host C++ structure unsuitable
for disk parsing. Reference integers and command semantics remain raw.

## Validation and reproduction

```bash
cmake -S assets -B working/build/dat -DBUILD_TESTING=ON
cmake --build working/build/dat -j4
ctest --test-dir working/build/dat --output-on-failure
python3 tools/export-save-world-support.py --decompile
python3 tools/test-save-world-writers.py working/build/dat/mnm-persistence-inspect
```

The static exporter checks the executable hash and immutable manifest before
and after read-only Ghidra export. Recorded 55-function evidence:
`working/decompiled/save-world-support-xrj_iomg/report.json`.

All 39 asset CTests passed. World tests verify empty/populated streams, both
save layers, optional branches, known generated block offsets, nested ranges,
owned lifetime, truncation, unexpected trailing bytes and resource limits.
Four persistence/world checks passed with ASan/UBSan in
`working/build/persistence-sanitized`, with LeakSanitizer disabled for the sandbox.

Ten selected original writer captures were accepted at their exact generated
entity extents, spliced into synthetic complete world/envelope fixtures:
three creature, three missile and four effect cases. Capture and comparison:
`working/tests/save-world/run-knijjno2/report.json` and `native-comparison.json`.
See the runtime document for shim provenance and precise branch coverage.
These support selected serializers; they do not establish complete original
world writer/reader equivalence or real save compatibility. The remaining
persistence milestone is original campaign/battle save corpus validation,
followed separately by resource/reference reconstruction and live restoration.
