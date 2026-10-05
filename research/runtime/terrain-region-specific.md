# Specific terrain placement and fallback

Selected No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is a bounded offline reconstruction of `0052f3f0`, over an owned descriptor
catalog and optional existing placements. It precedes the
[one-attempt solver](terrain-region-solver.md). It does not load CFG files,
implement the ten-attempt caller, assemble generated scenes or replace live work.

## Selected contracts

| Address | Behavior |
|---|---|
| 0052f3f0 | Two-pass Specific placement and configuration-warning recovery |
| 00530c60 | Seeded choice of admissible offsets from the current scratch rotation |
| 005310f0 | Compact row-first location/rotation table construction |
| 00531710 | Seeded location and orientation choice |
| 005300a0 / 00531b20 | Previously recovered admission and connector-walk placement |
| 005c520c | MessageBoxA import slot used by configuration warning branches |

The native `terrain_specific.hpp/.cpp` accepts placement state and one request
per descriptor, by value. Requests carry signed column, row and rotation fields;
`-1` is the wildcard. It returns owned placement state, mutated requests, the
last compact location table and ordered typed diagnostics. There are no widgets,
shared original objects or dialogs in this interface. The seed is supplied as a
value and remains unchanged, matching the original placement driver's receiver.

The original descriptor request fields are column at `+3a`, row at `+3e` and
rotation at `+66`, with 63-byte descriptor stride. These remain reference fixture
layout only; native requests are separate from the reusable descriptor bank.
The original location table has 25 twelve-byte slots at receiver `+386c`;
unused entries hold coordinates ffffffff and four zero rotation flags.

## Fixed anchors before wildcard locations

Each of two passes scans descriptor order. Ordinary descriptors and Specific
descriptors whose count is already at least their maximum are skipped. A live
Specific request with either coordinate equal to -1 has **both** coordinates
changed to -1. Each live descriptor loads its unrotated scratch carry even if
that pass does not place it.

The first pass handles fixed anchors. A wildcard rotation tries four rotations
and chooses an admissible one with the recovered seeded chooser. A fixed
rotation first tries the requested orientation. Failure records
`RequestedRotationRejected`, then tries four **offsets relative to the current
scratch descriptor/orientation**. That scratch descriptor can have changed
while admitting a multi-block section. The chooser restores its saved carry
after the tests. The chosen offset is added to the authored rotation for actual
root placement. Native storage normalizes that sum modulo four, matching the
original rotation helper's repeated turns; the request's authored rotation is
not rewritten.

If no rotation works at a fixed anchor, the driver records
`FixedLocationRejected` and changes both request coordinates to -1. That request
is reconsidered in the second pass alongside originally wildcard locations.

The second pass builds locations in row-first, column-second order. A requested
rotation restricts each location's mask to that orientation. If the table is
empty, `RequestedLocationsEmpty` is recorded and it rebuilds the table allowing
all four orientations. An already wildcard rotation goes directly to that
all-orientation search. Location and rotation are chosen with the prior seeded
chooser, and placement updates counts and grid assignments.

If every location fails, `AllLocationsEmpty` is recorded and the descriptor's
count is set to its maximum **without placing a block**. That count suppression
is recovered behavior, not a native substitute for the configuration error.
The requests retain wildcard coordinates after successful wildcard placement;
the selected coordinates appear in assignments and the retained location table.

This bounded entry point starts with an empty location table. Arbitrary incoming
retained location tables are outside this fixture contract. The final location
table is retained even after placement. Candidate tables for
the later solver are left untouched. All-rotation location scans restore the
root's unrotated carry after each coordinate. Nonzero fixed-rotation scans
rotate the possibly changed admission carry back to zero; fixed-zero scans
leave it as returned by admission. These details are checked alongside visible
placements, because retained scratch state affects subsequent original work.

## Offline reference and native bounds

```sh
python3 tools/test-terrain-specific.py
```

The wrapper verifies the immutable-input manifest before and after, pins the
selected executable hash, independently decodes 683 installed MAP headers and
records input, source, export and helper hashes. `--source-root` allows the
corpus to use a frozen source tree. The 32-bit harness privately maps the PE and
executes its placement code with the previously documented private FS exception
head. A script-compiled reference binds **only the private MessageBoxA import
slot**, after verifying its import name, to a stdcall recorder. The recorder
checks null window, warning flags, title and one of the four selected warning
text addresses and returns IDOK. No instruction bytes or executable file are
changed; no Wine process, actual Win32 dialog or other Win32 service is used.
Original message return values are unused on these paths.

Every accepted fixture compares the complete 50-descriptor packed bank,
including mutated requests and counts, all 25 grid records, all 25 candidate
tables, all 25 retained location entries, scratch carry and unchanged seed.
It also compares the ordered warning categories recorded by the import adapter.
Native diagnostics attach a descriptor index; original warning category/order
and final descriptor effects are compared, rather than claiming an independently
captured descriptor index for each original MessageBoxA invocation.

The synthetic corpus contains 4,096 cases: asymmetric edges, fixed and wildcard
requests, one-coordinate wildcards, all five rotation modes, occupied obstacles,
already exhausted descriptors, mixed section shapes, fixed-anchor relocation,
rotation fallback and no-location suppression. Each installed header contributes
ten cases: five fixed-anchor and five wildcard requests over a 4x4 grid. These
are single-source catalog fixtures, not complete original realm recipes or scene
images.

Of 10,926 proposed cases, **10,786 matched** (2,430 multi-block cases), with
24,438 newly placed blocks and 1,636 suppressed descriptors. Compared warning
counts are 1,280 requested-rotation rejections, 1,337 fixed-location rejections,
1,472 requested-location-table failures and 1,636 all-location failures.
The other **140 synthetic cases** reached the existing native missing-connector
refusal, after evolving counts/partial multi-block state. They are counted and
excluded from original execution; equivalence for those unchecked original
paths is not claimed. All 6,830 installed-header cases were accepted.

Native bounds require consistent owned assignments, supported grid/catalog
extents, one request per descriptor, coordinates -1 or within the grid, and
rotation -1..3. Fields are bounded before normalization, including requests
for skipped descriptors. Missing connector and occupied-write refusals from the
placement foundation remain in force. Input ownership ensures that refusal
cannot mutate the caller's state. Ordered diagnostics replace blocking UI as an
explicit native interface policy, while their selected recovery effects follow
the original driver.

The complete accepted corpus and refusal counts are repeated under ASan/UBSan.
Unit checks cover caller ownership, normalization, ignored ordinary requests,
count suppression, rotation correction, invalid fields/state and missing
connectors. The rendering reconstruction CTests run in normal and sanitizer
builds; all eleven passed in each build. Leak detection is disabled because
LeakSanitizer cannot inspect threads under the execution sandbox; address and
undefined-behavior checks remain enabled. Validation counts/provenance are recorded in
[the companion report](terrain-region-specific.json).

Confidence is high within the compared bounded domain. Remaining work is the
original generation caller's attempt reset/seed progression, complete CFG
catalog initialization and connector threshold, conversion of generated
assignments to owned assembly plans, and generated scene comparisons. Entities,
lighting, water and live integration remain separate.
