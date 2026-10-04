# Native EVT event-area loading

Reviewed 2026-10-04. Owned offline version-1 input reader and inspector.
Evidence combines all installed files and hash-pinned original reader/writer
and selected section-loading consumers. Runtime trigger execution is outside
this input milestone.

## Confirmed layout

All 685 installed files total 156,400 bytes and 2,020 event-area records.
452 files contain zero records; counts range from zero to 80. All use version
one and `EVT\0`. Integers are little-endian. Confidence is high for boundaries
and both coordinate triples. The 48-byte name field follows installed readable
labels; complete original string/trigger semantics remain unverified.

| File offset | Bytes | Meaning |
| ---: | ---: | --- |
| 0 | 4 | `EVT\0` signature |
| 4 | 4 | Size word; original writer emits `16 + count * 16` |
| 8 | 4 | Version, one |
| 12 | 4 | Record count |
| 16 onward | count × 72 | Event-area records |

Actual extent is exactly `16 + count * 72`. The original reader ignores the
size word for admission; it is preserved as metadata. Its writer-generated
formula does not describe the actual file length.
See [original reader/writer and coordinate evidence](../runtime/evt-area-loading.md)
for addresses, executable hash and reproducible static exports.

| Record offset | Bytes | Native representation |
| ---: | ---: | --- |
| 0, 4, 8 | 4 each | Signed section-local x/y/z for first endpoint |
| 12, 16, 20 | 4 each | Signed section-local x/y/z for second endpoint |
| 24–71 | 48 | Raw name bytes, including any bytes after the first NUL |

311 installed records have reversed endpoint ordering on at least one axis.
60 contain nonzero bytes after the first name terminator. Preserve these,
including negative coordinates and duplicate endpoints/names. All installed
name prefixes are ASCII and NUL-terminated; maximum length is 39. This does
not establish an ASCII-only admission rule or permission to discard padding.

## API and policies

[assets/evt.hpp](../../assets/evt.hpp) exposes `EvtArea`, `EvtAsset`, `EvtLimits`,
`EvtResult`, `decodeEvt(bytes)`, `loadEvt(file)` and `evtAreaName(area)` through
`mnm-evt-loader`. Public types use the standard library and native read-only
asset access, without widgets or build-specific reconstruction dependencies.
Results own their ordered records, header metadata and source byte count.

Native policies, separate from original failure behavior:

- Validate the signature and require version one. Preserve arbitrary size words.
- Require exact record extent; reject truncation and trailing data. Compute
  extents in 64 bits before allocation.
- Defaults: 4 MiB input, 65,536 records and 4 MiB decoded record storage.
  Charge `sizeof(EvtArea)` per record; container overhead is outside that budget.
- Preserve signed 32-bit coordinate values and all 48 name bytes. No world
  coordinate bounds, endpoint normalization or duplicate removal is imposed.
- `evtAreaName` returns the byte prefix before the first NUL, bounded to 48
  bytes. A full unterminated field and an empty prefix are accepted. No text
  encoding is imposed by the parser.
- Errors carry code, offset and detail, with backend input errors retained.
  Input limits beyond the signed file API range return invalidArgument;
  allocation/length failures return limitExceeded. No partial asset is returned.

The JSON inspector closes the file before printing header metadata, both
endpoints, bounded name and complete `name_bytes_hex`. Its display string maps
bytes through Latin-1, a reversible inspector policy rather than a claim about
the game's encoding. It uses AssetStore containment/case folding, exits 2 on
failure and requires no game process or display server.

## Reproduction and validation

```bash
cmake -S assets -B working/build/evt -DBUILD_TESTING=ON
cmake --build working/build/evt --parallel 4
ctest --test-dir working/build/evt --output-on-failure
working/build/evt/mnm-evt-inspect working/game-clean Realms/Celtic/Forest/CFsec01.evt
python3 tests/test-evt-loader.py working/build/evt/mnm-evt-inspect \
    --installation working/game-clean \
    --report working/tests/evt-loader/installed-comparison.json
python3 tools/export-evt-support.py --decompile
```

Native fixtures cover extreme signed endpoints, reversed ordering, retained
nonzero name tails, full unterminated names, empty names/lists, owned storage,
arbitrary size metadata, every truncation of a valid file, extra bytes, bad
version/signature and input/count/allocation limits. Permissive budgets with
a huge count still fail extent validation before allocation.
Four independent Python `struct` fixtures compare every field and raw name
byte, including repeated labels, high bytes, case folding, missing files and
parent traversal. The installed run records source hashes and verifies
immutable originals before/after, including failures.

All 21 asset CTests and both EVT ASan/UBSan tests pass. Leak detection is
disabled because tracing prevents LeakSanitizer; prebuilt Qt is uninstrumented.
All 685 installed inputs and 2,020 complete records match independent decoding.
All 2,927 immutable originals verify. Comparison report:
`working/tests/evt-loader/installed-comparison.json`; ordered canonical record
SHA-256: `b4648f9ceb6c5c6e60385bb0b720002a8f32b141429e2d951ce105dfc81e298d`. Tested inspector SHA-256: `f6d4912cf2449321a62e6d5a360b8bb77ea9aea6fb134eebed5d5a16ec09e703`
(GCC 16.2.1, C++17, Qt 6.11.2).

Remaining work: recover name lookup and script binding, validate transforms,
containment and trigger actions, then integrate native world consumers and
bounded live loading. No native writer or trigger execution is supplied.
See [coverage ledger](../runtime/coverage-ledger.md).
