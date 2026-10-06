# Effect animation selection

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Whole recovered selector

Read-only thiscall `00489200` reads type at effect `+28`, kind at `+4c` and,
only for one branch, a creature ordinal at `+48`. It returns a raw animation
ordinal or `ffffffff`. No helper calls, writes, allocation or animation attachment
occur. The native function accepts these raw words and owned kind/creature
metadata arrays; it consults only the data used by the selected original branch.

Types 6 and 7 have a first-priority override unless kind is 68. Kind33 returns
27 directly. Every other override reads metadata DWORD
`006b13cd + kind * 721`: class0 returns20, class1 returns21, class2 returns13,
and all other values return `ffffffff`. This override precedes every general
kind branch, including the creature lookup. Kind68 always selects87.

General dispatch admits kinds 0..102 by unsigned comparison at `00489266`.
A 103-byte map at `00489490` chooses one of 30 pointers at `00489418`;
other raw kinds return `ffffffff` without reading metadata. The exact map and
all 30 pointers are asserted against original private mapped bytes by the
comparison helper. Zero bytes elided in the existing disassembly were confirmed
by this exact-byte assertion, not assumed in the final evidence.

| General kind(s) | Result or selection |
| --- | --- |
| 0,1,3,5,10..16,19..26 | Class0/1/2 selects15/16/14; other class returns `ffffffff` |
| 2,4,6..9,17,18 | Class0/1/2 selects18/19/17; other class returns `ffffffff` |
| 33 | 43 |
| 34 | Type35 selects55; type36 selects56; every other type selects57 |
| 35,38,41,42,43,44 | 63,51,74,73,48,71 respectively |
| 50 | 30 |
| 52 | Creature descriptor class2 selects17; every other raw class selects14 |
| 54,58 | 75,59 respectively |
| 62 | Type5 selects0; every other type selects59 |
| 68,71,73,74,77,79,82 | 87,0,49,34,28,70,69 respectively |
| 89 | Type2 selects0; every other type selects59 |
| 93,98 | 2 |
| 94,95,96,99,102 | 3,4,5,11,78 respectively |
| All other kinds | `ffffffff` |

The class-dependent general branches read the same kind metadata DWORD as the
override. Kind52 resolves the raw `+48` ordinal against creature count
`006def5c`, uses base `006def58` with stride `e4b`, follows creature `+ac` to a
descriptor and reads descriptor `+8`. It does not translate a creature ID into
an ordinal or inspect status/occupancy. A descriptor word of2 returns17; the
original subtraction/negate/SBB/low-byte-mask sequence returns14 for every other
word, including negative-looking values. Invalid original ordinals produce a
null pointer which is subsequently dereferenced; no valid fallback is inferred.

## Evidence, ownership and remaining scope

[TL21 comparison](effect-animation-selection.json) executes whole unmodified
`00489200` in a private PE32 mapping. No child callbacks, stubs or patches are
used. Original entry and the complete 30-pointer/103-byte dispatch data are
checked. All 558 effect receiver bytes plus external guards remain unchanged
for every call. Creature/descriptor allocations and each seeded metadata DWORD
are checked unchanged after every metadata/descriptor group.

The matrix covers 96 raw types (0..88 plus boundary/unsigned-limit words),
108 raw kinds (0..102 plus boundary/unsigned-limit words), seven metadata classes,
seven descriptor classes and three valid creature ordinals. The 1,522,626 calls
include 29,988 overrides, 13,818 creature lookups, 962,514 `ffffffff` results
and 36 distinct animation results. For types6/7, out-of-table kinds are excluded
from original execution: that branch would read unchecked metadata instead of
using the general fallback. Native units exercise missing required metadata and
invalid/missing creature descriptor refusals instead.

Original PE32, native64 and ASan/UBSan result streams match. Explicit units check
override precedence, kind33/68 exceptions, type-dependent branches, descriptor
selection, metadata-class fallback and lazy storage access. Strict normal and
sanitized units and standalone CMake/CTest pass. Executable/source fingerprints
are stable and all 2,927 immutable originals verify before/after the run.

```sh
python3 tools/test-effect-animation-selection.py
```

Confidence: high within the owned 103-kind metadata and valid required creature
reference contract. Native bounds checks throw only when a selected branch needs
a missing owned entry; these are ownership policies, not recovered original
error handling. Raw metadata provenance remains authored. Creation dispatch
calls this selector, for example at `00494bde` after the separately recovered
[0048a950 motion initializer](effect-motion-initializer.md), then uses its result
to choose animation metadata. That caller relationship is static evidence, not
whole-dispatch execution or animation asset binding. The 89-entry creation
setup dispatch remains incomplete; its original common-placement comparison
still defers the whole child. Installed metadata loading, animation asset/frame/
direction/start-state binding, child effects, creator ownership and full creation
composition remain pending, alongside live scheduling/replacement. Historical
motion, trajectory and placement evidence retain their recorded fingerprints.
