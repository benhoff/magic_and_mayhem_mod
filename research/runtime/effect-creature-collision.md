# Effect creature candidates and collision

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the selected authored-world comparison; no live replacement.

The complete movement parent `004883f0` gathers an ordered array of 27 creature
pointers before trajectory stepping (`00488443..00488528`). Entry zero uses
the effect's initial current cell directly. Entries 1..26 use the static triples
at `005e1780`, wrapping cell X/Y repeatedly and rejecting out-of-world Z. The
unused table entry zero is (1,1,1); it does not determine candidate zero.
The order is current cell, eight horizontal neighbors (N,NE,E,SE,S,SW,W,NW),
then above and its eight neighbors, then below and its eight neighbors.
Positive Y defines the south direction here, without asserting camera orientation.

Cell+4 supplies one WORD creature ordinal, not a creature chain. `ffff`, ordinals
outside the catalog count at `006def5c`, and the creator ordinal (effect+44)
produce null entries. Kind (effect+4c) 68 skips gathering and scanning. Kind 34's
exception allowing the creator is visible statically but outside this native
movement contract. Valid ordinals address catalog base `006def58` + ordinal*e4b.
Candidate pointers are refreshed on an unblocked cell transition by the second
gather loop `00488843..00488934`; same-cell steps retain the previous snapshot.
Wrapped-cell aliases are not deduplicated. Both gather loops preserve the kind34
creator exception; this kind remains outside the bounded native movement API.

After terrain sampling/cache publication, `004889fc..00488a6f` scans all 27
entries in order. Every visited entry writes effect+198, including null and
status-filtered entries. Creature DWORD status +a8 equal to 27 skips its query.
Otherwise effect fine XYZ minus creature fine XYZ (+14/+18/+1c) wraps as DWORD
subtraction and is interpreted as signed input to real occupancy `004e1200`
on creature+b97. These differences are not adjusted by the world wrapping period.
The first hit writes creature DWORD id +0 to effect parameter 7 (+48).
Selected types 13/22/24/36 immediately return 0. Type 3 records return 0 and
continues stepping; later hits can overwrite parameter 7, while a later terrain
or height exit retains its own return 1 or 2. Kind 68 preserves +198. An exhausted
non-skipped scan publishes its final candidate (possibly null), even on no hit.

`transitionEffectCreatureWorld` composes the existing bounded movement operation
with an owned, immutable creature catalog and cell occupant vector. The new
state stores an ordinal for +198 rather than a process pointer. It preserves and refreshes the
candidate snapshot at the recovered points and passes raw coordinate differences to the recovered
footprint query. Creature occupancy also suppresses the empty-cell cleanup flag when the moving
effect leaves an otherwise empty effect chain; a non-ffff occupant blocks cleanup
even when its ordinal is outside the catalog. Other cell head +6 is authored
ffff throughout this contract. The existing movement and footprint sources
remain unchanged.
Unsupported admission throws before publishing caller state; this is intentional
native policy rather than a recovered original failure mode. Creator filtering
is admitted locally without broadening the earlier empty-world API.

The comparison runs the complete unmodified original parent with guarded effect,
creature, cell, column and terrain allocations and real helper calls. No code
patches, callbacks or stubs are installed. Whole effect and cell outputs, candidate
pointer-to-ordinal mapping, and immutable creature/catalog/column bytes are checked.
Original32, native64 and ASan/UBSan streams are compared; the executable and source
hashes are pinned and original manifests verify before and after. Authored cases
cover repeated calls, multiple selected types, creator and status exclusions,
clear/occupied terrain, kind 0/68, empty/invalid cell ordinals, aliases, raw signed
coordinate differences and both membership settings. Dedicated 48 additional fixtures force a transition from cell X=33 to 34
with creator-only occupancy proving exclusion after refresh, top/bottom Z and Y edge coordinates; sixteen also
leave the sole effect in an occupied terrain-zero cell to check cleanup suppression. Dedicated native tests
exercise hit/no-hit outcomes, kind/status bypass and atomic invalid-world rejection.

The record `effect-creature-collision.json` is the execution evidence. It does not
prove every combined movement/collision branch or installed world production.
Remaining scope: kind34 creator exception, other effect types/metadata, type35
terrain mutation, installed descriptors and creature construction/lifecycle,
installed cell/catalog capture, real caller scheduling/removal/recycling and live
replacement. TL17 separately extends the authored comparison to all candidate positions,
wrapped/aliased and vertical boundaries, competing hits, mixed exits and fixture
changes between calls. See [expanded proof and limits](effect-creature-boundaries.md).
Real scheduling, concurrent mutation and exhaustive combined coverage remain open.
