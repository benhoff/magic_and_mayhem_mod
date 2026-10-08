# Complete World raster handshake, V2

The experimental adapter uses little-endian fixed64-byte headers and owned,
tight RGB565 rows. It is separate from the V1 at-most-eight indexed draw
experiment in `protocols/include/mnm/world_producer_bypass_v1.h`.

| Word | Request | Reply |
| --- | --- | --- |
|0..1|ASCII `MNMWBP02`|ASCII `MNMWBR02`|
|2..3|version2, header64|same|
|4|global selected-raster ordinal1..65536|same|
|5|captured queue ordinal1..16|same|
|6|exact producer sequence|same|
|7|canvas generation identity|same|
|8..10|width, height, tight stride=width|same|
|11|zero|payload byte count=width*height*2|
|12|zero|FNV-1a32 over pixel payload|
|13|zero|success1|
|14|signature-checked original raster entry|same|
|15|recovered low16AX result0/1|same|

Files are `world-raster-000001.request/.reply` with six-digit ordinals;
V1 retains its four-digit `world-bypass-0001` names. Reply publication is an
atomic rename. Native admission independently checks entry/backend and
geometry-derivedAX; runtime checks identity, exact extent, checksum and writable
destination pages before any copy. Width/height are1..2048 and original stride
must be at least width and at most4096. Native tight rows are written to the
existing locked destination using its actual stride. No original destination
pixels enter native rendering.

`MNM_WORLD_RASTER_QUEUE=N` selects one complete queue, or
`MNM_WORLD_RASTER_PREFIX=N` selects contiguous queues1..N. These controls and
legacy bypass are mutually exclusive. The original dispatch/traversal remains
active, including AX-directed clipped fallback. Each selected outer raster is
published before its original drawing body is skipped; unknown callers/entries
or malformed/late replies terminate this bounded experiment. Per-request wait
is120seconds. The fixture admits only reviewed format0 caller paths, not a
general native ABI replacement. Non-drawing lazy wave preparation is counted
separately from raster bodies.

The producer file is an append-only single-writer capture. Incremental native
reading validates the envelope, new contiguous records and complete payloads;
it does not repeatedly re-read previous bytes. Final completion compares the
entire accumulated stream against the file once. Prior-prefix mutation during
streaming is outside this observation transport contract; a final mismatch
refuses completion. This wire is filesystem based and is not a production
real-time transport.
