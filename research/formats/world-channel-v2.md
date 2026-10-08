# Finite native World history channel v2

Opt-in native wire policy, not a recovered original contract. V1 remains a two
slot newest-frame channel; v2 retains a finite first prefix without dropping or
superseding queues. It never waits for the viewer in the original consumer.

All words are little endian uint32. Magic `MNMWCH02`, version2, 128byte header.
Header offsets follow v1 through64; flags44 is HISTORY2 plus optional VERIFY1,
slots52 and target68 equal1..16. Reserved72..127 must be zero. File extent is
128 + target*(32 + 32MiB input capacity + 8MiB oracle capacity). Files are sparse;
16slots reserve about640MiB of PE32 virtual address space. This is a bounded
startup experiment, not sustained retention or a measured memory ceiling.

Each slot is published once. Offsets: ownership0 (FREE0, WRITING1, READY2,
READING3), publication sequence4, input length8, oracle length12, logical canvas
identity16, actual original consumer queue sequence20, native reset flags24,
reserved zero28. Inputs begin32; optional original output oracle begins
32+32MiB. The oracle is diagnostic output only, never native raster input.

The reader takes the oldest READY publication and requires both sequences to
cover exactly1..target. Logical identities are nonzero. First packet requires
NATIVE_ZERO_RESET1; later changed identities require reset, otherwise history
is retained. Reset explicitly chooses native word-zero initialization; it does
not claim original startup memory was zero. The current original producer maps
one unchanged original pointer/dimensions/stride to logical token1 and refuses
changes. It requires skip0, interval1 and startup-wave observation enabled.

Final publication sets ENDED. The viewer drains every queued packet and finishes
its last pending CPU/GPU job before terminating. Missing/incomplete queues,
identity changes, unsupported raster kinds, malformed reset metadata and
publication failures are fatal. Producer reason103 indicates a missing queue
boundary,104 a changed/invalid canvas,105 a slot collision,110 incompatible
sampling/startup configuration. Original drawing remains active.
