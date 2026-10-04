# EVT event-area reader and selected callers

Reviewed 2026-10-04. Static No-CD evidence, installed byte inventory and native
offline tests; no original execution, observation hook or live replacement.
Executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses apply only to this build; heap addresses remain launch-specific.

## Reproduction and confidence

```bash
python3 tools/export-evt-support.py
python3 tools/export-evt-support.py --decompile \
    --project working/decompiled/nocd-96ofmeq_/project
```

The wrapper checks the executable hash, exports selected objdump ranges and
inventories installed files. Optional Ghidra processing uses the existing
analyzed project with `-readOnly -noanalysis` and
[ExportEvtSupport.java](../../tools/ghidra/ExportEvtSupport.java), which checks
the project executable hash. Every requested pseudocode export must exist.
Original manifest checks run before/after, including failure; executable and
installed source hashes are checked again before writing the report. Output
is a fresh `working/decompiled/evt-support-*` directory. No binary is patched.

The reviewed run is `working/decompiled/evt-support-ak8co0fh`; its report hash
is `ba13059cf802828931f8693442994587d7a3e6e2d0a7e835df8b4f3898715471`. All 685 installed files total 156,400 bytes and 2,020 records.
Confidence is high for the static reader/writer layout and selected coordinate
consumers. Full event-name lookup, trigger execution and runtime equivalence
remain unverified. The human-readable name interpretation follows installed
48-byte record tails; the original text encoding and name-validation rules
are not recovered.

## Reader, writer and object fields

| Entry/address | Confirmed selected behavior |
| --- | --- |
| `0x0049ee50` | Reset: frees object `+0x10` storage and clears pointer/count |
| `0x0049ee80` | EVT reader: ECX object, stack path, success flag, `ret 4` |
| `0x0049ef09` | Reads the 16-byte header through `0x004a1760` |
| `0x0049ef12` | Requires header version one |
| `0x0049ef82`–`0x0049ef9d` | Computes count times 72 and compares actual file length with that plus 16 |
| `0x0049efac`, `0x0049efc8` | Allocates and reads the entire 72-byte record array |
| `0x0049efe0`–`0x0049eff9` | Stores pointer/count and four header words |
| `0x0049f020` | Append: coordinate duplicate checks, then copies all 18 words of each record |
| `0x0049f100` | Writer: writes `EVT\0`, size word `(count + 1) * 16`, version one and count, followed by count times 72 bytes |
| `0x004eda90`, `0x004ee0ba` | Section-loading caller constructs `.evt` path and calls reader |
| `0x004ef2c0` | Transforms the two coordinate triples, reorders endpoints, wraps x/y and appends areas |

The original reader checks version and actual extent, but does not validate
magic or use the size word for admission. Native signature validation is an
added policy. The size-word formula is confirmed directly in the original
writer (`0x0049f185`–`0x0049f1ae`), rather than merely inferred from the corpus.
Do not use it to trim the input or infer 16-byte records.

The PE32 runtime object contains the four header words at offsets 0–15,
record pointer at `+0x10` and runtime count at `+0x14`. These offsets are not a
native/shared ABI. The native loader owns a vector instead. Original rollback,
error boxes, signed count arithmetic and allocation failure behavior are not
replicated or claimed equivalent.

## Area and coordinate evidence

The original version error string at `0x005e03b8` calls these “event areas”.
The transformation caller logs point-one x/y/z using `0x005e48d0` and point-two
x/y/z using `0x005e48b0`, consuming the first six words. It advances 72 bytes.
It rotates/translates x/y, conditionally skips second-point transformation
when its x or y equals -1, reorders both endpoints on each axis, wraps x/y,
and appends into the destination list. These are selected static observations,
not a complete independently tested world transformation contract.

Append compares the proposed first coordinate triple against either endpoint
of existing areas and displays a duplicate warning on a match. The input
reader itself does not deduplicate. Native loading preserves original file
order, both endpoints and all 48 remaining bytes without applying these rules.

The installed corpus includes reversed endpoint ordering in 311 records and
nonzero bytes after the first name terminator in 60 records. Neither is evidence
of malformed input. Names include `CF01 Hermes Outside Hut` and
`GV62 Main Door To Arena`; all installed prefixes are NUL-terminated ASCII,
with maximum length 39. Exact name matching, linkage to scripts, trigger
containment and execution require further recovery and runtime validation.

See [native schema and validation](../formats/evt-native-loading.md) and
[coverage ledger](coverage-ledger.md). No writer, transform, trigger runner or
live loading adapter is implemented by this work.
