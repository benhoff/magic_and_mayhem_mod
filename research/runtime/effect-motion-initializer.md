# Effect motion record initialization

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Whole helper contract

Thiscall `0048a950` (`ret 0x18`) receives a valid effect receiver and six raw
fine-unit words: source X/Y/Z followed by destination X/Y/Z. It is used within
the type setup dispatcher `00494ab0`, including calls at `00494bd7`, `00494c8c`,
`0049626a` and `00496e2d`; a separate caller at `0044a912` operates on a temporary
record and subsequently requests membership-disabled movement. These caller
mappings are static evidence. Full dispatch and caller execution remain pending.
The previously noted `004891e0` belongs to ballistic update `00489080`, not the
initial creation helper. Its floating-point destination/angle production remains
separate from this integer setup contract.

The helper resets exactly nine DWORD caches to `ffffffff`: record `+1aa/+1ae/
+1b2`, `+1b6/+1ba/+1be` and recount `+1c2/+1c6/+1ca`. It zeroes all 14 trajectory
words through whole original `004df230` at `0048a996`, then initializes them
through [004df3a0](effect-trajectory-initializer.md) at `0048a9df` with the six
arguments, counter `+1ce`, multiplier 32, and X/Y map dimension globals
`006c5494/006c5498`. Consequently word3 starts at zero, unlike the standalone
initializer which preserves it. Counter becomes zero.

Source words are stored both at fine-unit `+14/+18/+1c` and startup
`+1de/+1e2/+1e6`. The latter are distinct from movement's previous fine units
`+1ea/+1ee/+1f2`; those previous units are unchanged. The source logical shifts
by 5/5/4 produce cell coordinates `+8/+c/+10`. Source coordinates are not clamped
or wrapped by this helper. Destination words are not copied into record parameters;
only the trajectory consumes them.

Y/Z lookup globals `006cb942` / `006cb8c2` contribute authored row/plane offsets;
the helper selects a 12-byte cell at global `006c54dc`. It publishes that cell
pointer at `+194`, reads its terrain WORD at `+0`, and publishes the terrain
catalog pointer (`0065660c + ordinal * 356`) at `+190`. No catalog data is read.
Native setup uses the existing row-major owned cell representation and stores
cell/terrain ordinals, retaining all raw 16-bit terrain ordinals, including 65535.
Nonstandard row/plane tables are outside this native representation.

The native interface initializes an existing `EffectPlacementRecord`,
`EffectTransitionState` and separately owned startup words. Parameters, activity,
type, chain link, initial-position caches `+1f6/+1fa/+1fe`, previous-position
caches `+202/+206/+20a`, the last three placement sentinels and other fields stay
unchanged. It does not insert/remove cell membership, change cell flags, create
an active record, allocate a slot or replace type/animation setup. Callers must
coordinate this setup with record placement and membership lifecycle separately.

## Validation and native policy

[TL20 comparison](effect-motion-initializer.json) executes whole unmodified
`0048a950` and its real reset/trajectory/XY children in a private PE32 mapping,
with no child callbacks, stubs or binary patches. It compares all 558 receiver
bytes plus external guards, checks original host pointers against owned cell
and catalog allocations, then normalizes them to ordinals in the output stream.
Every cell byte/guard remains unchanged; the guarded 65,536-entry catalog remains
unchanged. Random initial bytes expose missing resets and unintended writes.

There are 1,280 fixtures across five grids, including one-cell axes, asymmetric
maps and dimension limits. Source samples cover zero, maximum valid fine units
and interior points. Destination samples cover equality, opposite boundaries and
full raw random words. Each fixture additionally compares 16 unmodified original
`004df500` steps (20,480 total) against the initialized native trajectory.
Original PE32, native64 and ASan/UBSan output streams match. Explicit native
units check field preservation, reset semantics, terrain ordinal 65535 and atomic
invalid-input refusals. Strict units, sanitized units and standalone CTest pass.
The manifest verifies all 2,927 immutable originals before and after execution.

```sh
python3 tools/test-effect-motion-initializer.py
```

Confidence: high within valid authored starts and row-major owned cells.
Native dimension limits (1..128 horizontally, 1..32 layers), exact cell-storage
size and source bounds are checked before mutation. These refusals are native
ownership policies; the original would compute unchecked pointers. Inputs,
receiver, movement and startup storage must be disjoint. Installed parameters,
full creation/type-dispatch/animation setup, creator ownership, ballistic update,
nonstandard offset tables, membership changes and live replacement remain pending.
Historical initializer and movement evidence retain their original fingerprints.
