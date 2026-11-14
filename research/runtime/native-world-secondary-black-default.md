# Default secondary black drawing

The default runtime now admits the No-CD secondary World queue's black return
`0x500a16` for tag 12 and clipped return `0x500a32` for tag 17. Their exact ten-byte
signatures are checked by the existing production predicate. The original
executable, wire format, pixel algorithms, simulation and game balance are
unchanged. All 64 existing ordinary caller/tag pairs remain byte-for-byte intact;
the default table now has 56 callers / 66 pairs.

The first caller was observed refusing during an earlier interaction acquisition
at World 228, failure 43 / exit 95. The separate historical V1 compiler-overlay
variant already adds these same pairs to its exact older source. This change
enables them in the current default source. It does not retarget that frozen
variant or give its evidence new source fingerprints. The earlier failed
acquisitions and their charges remain recorded.

The secondary branch differs from the primary black branch: its short AX test
joins the secondary node store at `0x5009d2`. The clipped continuation stores
exactly two bytes at node+`0x1e` before joining `0x500d05`. The generator verifies
all 48 original bytes from `0x500a0e` and the 9-byte alternate store, including the
direct black/clipped targets, argument pushes, cleanup and branch destinations.
Both callers survive regeneration from an authored empty stream.

## Bounded validation

`SB14.default-original01` and `SB14.default-native01` retain the source-bound
aggregate under `secondary-black-default-results-20261114/summary01.json`.

- A freestanding PE32 fixture executes the unchanged secondary pushes, calls,
  normalization, fallback and stores. 393,216 AX cases cover all 65,536 low words,
  three high-word patterns, arbitrary black results and arbitrary clipped
  results. Production caller predicates inspect the actual return addresses.
  Stack cleanup, selected nonvolatile registers, exact node store and canaries
  pass. Raster callouts are synthetic in this AX phase.
- A separate phase executes the actual original black/clipped pixel bodies,
  then the native CPU pixel kernel through the same original queue fragment.
  All 6,804 cases match, totaling 2,068,416 compared WORDs including stride padding
  and canaries. It covers two RLE variants, empty width/height, signed origins,
  two clipping windows and offscreen/edge/interior positions. Source bytes and
  the selected original fragment remain unchanged.
- Actual installed producer hooks execute three original secondary branch calls
  with the entire original 800×600 destination PAGE_NOACCESS. They emit five
  owned CPS records: three black calls and two clipped fallbacks. AX metadata,
  node canaries, selected nonvolatile state, LastError and batch counts pass;
  after protection is restored all 480,000 original words remain unchanged.
- SoftwareGL consumes those actual five records and their canonical batch.
  All 480,000 words match an independently authored nonzero-background image;
  exactly eight words become black. Ordinary drawing/presentation uses zero
  readbacks; one explicit validation read compares the canvas.
- Default guards pass 4,412 refusals; the prior 54 caller / 64 pair baseline passes 4,258.
  The regenerated 25 caller / 28 pair subset passes 1,955 refusals. The complete existing
  producer forwarding/refusal suite passes, including 1,032 sprite controls,
  912 actual inaccessible-page spans, 15 primary sprite calls and 20 owned rasters.
  A normal production DLL builds with its current full source manifest.

Earlier private-memory setup refusals, an incorrect authored capture-header
sample count and a test compiler formatting refusal remain retained separately.
They are not renderer failures or successful executions. The accepted PE32
fixture reserves its fixed-address range in its own executable, loads the pinned
original drawing range into private memory and stops before queue traversal.
The owned model redirects only the two private raster entries. No original file
or running game is patched. Original consumption has verified manifest bookends.

This is isolated original/CPU/native integration validation. No complete game,
natural successful encounter with the new default callers, full volatile caller
ABI, arbitrary original pointers/modes, hardware performance or whole-player
acceptance is claimed. Historical broader rendering proofs retain their source
phases. The Linux i386 reference drivers remain available; the current ARM host
uses the freestanding Win32 alternative.

## Reproduction

Use fresh owned output/report paths and the machine storage guard. Build the
fixture import library first with `python3 tools/build-scene-observer.py --selftest`.
Run `tools/test-world-secondary-black-win32.py --output <owned-working-directory>
--report <new-research-report.json> --wine-prefix <owned-fixture-prefix>`.
For actual hook packets run `tools/test-canvas-producers.py --secondary-black-only
--wine-prefix <owned-fixture-prefix> --output-parent <owned-working-directory>
--report <new-research-report.json>`. Run these serially within that private
prefix. The normal invocation without `--secondary-black-only` runs the complete
producer suite. It launches fixtures, not a game installation.

Build `mnm-secondary-black-producer-test` through `tools/render-replay/CMakeLists.txt`
and pass the hook report's `stream` and `batch` files. Use an isolated display and
one-thread llvmpipe for the scoped GPU control. The retained controls record exact
commands, tested binaries, source versions and temporary-output reconstruction.
