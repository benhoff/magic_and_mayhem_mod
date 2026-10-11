# Native GPU frame ownership and DirectDraw retirement

Requested 2026-10-10. Implementation and validation are pending.

The first milestone retains native World output on the GPU while source-only
HUD/minimap/copy operations preserve intervening history. A CPU staging shadow
may retain only independently known pixels; regions modified by GPU drawing
become undefined there. Destination-dependent CPU producers must refuse unknown
pixels rather than reuse stale World output. Native GPU copies preserve GPU
contents without a CPU export. Legacy comparison/writeback mode stays separate.

The second milestone introduces an explicit, versioned acknowledgement-only
original/native session. Owned sources, exact request identity, destination
protection, deadlines, cancellation and actual original return remain checked.
Ordinary delivery performs no World readback, pixel reply or original World
writeback. Diagnostic comparison may explicitly read native pixels in separate
verification runs. Original drawing outside the admitted World body and its
surface services remain active until separately replaced. No general picking
or arbitrary original CPU pixel-read equivalence is asserted.

The third milestone replaces the supported original graphics services and
presentation with a native compatibility boundary: factories, surface storage,
locks, copies, palettes, DC ownership, lifecycle and recovery. Final proof must
disable the real DirectDraw backend and retain independent frame and caller
comparisons, bounded active-gameplay/failure/recovery scenarios, native Qt
presentation, resource cleanup and the original-source manifest checks.
Unknown operations remain explicit refusals and incomplete milestones.

The current initial threshold is nineteen actual admitted World completions and
Qt presentations per second, without changing game speed or the selected
twenty-FPS engine cap. Sixty-FPS presentation is a later separate milestone:
removing transfers does not itself lift the original render cadence or prove
sixty independently rendered frames per second. Simulation/rules modernization
and gameplay balance changes remain outside this workstream.
