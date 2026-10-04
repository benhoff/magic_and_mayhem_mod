# MPS placement reader and selected callers

Reviewed 2026-10-04. Static No-CD evidence only; no original execution,
observation hook, native placement simulation or live replacement.
Executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses below apply only to this build. Heap addresses remain launch-specific.

## Reproduction

```bash
python3 tools/export-mps-support.py
python3 tools/export-mps-support.py --decompile \
    --project working/decompiled/nocd-96ofmeq_/project
```

The first command exports hash-pinned selected objdump ranges and an inventory
of installed inputs. The second additionally uses an existing analyzed Ghidra
project with `-readOnly -noanalysis` and the hash-checking
[ExportMpsSupport.java](../../tools/ghidra/ExportMpsSupport.java) script.
The project path may be replaced with another analyzed `MagicMayhem` project
for the same executable hash. No automatic executable patch/import is performed.
The wrapper verifies original media before/after, rehashes the executable and
installed inputs, and records script/input/artifact hashes. Output is a fresh
`working/decompiled/mps-support-*` directory.

The reviewed run is `working/decompiled/mps-support-67xgd3jk`: selected assembly,
Ghidra pseudocode/references, per-file inventory and `report.json`.
The reviewed report SHA-256 is
`321a771d44c33ff1c3b84d5527bb747535553f5cb3166c6a9ebe16ff00b8b518`.
All 683 files total 389,488 bytes and 9,464 records. Immutable manifest checks
pass for all 2,927 source files. Confidence is high for the selected static
control flow and byte layout; complete placement parameter semantics and live
caller behavior remain unverified.

## Reader contract

| Entry/address | Confirmed selected behavior |
| --- | --- |
| `0x00532f00` | Reset: frees object `+0x10` record storage, clears pointer and `+0x14` count |
| `0x00532f30` | MPS file reader, thiscall: ECX object, stack path, returns success flag, `ret 4` |
| `0x00533079` | Computes count times 40 and requires actual file size equal to that plus 16 |
| `0x005330ad` | Allocates count times 40 bytes and reads the complete record block |
| `0x005330e5` | Copies four header words and stores decoded pointer/count |
| `0x00533250` | Append: compares x/y/z/kind for duplicates, copies full ten-word records |
| `0x004eda90` | Section-loading caller containing the `.mps` path construction and load call |
| `0x004edfcb` | Calls reader with the stack-local MPS object after replacing the section extension |
| `0x004eefc0` | Mutates local x/y coordinates for section rotation/translation/wrapping, appends placements to a destination list |
| `0x004ef5e0` | Checks deployment cells using `z * layerStride + y * rowStride + x`, advancing 40 bytes per record |

The reader reads 16 header bytes. It checks version (word at file offset 8)
equals one and actual file length equals `16 + count * 40`. It does not compare
the signature or use the header size word to bound the record block. Native
signature validation is therefore an added policy. Installed size words all
follow `16 + count * 16`; this correlation is confirmed, but its historical
origin is not established. Do not reinterpret the record size as 16 or trim
the file to that word.

The runtime object stores the four header words at offsets 0–15, record pointer
at `+0x10`, and count at `+0x14`. These are PE32 runtime fields, not a shared
host/native object layout. The parser owns its vector instead.
The original replaces the old allocation on successful load and has rollback/
error-box paths; malformed-input, allocation-failure and retry equivalence are
not claimed by the native loader.

## Kind names and coordinate evidence

The debug switch reads record offset 12 and names values:
0 Undefined, 1 Friendly Wizard, 2 Enemy Wizard, 3 Multiplayer Wizard,
4 Creature, 5 Artifact. Jump-table entries at `0x0053322c` target the debug
cases at `0x0053316c` through `0x005331bc`; strings include
`0x005ea6f8` (Friendly Wizard), `0x005ea6c8` (Creature) and
`0x005ea6bc` (Artifact). The loop steps 40 bytes.

Installed kind 6 is common (3,301 records), but this debug switch has no named
case for it. Its meaning remains unknown. Values outside the named set are
preserved by the native API. No substitute type is assigned.

The cell checker confirms first three words as x/y/z. The transformation caller
modifies x/y and retains z, performs selected terrain-dependent rejection and
adds records to an accumulated list; one branch checks kind 4 and several values
of the next word. That establishes kind-dependent parameter use, without
establishing a complete ID catalog or the meanings of all six remaining words.
The native reader deliberately does not perform these transforms or append/
deduplicate records: it returns the original section data in file order.

## Remaining boundaries

Recover kind 6 and all per-kind parameter meanings, catalog IDs/defaults,
complete rotation and terrain admission semantics, placement selection and
entity creation before native world application. Also validate original
runtime inputs/results in bounded scenarios. The offline decoder does not
advance those gameplay/runtime milestones.
See [on-disk layout and native validation](../formats/mps-native-loading.md)
and the [coverage ledger](coverage-ledger.md).
