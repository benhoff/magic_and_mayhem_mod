# Moving-creature slowdown and local cache cleanup

The recording-free moving test still misses19–20FPS. The next measured target
is the original per-creature maintenance/action call at`0x509e50`: individual
calls stall for100–230ms in the private timing probe. Native renderer throughput
and simulation/callback wall time remain separate.

Evidence: [COMBAT12.slowdown-stages02](combat-small-colour-results-20261112/slowdown02.json).
Production engine, renderer, launcher and clock sources are unchanged by this
followup. The earlier small-colour implementation and Levitate finding retain
their own evidence and limitations.

## Storage result

Twenty-five stopped owned sessions held byte-identical380,914,028-byte,
source-derived JPEG pixel caches. The first reviewed Btrfs`FIDEDUPERANGE` pass
increased filesystem free space from13,069,262,848to21,596,712,960bytes, about
7.94GiB reclaimed. A second pass shared the same immutable cache bytes produced
by the new tests, recovering another1,776,091,136bytes. Separate before/after
measurements include unrelated allocation and Btrfs accounting effects.

Every selected cache keeps its inode, pathname, exact SHA-256, length, mode,
owner, modification time and extended attributes. Kernel deduplication verifies
equal bytes and preserves copy-on-write isolation. No evidence database, backup,
capture, source checkout, user save, prefix or retention pin was deleted or
shared with a backup. Stopped consumers and the cache producer/manifest were
reviewed before each operation. The10GiB floor plus4GiB launch reservation
passes afterward. Operation manifests, sources, checks and verified SQLite
backups remain under`working/slowdown-20261112/cleanup/` and
`working/coverage/backups/`.

## Recording-free moving test

`throughput11` observes eight ordinary casts and five Zombies changing world
positions. Its four ordered movement windows measure12.20,12.98,12.39and13.67Hz
from consecutive completed World headers. It records no video, source corpus
or frame trace. Existing bounded creature/spell observations and test controls
remain enabled, so this is still an instrumented scenario.

Eight four-second default/performance-core windows change only the owned
original main thread. One second after each switch is excluded. Rates remain
11.06–14.28Hz; short evolving windows do not prove a general CPU-placement
benefit. The original affinity is restored. No default affinity policy ships.

Native rendering averages15.85ms over the retained lifetime report. Source
intervals include much more time before the next World begins; that residual
includes original engine, UI, synchronization and host scheduling work. It is
not an exclusive simulation CPU measurement.

A new analysis also reuses the retained source07 terminal trace:4,417of4,719
tail polls have no ready or outstanding source snapshot. Its326source turns
average7.42ms verified-source queue delay,2.26ms acquisition/verification and
4.87ms native submission. Prior video/source observation overhead remains.
These observations favor investigating original production stalls; they do
not prove that every native rendering or presentation cost is negligible.

## Private original timing probe

The default engine-cadence PE32 preservation fixture exits3 on this Asahi Wine
route. GPRs, flags and LastError agree, but the saved x87 value moves from the
first to the third saved register slot. A private thunk retaining a separate
legacy x87 save/restore alongside FX state passes the fixture's13PE32 paths,
original forwards, state comparisons, guard and counter checks. The default
failed run and exact before/after bytes remain retained. This does not establish
a single-instruction translator defect or actual player-state corruption, and
no production workaround is installed.

Private staging overrides the observer entry assembly. The standard builder
manifest still names the production entry source; the retained`entry-x87.S`,
override script, linker map and exact staged DLL identify the tested variant.
The standard manifest alone is not a complete private-build source declaration.

The first instrumented setup is stopped following that control failure. The
first private-probe acquisition refuses pathname-based image identification.
The corrected acquisition handles anonymous Wine PE mappings, verifies the
loader-adjusted image base and every relocated text byte against the exact
staged DLL, and derives data locations from that build's linker map. It does
not assume a stable module or creature pointer across launches.

`throughput14` verifies six moving Zombies. Six stable read-only live-memory
prefixes contain up to146,667events, with zero recorded foreign-thread or
overflow counters. The final prefix supplies1,523complete advancing updates:

| Observed span | Mean | Maximum |
| --- | ---: | ---: |
| Accepted update interval |54.26ms|489.52ms|
| Accepted update to World queue |16.89ms|250.45ms|
| World queue to next outer pacer |21.90ms|438.27ms|
| Individual creature maintenance/action call`509e50` |0.166ms|229.58ms|

The maintenance median is0.0007ms and its95th percentile0.0087ms: the long
calls are outliers. A preceding retained prefix includes193maintenance calls
above10ms, with several above100ms. This is measured nested call wall time,
not CPU self time. Positional audio, action callbacks, movement/pathfinding,
translation, scheduling and other nested work still require separation before
choosing a production optimization.

The late300updates average50.84ms, after most Zombies stop moving. That quiet
tail does not satisfy the moving-combat target. Copied live-memory prefixes
are not original DLL-detach traces or normal-Quit evidence. All gameplay
probes end by bounded diagnostic cancellation; unsupported-raster and input
guards remain intact. No native creature replacement, gameplay balance change,
new original-output comparison or full-player acceptance is claimed.

The initial accounting attempt rejected the new scenario level`live`; the
corrected`live_observation`entry uses`COMBAT12.slowdown-stages02`. The original
report and accounting refusal remain immutable; this correction adds no execution.
