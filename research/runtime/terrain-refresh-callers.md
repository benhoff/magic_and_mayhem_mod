# Terrain light refresh callers and local scheduling

Build: no-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This batch follows [supplier recovery](terrain-light-suppliers.md). It separates
whole caller execution on admitted inputs from a selected world-loop region;
none of the claims establishes the complete gameplay scheduler or live use.

`TerrainLightRefreshCallers<Memory>` owns no game pointers. Its memory interface
supplies owned guest bytes and explicit external service endpoints. Those
endpoints are recorded fixtures, including collision, allocation, placement,
coordinate conversion, creature setup and lifecycle work. Their internal
behavior and installed-world inputs remain separate contracts.

## Creation caller

Whole `0048f690` is admitted with a valid existing linked creature at `+44`.
Collision rejection and owner population limits return before allocation. A
failed allocation stores null at `+198` and returns before initialization or
refresh. Success initializes, binds and sets the created creature state, then
compares its resulting owner with the current player at `00644520`. Only an
owner match requests refresh through the unchanged `004f2dd0`; the caller then
stores the new creature's first DWORD at `+48` and returns one. The old refresh
value survives every path that does not request it.

The new-owner selection branch (`+44=-1`), script creation, invalid indices and
external service bodies are excluded. The fixture supplies created records;
this is not evidence of installed creature production.

## Position event caller

Whole `0051e890` is admitted for event9 with `+718=0`, plus default events
(excluding5,8,14). Event9 reaches position transition at `0051e946`. Coordinate
and placement suppliers are explicit endpoints. Failed placement skips refresh.
If the lifecycle services deactivate the creature before `0051ea24`, the caller
returns before refresh and its later state-reset work.

Success runs cleanup and state services, calls `0050ab30`, then requests refresh
at `0051ea96`. It clears selected cached fields, copies the resulting position
into `+96b/+96f/+973`, restores `+104` (or zero when the `+798` service took
ownership), and dispatches event10. Defaults perform the two coordinate supplier
calls and return without requesting refresh. Other event branches and the
`+718` continuation remain unvalidated. The sparse original event jump table
remains explicitly incomplete in the coverage register.

## Selected scheduler region

The executed region is `[0046b836,0046b91a)` inside `0046afc0`, entered with
`EBX=0` and `ESI=parent`. Checked exit endpoints stand at `0046b91a` and
`0046bca3`, outside the claimed interval. This does not execute or characterize
earlier parent gates or the later world-loop continuation.

Nonzero `parent+fd5a` skips the region. Otherwise services run in recovered order;
`00571300` is reached only when both `006deed0` and `006f2aa8` are zero. A zero
`00536f00` timer-service result exits before lighting. A later deadline service
can update `parent+fdea` with the unsigned sum of clock and delay; it does not
gate the lighting call. Lighting itself requires nonzero `005e1404`.

Skipped dispatch preserves `005e41a8`. Admitted `004f27a0` runs creatures then
static objects and only then clears the flag, including a request written during
either callback. The recorder matrix checks this ordering; the scene matrix
restores both exact original subcycle bodies and executes actual kernels,
initializer, updaters, stamps and request helper against guarded owned buffers.
Creation and position callers inject requests between ticks; source admission,
buffer publication and eleven state values are compared after every tick.
The native side uses the existing `TerrainLightingCycle`, without inventing an
immediate refresh on each mutation.

## Reproduction and limits

Run `tools/test-terrain-refresh-callers.py --main-root MAIN --output NEW_DIR
--report research/runtime/terrain-refresh-callers-results-20261025/NEW.json`
under `tools/working-retention.py run`. The runner pins the original executable,
verifies the original manifest before and after, declares exact claims before
execution, checks compiler dependencies and source stability, and stores its
immutable report directly in SQLite. Original32, native64 and ASan/UBSan streams
must agree on all captured bytes and ordered normalized service calls. The
original fixture checks every adapter's expected bytes, makes executable sections
RX, restores original subcycles before scenes, and verifies protected bodies.

Full map loading, kernel/map-load caller chains, the other updater call at
`004721fc`, earlier/later world-loop gates, clock ownership, indirect flag writes,
installed inputs, heap failures and live integration/replacement remain open.
Historical evidence hashes and scopes are preserved.
