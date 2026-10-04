# Save game envelope and block inventory

Reviewed 2026-10-04. Static findings for the No-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
preferred image base `0x00400000`. Method: paired reader/writer disassembly and
read-only Ghidra pseudocode. High confidence for the sizes, order and constants
below; full semantics and real-file agreement remain unvalidated.

Reproduce with `python3 tools/export-persistence-progression.py` between
original-manifest checks. See [runtime evidence](../runtime/persistence-progression.md)
for caller, commit, restoration and progression boundaries.

## Two layers

Named files are `Save\\<stem>.sav`. The temporary `Save\\__temp.vas` is an
unpacked stream used by both loading and writing. Its extension does not mean
that final saves use `.vas`.

The final writer calls `0x004a17c0` -> `0x004a1a20`, selecting a packed output
and applying `0x004a1c00`; the reader uses `0x004a1820` -> `0x004a18b0`.
Static header fields follow the same layout as the separately recovered
[encrypted CFG container](encrypted-cfg.md):

| Outer offset, after undoing obfuscation | Bytes | Field |
| --- | ---: | --- |
| `0x00` | 4 | Seed; selected encode branch uses `GetTickCount() XOR 0x5a2387c3` |
| `0x04` | 4 | Decoded length |
| `0x08` | 4 | Packed-payload checksum |
| `0x0c` | 4 | Decoded checksum |
| `0x10` | 4 | Packing mode, branches 0/1/2 |
| `0x14` | Variable | Payload |

The encoder compares raw versus two packing candidates (`0x004a1ce0`,
`0x004a28d0`). Reader mode 0 copies bytes; modes 1/2 dispatch separate unpackers.
Checksums alternate XOR/add over complete DWORDs. The transform leaves the seed
and XORs subsequent words using the generator `0x0054e170` / `0x0054e200`.

This is static structural correspondence, **not** validation that the existing
CFG decoder accepts every save produced by this build. The native reader now selects a separate No-CD transform: at `0x004a1c99`
through `0x004a1ca9`, the remainder loop does **not** advance ESI. It XORs the
first trailing byte once per generated value while leaving the remaining one or
two bytes untouched. The clean CFG model advances the byte position. Static
evidence is high confidence; no original-executed save comparison is claimed.
Malformed-input behavior and real save compatibility remain unvalidated.

## Decoded stream header

All numeric fields below are little-endian for this PE32 build.

| Offset | Bytes | Evidence |
| --- | ---: | --- |
| `0x00` | 4 | Writer copies `SAV\0` from `0x005e4164` |
| `0x04` | 4 | Version `0x14` (20); reader explicitly requires this |
| `0x08` | 4 | Writer later overwrites with an accumulated byte-accounting value |

The inspected reader does not compare the magic or accounting value. Do not
call the latter an enforced file length or checksum. The wizard writer returns an accounting constant 19 bytes smaller than its
fixed physical write sequence per record, so that value must not be used to
locate the end of the stream. Real-file agreement remains unvalidated.

## Ordered block inventory

Offsets after wizard records are symbolic because those records call nested
serializers. Let `W` be the total bytes written by the 80 wizard serializers.

| Decoded offset / sequence | Size | Source/destination |
| --- | ---: | --- |
| `0x0c` | `0x200` | Path/config block at `0x006ddd58` |
| `0x20c` | `W` | 80 wizard records, `0x00594290` / `0x005942d0` |
| `0x20c + W` | `0x13dc` | Realm state at `0x00659e51`, raw block |
| Next | `0x16c` | Controller arrays: 3 + 63 + 25 DWORDs |
| Next | `0x18` | Global block at `0x00689910` |
| Next | `0x3268` | Script-related state: 200 * `0x40`, then `0x3c`, then `0x2c` |
| Next | 4 | Global `0x006e2034` |
| Next | 4 | Global `0x006e2038` |
| Next (`0x49dc + W`) | 4 | World-section marker |
| If marker == `0x17` | `0x18` + variable | Global block `0x00656620`, then world serializer |

Without the optional world tail, the static block sum is `0x49e0 + W`, including
the 12-byte decoded header. This is arithmetic from call sizes, not a measured
save length. Writer sets marker `0x17` when world flag `0x006c54dc` is nonzero;
otherwise writes zero. Reader only takes the world-tail branch for `0x17` and
sets a later load-mode field to 1 for that branch, 2 otherwise.

## Packed realm fields

The raw realm block's offsets are described in the
[runtime field table](../runtime/persistence-progression.md#realm-initialization-and-progression).
Its base `0x00659e51` is unaligned. The original raw bytes are an original-build
contract, not a native C++ layout to share over channels or persist with `memcpy`.

Wizard records have the physical sequence below; field semantics remain mostly
unnamed. World records mix primitive fields, arrays, nested state and reference
conversion. Their physical byte grammar is now available in [world structural loading](save-world-native-loading.md); enum values, ownership and resource
recreation remain unresolved. Script records are a fixed-size byte block here;
individual record/variable/trigger semantics are not yet recovered.

## Variable wizard records

Paired record writer/reader `0x00592e90` / `0x00593560`, entry writer/reader
`0x005906d0` / `0x00590800`, and list reader `0x005904c0` establish this order
(high confidence for physical sizes, not gameplay meanings):

| Sequence | Bytes |
| --- | ---: |
| Contiguous primitive/string/array prefix | 0x14a |
| Entry count | 4 |
| Entries (five DWORDs each) | count * 20 |
| Ten fixed records | 10 * 100 |
| Two unnamed DWORDs and list count | 12 |
| DWORD list | listCount * 4 |
| Extension presence byte | 1 |
| If present, raw extension | 0x57c |

The host entry stride is 24 bytes; the disk omits its final DWORD. Initialization
at `0x00590930` explicitly sets ECX to 0x2a before clearing those 42 records.
The native bound of 42 follows that fixed backing storage. Presence is emitted
as 0/1; the original reader treats any nonzero value as present, while the native
reader rejects other values. Primitive prefix and both fixed record blocks are
preserved raw. The list becomes owned DWORD values; its host allocation/pointer
is not serialized or restored.

Minimum physical size is `0x543` per wizard (330 + 4 + 1000 + 12 + 1).
Thus `W = 80 * 0x543 + sum(20 * entryCount + 4 * listCount + extensionBytes)`.
The writer's returned accounting uses 0x51f instead of the 0x532 prefix-plus-record
physical bytes; this explains a 19-byte discrepancy per wizard. Do not parse
using its return-value accounting. Names and all five realm DWORD arrays each
have 80 slots, followed by the 20 owner slots; see the corrected runtime table.

## Validation boundary

[Native offline readers](persistence-native-loading.md) now load the container,
all decoded campaign blocks and optional opaque world data. Synthetic save
fixtures verify boundaries, nested records and both branches; 36 CFG and 36
No-CD save-container fixtures independently exercise the transforms. Eleven
installed CFG containers agree with the existing Python decoder. These do not
prove original save compatibility.

No live save/load was performed and no actual original-generated `.sav` was
decoded. Collect campaign-only and active-battle saves, compare original/native
decoding before attempting a compatible writer. Explicit world structural readers now
cover the recovered block grammar with selected original-writer capture evidence;
see [world validation](save-world-native-loading.md).
