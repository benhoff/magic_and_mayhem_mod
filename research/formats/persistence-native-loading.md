# Native persistence and progression readers

Reviewed 2026-10-04. Offline, read-only C++17 loaders in
[assets/persistence.hpp](../../assets/persistence.hpp) use owned standard-library
values and the existing `AssetFile` interface. The `mnm-persistence-loader` target
depends on `mnm-assets`; no widget, Wine hook or reconstructed runtime object is
part of its API. The implementation never restores host pointers or invokes
campaign rules.

## Supported inputs

| Input | API | Result / boundary |
| --- | --- | --- |
| Packed/encrypted CFG container | `decodePackedContainer`, `loadPackedContainer` | Seed/mode/checksums and decoded bytes; packing modes 0/1/2 |
| Plaintext CFG | `decodeConfig`, `loadConfig` | Owned section/key/value map |
| Packed CFG text | `loadConfig(file, true)` | Container validation followed by CFG parsing |
| `RealmView.cfg` | `decodeRealmConfig`, `loadRealmConfig` | Name, next realm, player/last-region, wizard configuration and optional region owners |
| `*regionNames.txt` | `decodeRegionNames`, `loadRegionNames` | Ordered byte strings, preserving blank lines |
| Decoded save / `.vas` | `decodeSavedGame`, `loadSavedGame(file, false)` | Version-20 campaign blocks and optional opaque world tail |
| Packed `.sav` | `loadSavedGame(file)` | Container validation followed by decoded-save parsing |
| Extracted realm-state subblock | `decodeRealmState`, `loadRealmState` | Exact 0x13dc raw bytes plus statically recovered fields; this is not a standalone installed extension |

`Bytes` and all nested strings/vectors are owned, including unknown fields.
An input buffer, file handle and store may be destroyed after a successful load.
For example, call `auto result = loadSavedGame(file);`, check
`std::get_if<PersistenceError>(&result)`, then take `SavedGame` from the variant.
Use `loadSavedGame(file, false)` for an already-decoded stream.

The optional world data retains the 24-byte world globals separately from the
remaining serialized world. It is **not** a parsed or restored world/simulation.
Wizard record field meanings and script records also remain largely opaque.

## Native parsing policies

These are deliberate native input policies, not recovered original failure
behavior. `PersistenceResult<T>` contains a value or a `PersistenceError`
(code, byte offset, detail and optional underlying asset error). Never extract a
value before checking the variant. File read/limit errors retain their original
`Error` inside the `assetInput` error.

Default limits: 64 MiB input and decoded data, 65,536 text lines and DWORDs per
wizard list. Decoders validate available bytes before allocating variable arrays,
use explicit little-endian reads and catch allocation/length failures. The fixed
wizard-entry storage admits at most 42 entries per record. No partial result is
returned after a parse failure. Text decoders apply the decoded-byte limit; file
wrappers additionally apply the input-byte limit.

`ContainerTransform::cfgBytes` is the default for shipped CFGs. Saved-game
loads explicitly select `ContainerTransform::noCdSave`: the No-CD outer
transform repeatedly XORs the first leftover byte, without advancing the
pointer. The other leftover bytes are untouched. This differs when there are
two or three trailing bytes. Do not guess or silently fall back between builds.
Use `loadPackedContainer(file, limits, transform)` for an explicit selection.

The container decoder validates both alternating XOR/add DWORD checksums,
ignoring the final one to three bytes as the recovered codec does. Raw payloads
can contain checked padding; compressed payloads can retain unused trailing
bytes/bits after the declared output is complete. Native readers reject runs
that exceed the declared output instead of truncating their output as the Python
reference does. Unsupported modes, truncation, early LZSS termination and
checksum mismatches fail explicitly. Generator state is local to each decode.

CFG parsing accepts `key=value`, ASCII-case-insensitive section/key names,
ASCII space/tab trimming, LF/CRLF, whole-line `;`/`#` comments and keys before a
section. Values preserve additional `=` characters. It rejects duplicate keys,
empty section names, malformed sections and embedded NUL. Shipped `creature.cfg`
and `effectani.cfg` contain bare titles, and `objects.cfg` contains an empty-key
` = ANI_102` line. These non-key lines are retained as owned annotations with
byte offsets rather than rejected or interpreted as properties. It does not implement
Windows profile fallback, quotation removal, numeric coercion or inline comments.
This is a native subset, not general `GetPrivateProfileStringA` equivalence.
Text is retained as byte strings (the inspector displays it as Latin-1).

The typed realm reader requires the documented GENERAL and wizard keys,
strict signed decimal integers, 1–80 wizards, 1–20 regions and valid player and
last-region indices (-1 is accepted for last-region). Region ownership keys
may be absent; supplied owners must be -1 or a valid wizard index. Wizard
locations and movement regions remain numeric values: the installed Medieval
configuration includes location 17 with regionCount 17, so regionCount must not
be interpreted as a universal zero-based upper bound. No missing owner/default
is invented. The raw realm reader preserves dormant slots and does not apply
configuration validation to saved state.

The save reader requires `SAV\0` and version 20, accepts only world markers 0 and
0x17, and accepts only extension presence bytes 0/1. Campaign-only saves must end
at the final marker. The accounting field is exposed without interpreting it as
file length. These stricter checks differ from selected original reader branches.
The world tail is bounded by the decoded-file limit but has no schema validation.

## Build, inspect and validate

```bash
cmake -S assets -B working/build/persistence -DBUILD_TESTING=ON
cmake --build working/build/persistence --parallel 4
ctest --test-dir working/build/persistence --output-on-failure
working/build/persistence/mnm-persistence-inspect working/game-clean realm Realms/Celtic/RealmView.cfg
working/build/persistence/mnm-persistence-inspect working/game-clean container CFG/Encrypted/creature.cfg
python3 tests/test-persistence-loaders.py working/build/persistence/mnm-persistence-inspect --installation working/game-clean
```

The inspector takes `ROOT TYPE PATH`; types are `cfg`, `packed-cfg`, `container`,
`realm`, `regions`, `realm-state`, `sav`, `vas`, `save-container` (explicit
No-CD container transform). It uses bounded read-only asset
resolution, emits JSON and returns 2 on failure. Select the layer explicitly;
it does not guess from an extension or reinterpret malformed packed data as text.
The installed comparison runner verifies the immutable manifest before and after,
including on failure. Omit `--installation` for installation-independent fixtures.

Evidence from this milestone:

- Seven asset CTests pass, including native malformed/truncated/bounds/lifetime
  fixtures and an independent Python codec comparison.
- 36 CFG and 36 No-CD save-container synthetic matches cover modes 0/1/2, empty outputs, final partial words,
  multiple seeds, the RLE signed-byte boundary and LZSS ring wrap. Native fixtures
  additionally exercise repeat runs and overlapping LZSS references.
- Synthetic `.sav` and `.vas` layers agree; all 80 wizard slots, nested entries,
  a DWORD list, optional 0x57c extension, campaign/world branches and realm-array
  end offsets are tested. Additional world tails of lengths 1/2/3 verify the save-specific transform.
  Synthetic saves are not original-generated saves.
- Installed comparisons cover decoding and CFG parsing of all 11 encrypted CFG containers, all three
  RealmView configurations and four region-name files. Decoded container SHA-256
  values match `tools/decode-cfg.py`. Report:
  `working/persistence-loaders-validation-final.json` (disposable generated output).
- AddressSanitizer/UndefinedBehaviorSanitizer pass for both persistence tests.
  LeakSanitizer cannot run under this environment's tracing; these runs use
  `ASAN_OPTIONS=detect_leaks=0` and do not establish leak-detector coverage.

No real game save has been decoded, no original/native save comparison or live
load has been performed, and no writer was implemented. The outer save-container
correspondence and inner save layout have static evidence plus native synthetic
tests; they remain candidates for real-save compatibility. Campaign behavior,
full world serialization, reference rebinding, durable writes and application
integration are separate milestones. See [save format](save-game.md),
[runtime research](../runtime/persistence-progression.md) and
[coverage ledger](../runtime/coverage-ledger.md).
