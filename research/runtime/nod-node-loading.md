# NOD node reader and selected connection lookup

Reviewed 2026-10-04. Static evidence from the No-CD executable with SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
High confidence for the selected record reads/copies and local transforms;
complete graph lifecycle, connection semantics and search consumption remain
unvalidated. No live replacement or gameplay changes.

## Reproducer

```bash
python3 tools/export-nod-support.py --decompile
```

The exporter verifies the original manifest before/after, checks executable
hash, inventories all installed NOD files, rechecks their source hashes and
saves selected disassembly. Optional Ghidra decompilation runs read-only;
function creation is discarded with that analysis session. Recorded evidence:
`working/decompiled/nod-support-cvgwjra7/`; report/artifact hashes are retained
in [NOD evidence](../formats/nod-native-loading.json).

| Address | Selected role |
| --- | --- |
| `0x0053eaa0` | NOD file reader and section-local node installation |
| `0x0053ea50` | Search 18 slots for a nonzero value and matching target |
| `0x004a1760` | File wrapper exact byte read; returns boolean |
| `0x004ee572` | Map-section caller, receiver `0x006eaf08` |
| `0x0053faf0` | Referenced section registration; not fully recovered here |

## File reads and runtime copies

The reader opens the path through `0x004a1360`, reads exactly 16 header bytes
through `0x004a1760`, and uses header DWORD +12 as count. `0x004a1590(0)` returns
actual file size; expected size is `count × 0x1ee + 0x18`. The selected routine
does not compare the signature, version or stored size word. Native validation
adds those checks; [format documentation](../formats/nod-native-loading.md)
separates them from original behavior.

For each node it reads 494 bytes into a stack buffer. The runtime node stride
is `0x294` (660), not the packed file stride. The selected copies are:

| File record | Runtime node | Role |
| ---: | ---: | --- |
| `+0` | `+0` | State |
| `+8`, `+12`, `+16` | `+4`, `+8`, `+12` | x/y/z |
| `+20 + i×25`, i=0..17 | `+16 + i×28` | 24 copied bytes plus one byte per connection |
| `+478` | `+524` | One opaque tail DWORD |

The other packed words are not copied by this selected loop. The routine never
reads the eight bytes after the counted records; installed files put `1000`
and count there. That correlation is recorded, not assigned a semantic role.

The receiver has a node-storage pointer at +0, capacity at +4 and append/base
count at +8. Loading requires `base + fileCount < capacity`, so one spare slot
is required by the original check. After copying, it references section
registration, transforms x/y for modes 0–3, translates them by section position
and wraps against global dimensions `0x006c5494/98`. It adds the previous node
base to targets whose slot value is nonzero, then advances the append count.
Its transform/rebase walk tests node state rather than counting iterations;
full sentinel initialization and state lifecycle are not recovered here.

The lookup at `0x0053ea50` computes runtime node `storage + index × 660`, scans
18 slots with a 28-byte stride, selects `value != 0 && target == requested`,
and returns the first matching value or zero. This establishes connection
selection and ordinal use, but not the full meaning of the returned word.

Native loading preserves local values and raw metadata. Runtime allocation,
node sentinels, section registration/transforms, graph joining, interpretation
of connection metadata and complete pathfinding behavior remain separate.
