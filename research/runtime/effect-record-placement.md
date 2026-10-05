# Common effect record placement

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

The source positions used by lighting originate in `00493f20`, the effect
record creation routine. The installed emitting types 3, 13, 22, 24 and 36
use its ordinary start-position branch. The sixth emitting type, 21, takes
`0049421f` into the separate `0048a270` height-search helper and is excluded
from the current model. This is a selected common placement reconstruction,
not complete effect creation or an original collection allocator.

Creation copies 63 DWORD parameters to record `+0x2c..+0x127`. The first six
describe start and destination X/Y/Z in fine units: 32 horizontal units and
16 vertical units per map cell. Both Z endpoints receive a signed upper clamp
to `mapLayers * 16 - 1`. Each X/Y endpoint adds one map period if negative,
then subtracts one if at least the period. This is one-period wrapping, not
arbitrary modulo normalization. The native scope requires coordinates within
one period beyond either boundary and nonnegative heights.

The ordinary branch derives source cell coordinates from start units with
logical shifts by 5, 5 and 4, storing cells at `+8/+0x0c/+0x10` and fine
units at `+0x14/+0x18/+0x1c`. Initial cell coordinates are copied to both
`+0x1f6/+0x1fa/+0x1fe` and `+0x202/+0x206/+0x20a`. Twelve metadata DWORDs
are reset to `0xffffffff`: `+0x1aa`, `+0x1ae`, `+0x1b2`, `+0x1b6`,
`+0x1ba`, `+0x1be`, `+0x1c2`, `+0x1c6`, `+0x1ca`, `+0x212`, `+0x216`,
and `+0x21a`. The recount coordinates therefore begin at `-1`, even though
the stamping coordinates have already been populated.

The creator parameter at record `+0x44` (parameter 6) must be `-1` in this
scope. Other values enter creature-owner and child-association logic through
`00522720`; that path is not implemented. Remaining copied parameter words
are retained without invented meanings.

The selected cell is `(z * mapHeight + y) * mapWidth + x`. Its original
12-byte entry contains a terrain ordinal at `+0` and effect-chain head at
`+2`. When cell flags at `+8` lack `0x40000000`, creation installs an empty
head or appends through effect record `+0x1a0` WORD links, terminated by
`0xffff`, then clears cell flag `0x80`. When `0x40000000` is set, cell-chain
insertion and flag clearing are skipped. In both cases record activity at
`+4` becomes 1 and `006898dc`, the lighting scan count, grows to at least
`recordIndex + 1`. This count is a high-water bound, not an active count.
The pool stores owned cell/record indices rather than original pointers.

The original stores selected cell and terrain-object pointers at `+0x194`
and `+0x190`; the helper checks those against its private owned allocations.
It also checks common zero resets at `+0x198/+0x19c/+0x20e/+0x21e/+0x222`
and the initialization marker `+0x22a=1`. Those unnamed implementation fields
are observations, not reconstructed child-system behavior.

`00494ab0` writes the effect type and dispatches 89 type-specific setups.
These set up animation, trajectories and other behavior. The comparison
redirects that child routine to a checked callback which only records the
type; the complete parent `00493f20` remains unmodified. Logging is disabled,
the creator is absent, and record/cell arrays are preallocated. A private
Windows exception-head segment permits the parent's normal SEH prologue and
epilogue; no exception recovery is claimed. The helper verifies the executable
hash and expected entry bytes before its private-memory redirection. It never
changes an executable file or a running game.

The durable [comparison result](effect-record-placement.json) records fixture
counts, output hashes, source provenance and normal/sanitized suites. Each
placement compares all copied parameters, positions, initial caches, metadata
sentinels, links, cell flags/heads/terrain ordinals and scan growth. Record and
cell allocation guards are checked. Native unit tests cover unavailable slots,
unsupported types/creators, coordinate bounds, corrupt/cyclic chains and
rollback before mutation. These refusals are explicit native policies; the
original may access invalid pointers or loop on such inputs.

```sh
xvfb-run -a python3 tools/test-effect-placement.py \
  --build working/tests/effect-placement/build \
  --sanitized-build working/tests/effect-placement/sanitized
```

Confidence: high for the selected common branch with the stated deferred child
boundary. Full type setup, type-21 height search, slot selection/recycling,
creator ownership, removal and movement remain open. The movement path around
`00488540` updates recount positions from fine-unit trajectory results; creation
alone does not establish when those results reach the lighting pass. The native
lighting adapter refuses an active emitting record while recount coordinates
remain pending. This guard prevents treating creation as validated moving
source production or a complete scene lifecycle. Live observation/replacement
remains absent.
