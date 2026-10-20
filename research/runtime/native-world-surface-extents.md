# Bounded resident World surface extents

The verified clean No-CD installation starts its first campaign World on a
640x480 surface with tight640-word stride. The fresh native campaign attempt
run-x5xd3n3s reaches the handoff then records producer failure46/detail1 before
its first World: the existing batch_start guard requires800x600/tight800.
This reproduces an existing admission boundary, not a new raster-pixel defect.

The pending native policy admits exactly tight RGB565640x480 and800x600.
Producer protection and host batch validation must agree. Capture width/height/
stride at queue admission and reject changed dimensions, pointer or surface
before completing a batch. Keep palette/format, page protection, source ownership,
thread, generation/nonce and history checks unchanged. No new wire layout is
needed: existing headers already carry width/height/tight stride and older
consumers safely reject unsupported extents. Other extents and pixel formats
remain refused. Validation and live continuation are pending.

The first repaired640 run renders its native batch then stops before import.
The public RenderResidentWorldV1 binding has a second fixed800x600 guard.
It now uses the same admission predicate while retaining exact current-thread
full-Lock dimensions, pitch, format, epoch and ownership checks. Actual PE32
public-bind tests must pass at both extents before another live retry.

The legacy continuous pixel-reply comparison envelope remains800x600.
640admission is limited to dynamic file batches and resident ACK transports;
its rejection precedes any access to an unsupported continuous pixel slot.

The next640 live run verifies an actual original World return and binds the
owned surface, then the native command consumer rejects its640 destination.
The World import guard now shares the same two-extent predicate; generation,
nonce, RGB565 format and provider frame identity remain enforced. The GPU
composition test must independently check World/HUD/copy words at both extents.

The current headless run passes both host batch/reply extents,27malformed
manifests each, protected640CPU canvas/actual effect entry, four invalid tuples
and five midqueue identity changes. Actual exported public binds pass640and800.
GPU composition at both extents checks every RGB565word and World/HUD/copy output
without ordinary World readbacks; invalid identity/resource controls still pass.
Current shared effect and kind9 caller regressions are retained separately.
Live campaign outcomes are recorded in the ledger and immutable attempt review;
these native admission checks do not establish full live replacement.
