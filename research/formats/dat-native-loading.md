# Native DAT AI input

Reviewed 2026-10-04. Two headerless little-endian DWORD grammars account for
all installed bytes. Confidence is high for the stored layout: hash-pinned
No-CD reader disassembly and independent native/reference byte roundtrips
agree. Readers were inspected statically, not executed. Parameter meanings,
training and inference behavior remain unresolved.

## Brain.dat

Repeat the following model record until EOF. There is no magic, version,
global count or trailer:

| Field | Stored extent |
| --- | --- |
| Model key | DWORD |
| Dimension | DWORD |
| Layer count | DWORD |
| Layers | Ordered layer records, count from preceding field |

Each layer contains a scalar DWORD, a node-count DWORD, `node_count` records
of 12 DWORDs each, a tail DWORD, and `node_count * node_count` DWORDs in row
order. Layer extent is `12 + 48*n + 4*n*n` bytes. Field names describe position,
not recovered semantics. Scalar, state and matrix values retain exact bits;
no float conversion, finite-value validation or normalization occurs.

`AI/Brain.dat` contains seven models and 21 layers. In file order, keys are
6, 8, 9, 11, 12, 13 and 14; dimensions are 72, 73, 71, 71, 71, 73 and 73.
Each installed model has three layers whose node counts equal its dimension.
This relationship is an observation, not a native parser restriction.
There are 1,512 node records and 108,882 matrix words in 508,440 bytes.

## Experien.dat

Repeat `key DWORD`, `value_count DWORD`, `value_count` payload DWORDs,
then two parameter DWORDs until EOF. Extent is `16 + 4*value_count` bytes.
The payload and parameters retain exact DWORD bits. Signedness and semantic
names are not inferred. Order and repeated keys are preserved.

`AI/Experien.dat` contains 2,500 records and 180,251 payload words in
761,004 bytes. The installed key/count combinations correspond to model
keys/dimensions in Brain.dat; the APIs do not require the companion file.
Per-key counts and parameter distributions are retained in the JSON report.
Neither filename's first word identifies a format version.

## Native API and limits

`assets/dat.hpp` exposes `decodeBrainDat`/`loadBrainDat` and
`decodeExperienceDat`/`loadExperienceDat` through `mnm-dat-loader`.
Results own their vectors; the byte decoder is independent of Qt and recovered
engine code. File loading uses the existing read-only AssetFile boundary and
preserves backend error details. The inspector emits all words as unsigned
JSON integers, after closing the source file:

```sh
working/build/dat/mnm-dat-inspect brain working/game-clean AI/Brain.dat
working/build/dat/mnm-dat-inspect experience working/game-clean AI/Experien.dat
```

Default native limits are 32 MiB input, 64 MiB decoded payload/record metadata,
100,000 records, 64 layers per model, 512 nodes per layer and 65,536 values per
experience record. Multiplication uses 64-bit arithmetic; counts, available
extents and aggregate decoded budgets are checked before their allocations.
Checked matrix multiplication also rejects oversized products when caller limits
are raised. Vector spare capacity and allocator overhead are not a process-wide memory cap.
Allocation failures become structured errors. Limits are native policy, not
claims about original supported maxima.

An empty stream is accepted as an empty catalog. Incomplete records and stray
trailing bytes are rejected. With no global count/checksum, loss or addition of
whole valid records cannot be detected. The caller selects the schema explicitly;
there is no reliable general `.dat` auto-detection contract.

## Reproduction and validation

```sh
cmake -S assets -B working/build/dat -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/dat -j4
ctest --test-dir working/build/dat --output-on-failure
python3 tests/test-dat-loader.py working/build/dat/mnm-dat-inspect \
  --installation working/game-clean --report working/dat-comparison.json
python3 tools/export-dat-support.py --decompile
```

The independent Python parser compares every model, layer, state, matrix,
experience value and parameter with native JSON, then reconstructs every source
byte. Seven synthetic comparisons additionally cover empty catalogs, zero-sized
layers/vectors, duplicate keys, dimension differences and unusual numeric bits.
C++ checks exercise ownership, every partial-record prefix, limits and exact
bit preservation. Input/path failures are exercised through the inspector.
All 35 asset CTests and both DAT ASan/UBSan tests passed. LeakSanitizer cannot
run under the sandbox tracing environment, so `ASAN_OPTIONS=detect_leaks=0`
was used for sanitizer verification. The initial leak-enabled run failed for
that environment reason, not a reported DAT allocation leak.

Corpus comparisons and reader exports verify the immutable original manifest
before and after, and verify unchanged working inputs.

Retained corpus identities, decoded hashes, counts, native executable hashes
and static evidence identities are in [dat-native-loading.json](dat-native-loading.json).
Original runtime reader contracts and pseudocode limitations are documented in
[DAT AI loading](../runtime/dat-ai-loading.md). This milestone supplies offline
file input; it does not implement model execution, training, save updates,
original error handling, WBT execution or live replacement.
