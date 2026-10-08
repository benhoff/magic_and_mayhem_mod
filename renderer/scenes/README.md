# Native scene service

`mnm-scene-renderer` owns a persistent RGB565 background, canvas and bounded
resource upload cache. Its input is an ordered list of value-owned `SceneDraw`
records: semantic resource ID, bitmap index, projected anchor, visibility,
clipping policy and optional 256-word colour table. Sprite origins are applied
by the upload cache. Records draw in caller order, including equal-depth ties.

```cpp
render::SceneRenderer scene(renderer, resources, background);
scene.draw({{redcap, bitmapIndex, screenX, screenY}});
auto frame = scene.presentGpu();
```

The caller supplies camera projection, depth sorting, ANI scheduling and any
visibility pass. Drawing never advances a simulation or animation clock. This
keeps the service independent of widgets and build-specific reconstruction.
The sprite and world previews translate their existing display queues into
these records; world picking continues to use the original identity-bearing
queue. Resource recipes load through the native AssetStore/ResourceManager.

The default draw budget is 12,320 entries (explicit maximum 65,536), in addition
to the resource cache and renderer budgets. Whole-list preflight checks visible
IDs, frames, palette compatibility, extents and unclipped bounds before changing
the canvas. Admission failure leaves the previous completed frame available.
A GPU execution failure refuses `read`, `present` and `presentGpu` until another
complete draw succeeds; partial pixels cannot be presented through this service.
Preflight may load CPU resources, and allocation failure can evict cached uploads.

Repeated resident frames retain their uploads. `draw` performs no readback;
`read` and CPU `present` are explicit synchronization points. `presentGpu` uses
the renderer's GPU presentation contract. `release(id)` releases GPU uploads.
Destruction releases the cache and both scene surfaces. Use and destroy the
service on the renderer's GUI thread; the renderer and manager must outlive it.
The preview adapters retain references into decoded resources, so their manager
must also keep those resources loaded throughout the adapter's lifetime.

Build independently with `cmake -S renderer/scenes -B working/build/scenes`.
Run `python3 tools/test-native-scene-service.py` for source-bound synthetic
ownership, complete-pixel, ordering, clipping, failure/retry and migrated preview
tests. Installed preview/window comparisons are separate commands documented in
[the evidence record](../../research/runtime/native-shared-scene-renderer.md).
Live original scene snapshots remain the next integration milestone.

Prepared hosts set both `cache.residentOnly` and `cache.preparedOnly`. They use
`beginFrame`/`drawNext(SceneBatchBudget)` instead of synchronous `draw`. On an
uncached draw, `uploadNeed()` returns owned identity/geometry metadata. Prepare
its planes on a CPU worker and call `supplyUpload` on the GUI thread. Drawing
then transfers full-width rows within the byte budget, checking elapsed time
between operations. No draw or presentation can sample an incomplete upload.
Equal palette values and projected-shadow row shapes share cached textures.
Worker inputs must own their decoded sources; never borrow manager references
across threads. See [World upload preparation](../../research/runtime/native-world-upload-preparation.md)
for ownership, cancellation, budget limits and source-bound synthetic evidence.
