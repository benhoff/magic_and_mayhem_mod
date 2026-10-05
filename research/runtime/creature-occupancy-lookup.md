# Creature occupancy lookup

Build: No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high for the whole bounded-receiver leaf; no live replacement.

`004e1200` is a read-only thiscall query. ECX points at sixteen WORD footprint
rows (`+0..+1e`) followed by a raw height DWORD at `+20`; stack arguments are
signed fine-unit X/Y/Z offsets and the callee pops twelve bytes. It rejects
negative X/Y/Z, X/Y greater than or equal to 64, and Z greater than or equal to
`height << 4`, with the shift wrapping at 32 bits. Accepted X/Y select row
`Y >> 2` and MSB-first WORD mask `8000 >> (X >> 2)`. The result is normalized
to 0 or 1. Z gates the entire footprint; it does not select another bitmap layer.
Each bit therefore covers a 4-by-4 block of fine-unit XY coordinates, extruded
through the accepted height interval. Height zero or a shifted height of zero
rejects all Z. Signed-negative Z stays rejected even when the unsigned height
limit exceeds `7fffffff`.

The effect parent `004883f0` calls it at `00488a3c` using receiver
`creature + b97` and offsets obtained by subtracting creature fine XYZ from
effect fine XYZ. This call-site interpretation is static; candidate selection,
creature transforms/wrapping, original footprint construction (`004e1260`),
parent return/parameter behavior, source capture and live collision handling
are separate work. This chunk executes only the complete leaf, not that parent.

Native `CreatureOccupancy` owns its sixteen row values and raw height. The const,
noexcept query supports all signed 32-bit offsets and all raw 32-bit heights;
it follows the original rejection, mask and wrapped-shift rules without adding
new height restrictions. There are no borrowed buffers or host pointers. Original
comparison buffers are packed explicitly into a private 36-byte receiver plus
sixteen-byte guards on each side; native struct layout is not a wire contract.

`creature-occupancy-lookup.json` records matching original32 (including native32),
native64 and ASan/UBSan output streams. Fifty-two authored receiver fixtures
combine clear/full/diagonal/mixed rows with thirteen raw heights, including normal,
large, zero and overflowing shifts. Each exercises every X/Y in 0..63 at nine Z
values, then boundary/sign/extreme X/Y combinations at the same Z values. The
whole original function runs without instruction patches, callbacks or stubs;
executable hash and entry bytes are checked. Receiver bytes and guards, native
rows and native height remain unchanged after every query. Input source and PE
hashes are stable during execution; original media manifests verify before/after.

Strict normal/sanitized unit tests use an independent fine-coordinate rectangle
oracle for each of the 256 single authored bits. They cover 4-by-4 block edges,
MSB orientation, exclusive height limits, negative/extreme offsets and raw-height
shift wrapping. The standalone CMake target runs those tests through CTest.
TL14 registers this new proof; existing effect motion/terrain sources and their
historical evidence fingerprints are untouched.

Remaining boundaries: candidate gathering and original parent integration,
creature status filtering, hit selection/ordering, effect-specific collision
termination, wrapped/local coordinate production, original footprint initializer,
installed creature footprint production, lifecycle and live replacement.
