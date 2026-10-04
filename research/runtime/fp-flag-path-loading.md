# FP flag-path reader and Realm Viewer caller

Reviewed 2026-10-04. Static No-CD evidence and native offline comparisons;
no original execution, live observation, native flag behavior or replacement.
Executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses apply only to this build; heap addresses remain launch-specific.

## Reproduction and confidence

```bash
python3 tools/export-fp-support.py
python3 tools/export-fp-support.py --decompile \
    --project working/decompiled/nocd-96ofmeq_/project
```

The wrapper exports hash-pinned selected assembly and the installed inventory.
Optional Ghidra uses the existing project with `-readOnly -noanalysis` and
[ExportFpSupport.java](../../tools/ghidra/ExportFpSupport.java), which checks
the executable hash. Missing function definitions are disassembled/created in
the temporary in-memory program; read-only project processing discards those
changes. No persistent project or executable modifications are made.
All requested pseudocode exports must exist. Original manifests verify
before/after; the executable and all installed inputs are rehashed. Reports
record wrapper/Java script hashes and selected artifact/input hashes.

Reviewed output: `working/decompiled/fp-support-2sagykgr`, report SHA-256
`df7fce9659afdfd375fadbb13fdb8c8098567b03c6372ca60b6dfee14145192d`. The inventory covers 41 files, 132,356 bytes, 79 paths and
15,950 points. Confidence is high for selected static control flow/layout;
complete UI behavior and live caller agreement remain unverified.

## Confirmed entries and fields

| Address | Selected confirmed behavior |
| --- | --- |
| `0x004a2a60` | Constructs a 0x8c-byte FP object: `.FP`, version two, zeroed arrays/state |
| `0x004a2ae0` | Reader: ECX object and stack path; returns success, `ret 4` |
| `0x004a2b0a` | Reads one 84-byte header block through CRT read wrapper `0x0059c359` |
| `0x004a2b1b`–`0x004a2b57` | Compares `.FP` string at `0x005c6278` and requires version two |
| `0x004a2b66` | Reads four eight-byte position elements into object `+0x54` |
| `0x004a2bb2`–`0x004a2bc0` | Sums active path counts at `+0x14`, allocates sum times eight bytes |
| `0x004a2bfc` | Reads point payload in eight-byte elements; requires the expected element count |
| `0x004a2c40` | Selects an available one of four position slots and marks it busy; no slot yields (-1,-1) |
| `0x004a2cb0` | Matches a supplied coordinate pair against those four positions and clears its busy flag |
| `0x004a2cf0` | Last-point getter: uses stored path offset plus count minus one |
| `0x004a2d30` | Point getter: uses stored path offset plus supplied point index |
| `0x0054ef50`, `0x0054f010` | Realm Viewer collection loader and call to FP reader |

The original object begins with the 84-byte header, followed by four x/y pairs
at `+0x54`. Busy flags occupy `+0x74..+0x83`, point-array pointer is `+0x84`
and another runtime counter is `+0x88`. Only the first 116 bytes are on disk;
busy flags, pointer and counter are runtime state, not serialized fields.
The native asset owns a vector and does not copy the PE32 object layout.

Active path count is at `+0x10`. Eight point counts start at `+0x14` and eight
point offsets start at `+0x34`. Both getters demonstrate that offsets are point
indices rather than byte offsets, and that each payload element is x/y.
The header pair at `+8/+12` is retained without assigning a recovered UI role.

## Caller and native boundaries

The Realm Viewer collection loader allocates 0x8c-byte objects, calls the FP
constructor and formats `Interface\RealmViewer\Generic\%s_FlagPath_%02i.FP`
using a supplied realm name and ordinal. It calls the reader at `0x0054f010`.
This identifies Realm Viewer flag paths, rather than footprints, world geometry
or pathfinding search data. Original selection/movement/scaling/drawing and
complete failure behavior are outside this static input milestone.

The reader frees an old point allocation before allocating the new payload.
It checks the signature string, version and element read counts, but does not
safely bound path count to eight, validate stored ranges or reject trailing
bytes. Native exact-extent, capacity, 64-bit count/range and allocation checks
are intentional safety policies. The original pointer mutation/error behavior
is not reproduced. Getter behavior on malformed indices or empty paths is
not promoted into a native API contract.

Installed active offsets are prefix sums and unused slots are zero. Native
loading preserves arbitrary unused words and valid noncanonical active ranges;
it does not rebuild offsets or infer gameplay rules from coordinates.
No slot reservation, randomized selection, flag movement or drawing is performed.

See [on-disk schema and native validation](../formats/fp-native-loading.md)
and [coverage ledger](coverage-ledger.md). Offline parse agreement does not
establish original live loader or Realm Viewer equivalence.
