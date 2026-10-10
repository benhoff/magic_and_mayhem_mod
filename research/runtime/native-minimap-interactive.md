# Bounded interactive V2 minimap presentation

The public launcher `tools/run-native-world.py --startup-history --minimap-owned`
feeds owned minimap V2 inputs through the Qt producer session and existing
CPU/GPU World handoff. Original simulation and drawing remain active. Source
terrain, markers and camera corners are composed natively and completed canvases
are mirrored to retained GPU surfaces for presentation. Original completion pixels
are independent external oracles; they never initialize native storage.

The tail reader validates the complete version envelope before reading payloads.
V1 retains its 128 MiB bound; V2 admits up to 512 MiB and 262144 records. Asset
reads retain 128 MiB. Header changes, shrinkage, mixed versions, malformed records,
unfinished final records and inconsistent completion markers refuse. This is a
finite sixteen-queue viewer, not an unbounded stream or complete rendering takeover.

`--minimap-motion` builds the existing private-Xvfb fixture and adds 250 ms pacing
at completed original World returns. Launch under `xvfb-run`; all four stored
orientations and an adjacent same-orientation camera pan are required. Launcher
completion also requires every original/native checkpoint, all sixteen World
GPU comparisons, exact sequence/hash/extent identities and zero pixel mismatches.

`python3 tools/test-minimap-interactive.py --claims <prospective.json>` runs the
public launcher twice: paced motion and an independent unpaced startup. Reports
retain per-queue native CPU, checkpoint diagnostics, ingestion, minimap and World
preparation/submission/readback costs. Queue one includes menu/startup work; later
rows span successive completed World returns. The unpaced run disables fixture
pacing but retains original observation and all comparison/storage diagnostics.
These are Debug/software-GL diagnostic costs, not a gameplay FPS benchmark.

Pending boundaries include unbounded/manual gameplay, all maps, borders, source
visibility/camera generation, general caller/driver ABI, CPU-to-GPU minimap
rasterization and original minimap drawing suppression. Shared-source historical
evidence retains its hashes; this milestone does not revalidate other batch or
replacement contracts.

## Recorded result (2026-10-10)

[The public-launcher result](native-minimap-interactive-20261010.json) passes two
independent sixteen-queue sessions. Both compare 1062/1062 checkpoints and
474236912 pixels with zero mismatches. Motion observes views 0/1/2/3 and seven
centers, including same-orientation panning; its V2 stream is 262822134 bytes.
The unpaced startup observes view zero/one center with a 69186214-byte stream.
All World CPU/GPU results match; native input identities and all checkpoints
close exactly. Five malformed/oversized Qt tail fixtures refuse before records,
and all 84 actual repository compiler dependencies are prospectively declared.
Sources stay stable and original manifests verify before/after both sessions.

Unpaced Debug/software-GL medians for queues 2–16 are 211.2 ms native CPU
composition, 36.0 ms minimap composition (included in CPU), and 103.5 ms
checkpoint diagnostics. World costs are 148.8 ms CPU composition (part of the
211.2 ms total), 8.1 ms resource preparation, 68.4 ms GPU submission and 1.7 ms
readback. These overlapping counters must not be summed as independent frame
costs. Ingestion often happens before the later queue intervals; its zero median
does not mean it is free. Queue wall time also includes waits and other host
work. The next performance investigation should separate the always-enabled CPU
reference and completion diagnostics from normal native presentation, preserving
a strict comparison mode. No normal gameplay latency or physical GPU claim.
