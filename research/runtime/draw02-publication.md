# DRAW-02 independent primary publication acquisition

The new opt-in collector observes actual original Blt/BltFast HRESULTs at the
opaque wrapper's guarded COM call sites and independently reads the primary after
successful publication. It retains every observed call, including failures, and
records explicit readback losses. Producer history continues through the final
HUD/cursor and display copy instead of stopping at the preceding World return.
Original simulation and raster bodies stay active; movies are disabled only in
the disposable startup fixture, as in the established producer procedure.

The first diagnostic one-return Quick Battle map2 run records89 complete800×600
primary planes and210actual COM returns, with zero readback drops or producer
failures. Independent native replay compares all42,720,000pixels without a
mismatch. These counts belong to that retained diagnostic; startup/menu timing
can change the number of frames on a subsequent execution. Native input contains
only the producer journal, encoded source assets and declared source boundaries;
primary oracles and their hashes remain outside the renderer's input directory.
Native storage is cleaned up after the sequence.

```sh
python3 tools/build-scene-observer.py
xvfb-run -a -s '-screen 0 1280x1024x24' python3 tools/capture-scene-game.py \
  --canvas-producers --drawing-publication --samples 1 \
  --skip-window-screenshot --source-game <pinned-installation> \
  --prefix-template <prepared-wine-prefix>
python3 -B tests/test-drawing-publication.py
python3 tools/check-drawing-publication.py --capture-report <capture>/report.json \
  --headless --executable working/build/draw02-session/mnm-drawing-scene-replay \
  --report working/tests/drawing-publication-new.json
```

The parser checks the original call/producer ordering, fixed guarded flags,
complete frame extents/format, exact completion counts and monotonic loss counts.
It admits failed readbacks as explicitly incomplete observation, while native
comparison refuses them. Twelve synthetic guards cover truncated/reordered
records, forged losses, failed application publication, failed observer calls,
unsupported formats, missing final publication and unbalanced World boundaries.
They add no original failure or ABI-equivalence evidence.

The observer reads an original primary surface; native replay independently
constructs the source canvas that this bounded original opaque route publishes.
Alternate presentation routes and applied action identities remain unadmitted,
so this does not close any of the fourteen full journeys or twenty-one complete
operation/failure gates. Menu ACKs show successful requests but do not establish
which exact presented frame first reflects each action. Actual HRESULT retention
alone does not validate native failure/retry/Restore or caller-visible results.
The wire boundary and limits are in
[the format contract](../formats/drawing-publication-v1.md).
