# Native resource and state handling

The PE32 native facade owns canonical indexed8, RGB555, RGB565, BGR24 and BGR32
storage, independent interface aliases, palette references and current DC leases.
Explicit clip lists are canonical GDI unions, copied per operation. HWND clippers
sample the current system visibility region; no snapshot survives a call. Blt
applies clipped pieces in canonical order, retaining earlier writes on a later
source-bounds failure, as documented in the retained Surface2 driver corpus.
BltFast with an attached clipper returns BLTFASTCANTCLIP.

Indexed 8BIT|ALLOW256 palette objects own all 256 flagged entries. Surface
bindings retain them; GetPalette acquires a reference. Updates and rebindings do
not alter indices or an already acquired DC's palette snapshot. Newly acquired
DCs use current bindings. RGB555 masks and byte pitches follow each allocation.
Native WORD producer admission excludes non-WORD surfaces and reports 555 versus
565 explicitly. GDI conversion between unlike formats continues through GDI.

The new native-only cases also exercise keyed self-overlap and COLORFILL in all
five formats, with physical row padding checked independently.

Native DC painting checks its owned bitmap is still selected before using its
stored DIB pointer. A foreign selected bitmap refuses without native writes;
reselecting the owned bitmap permits release. DC-to-surface commit copies logical
row bytes and preserves physical surface padding. Aliases share all this state.
A flip exchanges physical storage, CPU balance, DC storage ownership and history
authority before returning busy when the new front remains borrowed. Keys, clips,
palette bindings and cached logical DC identity stay with their objects. The
focused RGB565 held-lease schedules are recorded in
[native-render-completion.md](native-render-completion.md); arbitrary interleaved
leases and unfocused driver behavior remain outside that comparison.

Owned host storage has no physical display-loss event. IsLost is healthy and
Restore preserves bytes, refusing while borrowed. This does not claim GPU history
restoration: undefined renderer contents still require explicit reload, and
palette updates cannot define them. Existing renderer ownership/recovery fixtures
cover shared palettes, aliases, borrowed flip state and explicit reloads separately.

Boundaries: the facade admits at most256 canonical clip rectangles and256 palette
objects. Copy conversion/stretching and the subsequently implemented small or
reserved-endpoint/primary palettes, indexed primary modes and2D effects have their
own [implementation scopes and results](native-render-implementation-closure.md).
Physical driver loss, arbitrary concurrent callers and complete resident GPU/CPU
history coherence remain independent contracts.
Execution evidence and current live integration are recorded separately in the
coverage ledger; implementation is not a whole-game replacement claim.

## Execution evidence — 2026-10-15

[The source-bound facade comparison](native-resource-state-comparison-20261015.json)
passes408 retained independent Surface2 clip cases/78,336 native WORD checks,
including both ordered partial-write failures. Six fresh HWND cases match all
6,216 caller/region words against Wine11.16 built-in DirectDraw, driver SHA-256
`24c8c8fbbe0bac100efa5a716ebc78506e0f42e80d2af019b641911c0847ab28`.
The identical caller separately creates, moves/resizes, occludes, hides/shows and
detaches a real window. No original output drives native clipping.

The same execution passes76,362 native-policy checks:80 five-format lifetimes,
12,288 palette words,22,464 visible bytes and2,880 physical padding bytes,
32 clip lifetimes,32 idle flip lifetimes with busy refusal before mutation, and
256 retained palettes with the257th refused. Final device/surface/DC/clipper/
palette owner counts are zero. Keyed overlapping copies and fills are supplied-input
native-policy checks; they are separate from the captured RGB565 clipping corpus.

[256 DIB cases](native-resource-dib-comparison-20261015.json) and
[1,024 owned DC copies](native-resource-dc-comparison-20261015.json) independently
match GDI in RGB565/RGB555, including1,024 and5,120 lease refusals respectively.
[64 unchanged surface cases](native-resource-surfaces-comparison-20261015.json)
match2,048 caller fields/37,440 WORDs against Wine. The missing-key Blt negative
policy now returns E_INVALIDARG, matching the retained driver slice; selected
BltFast missing-key/unknown-flag cases copy opaque as observed. The existing
[services regression](native-resource-services-comparison-20261015.json) matches
64 caller cases/22,464 WORDs plus16 startup cases/256 fields, native logical
flip/capacity checks, and terminal unknown device/surface/clipper calls. Original
media verification before/after those compatibility runs passes.

These results establish scoped native/driver and headless resource handling.
They do not close complete presented sessions, in-place resident recovery,
held-lease driver flip equivalence in this facade or GPU/CPU history dependencies.
The renderer's separate held-flip/recovery model is not silently substituted for
the facade's conservative busy policy. Historical source hashes remain unchanged.
The subsequent DRAW-02 merge changes broader scene dependencies in the monolithic
retirement declaration: its64-surface/services results retain their recorded
hashes and are not new current-code proof for the complete retirement contract.
The focused resource, DIB and DC source declarations remain current.

## Follow-up ownership result

The historical report above exercised the earlier pre-mutation busy policy. The
completion change supersedes that policy with the independently compared held-lease
transfer. Its fresh resource regression retains the five-format/clipping corpus
and zero final owners. Historical evidence hashes are unchanged.
