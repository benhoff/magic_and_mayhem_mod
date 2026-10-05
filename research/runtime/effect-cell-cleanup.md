# Effect ordinal-zero cleanup

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the offline scope below. No live replacement.

`00488330` is a thiscall unlink operation (receiver: 558-byte effect record;
stack argument: 12-byte cell; callee pops four bytes). After removing a found
head/interior record it sets that record's next WORD at `+1a0` to `ffff`.
An initially empty chain also enters cleanup without changing that next link.
A nonempty chain that does not contain the record returns without cleanup.

Cleanup requires terrain ordinal WORD `+0` zero, all three heads at `+2/+4/+6`
`ffff`, and DWORD flag `20000000` absent. It calls `00534520` with receiver
`006a49c0` and the cell pointer. A nonzero result ORs `80` into the cell flags;
a zero result leaves flags unchanged, including an already-set `80`.

`00534520` is a read-only eligibility query, not an allocator or destructor.
The receiver's first DWORD points to a six-byte-per-XY column table. A null
pointer returns one. For an allocated table it derives the cell's flat index
from `(cell - 006c54dc) / 12`, divides by plane size at `006c54a0` to obtain Z,
uses layer offsets `006cb8c2` and width `006c5494` to derive Y/X, and calls the
real Y/X wrap helpers `004132e0`/`0040e650`. For valid in-grid cell pointers the
result is the corresponding XY column. Column WORD `+0` zero returns one.
Otherwise signed BYTE `+2` and `+3` are compared with Z: either match returns
zero, neither match returns one. Bytes `+4/+5` are untouched. Their meaning,
the marker's broader meaning, table construction and other consumers remain
unrecovered; lower/upper member names are descriptive, not confirmed ordering.

Native `effectCleanupEligible` models the query using owned column entries;
`cleanupEmptyEffectCell` models the guard and flag update with explicit optional
creature/other heads. An empty vector represents the original null pointer.
`transitionEffectEmptyWorld` now accepts this owned table and terrain ordinals
0..3, applying cleanup immediately after unlink and before destination insertion.
Its empty-world contract still excludes creatures and other occupancy. Native
bounds/table validation and atomic rollback are intentional policies, distinct
from original behavior on malformed pointers or corrupt chains.

The report `effect-cell-cleanup.json` records three matching execution streams:
32-bit native inside the original helper, standalone 64-bit native and ASan/UBSan.
The whole unmodified `004883f0` movement comparison exercises zero/nonzero terrain,
null/allocated columns, five emitting types, kind 0/68, wrapping, old/new chains,
cache/reference/recount updates and return 3. A separate callback/unlink comparison
covers remaining effect heads, last/absent heads, nonzero terrain, creature/other
head suppression, flag `20000000`, existing flag `80`, zero/nonzero markers and
signed layer values. Full guarded cell/record/table buffers are checked after
original calls. No original code is patched or stubbed.

Historical TL08 fingerprints are preserved. Changing the transition source and
harness makes that historical evidence stale; TL09 is new evidence for the
expanded scope, not a rewrite of the previous result. Blocked destinations,
membership-disabled movement, occupied collisions, special termination,
trajectory setup, recycling and live scheduling remain separate gaps.
