# Public source descriptors at Wine submission

The accepted pending-copy run exposes two known wrappers with no persistent CPU
stride: local canvas5 is40x40 and canvas6 is400x280. The selected full-size source2
is800x600 with stride800. IDs are launch-local observations; dimensions alone do
not identify content or establish backing ownership. Byte capture and full
frame/action/caller/lifetime closure remain separate.

An opt-in diagnostic samples the public source resource descriptor at the exact
existing Wine submission. It adds two public queries only for a known-live source
texture generation: `wined3d_texture_get_resource` and
`wined3d_resource_get_sub_resource_desc`. The latter takes the actual submitted
subresource and returns ten32-bit public fields. See the
[Wine11.16 declaration](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h).
This upstream declaration is an ABI lead, not proof of installed execution.

The installed PE32module matches the existing SHA256
`3481e6904716421951615d8ee2ca097b55ea20eee65dea6bac1596426d950c82`.
Its non-forwarded public getter RVAs are219320and204340. Loaded export identity
must match before activation. The existing resource getter trampoline is used
directly, so the new diagnostic queries are counted separately from original
calls. No private layout, pixel bytes, map/unmap or additional wait is used.

Rows bind submission/API/thread/source-generation/subresource and a portable
resource ID. Untrusted sources are not dereferenced. Null getter output, failed
descriptor result and a changed generation remain explicit. The original blit
still runs once; its incoming and returned LastError/FP state must remain intact.
The default adds no queries. No source ownership, whole alias/lifetime coverage,
unobtrusive gameplay, native parity or complete acquisition is inferred.

Owned PE32forwarding/state and decoder guards must precede original activation.
This is a pending diagnostic policy; original comparison/integration/replacement
remain independent. Older evidence and source fingerprints are retained.

Fixture05 repeats the unchanged source with a corrected `headless` scenario
declaration after audit01 rejected the earlier `native_integration` scenario
label. Seven original forwards and all five decoder guards pass with stable
sources. Fixture04 and its exact claims remain historical; its scenario binding
is not current validation. The additional eight cadence guard methods pass.
The second unchanged3GiB original preflight also refuses, at101.470GiB free;
no Xvfb/game run starts and no original result is registered. The desired original
scenario will be registered with actual execution evidence, not a preflight.
Committed `e3c0c2bf` parent/current accounting covers15versions with exact receipts;
append-only journal metadata is separate. Remaining original activation and exact
three-journal join validation remain pending.

## Source descriptor activation and authorized storage override (2026-10-12 UTC)

The user explicitly authorized “override the gib restrictions.” The scoped
[override record](draw02-storage-override-20261012.json) disables the working GiB
ceiling and free-space reserve for this DRAW-02 continuation. The default policy
and archive/deletion scope remain unchanged. Commands still record output
estimates and measured storage. Implementation commit `69726578` passes the
working, staged and required hook checks; the two earlier stopped staged checks
remain historical logs. This authorization removes the earlier storage block.

[Original descriptor result01](draw02-wine-source-descriptor-original-01.json)
retains a fresh original-active short run with unchanged original manifests and
executable sections, stable source hashes and normal original Quit. All 2,546
source samples succeed and bind to exact submission/API/thread/source generation,
subresource and lifecycle prefixes. The diagnostic records 5,092 extra public
queries, 466 physical API witnesses and 174 consecutive advancing complete update
windows. Every-presentation retention and native parity remain false.

Twelve distinct descriptor shape/access tuples include 40×40 (3,200 bytes),
400×280 (224,000 bytes) and 800×600 (960,000 bytes). All report format116.
Launch-local opaque source generations distinguish observed instances; dimension
equality does not identify content or prove shared backing. The Wine11.16
[public declaration](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h)
names format116 as B5G6R5_UNORM and access bits1/2/4/8 as GPU/CPU/map-read/map-write.
Observed access13 therefore indicates GPU and mapping permissions; access14
indicates CPU and mapping permissions. These labels are an interpretation of the
public numeric descriptor, without private-layout reads. Pitch, source bytes,
ownership, all aliases and concurrency still require independent acquisition.

[Current control01](draw02-source-descriptor-cadence-control-01.json) retains
261 complete original update windows with counter/menu/input observation only,
unchanged originals and normal Quit. The
[current comparison01](draw02-source-descriptor-cadence-comparison-01.json)
binds the successful actual short input schedule, settings, input/counter
executables and raw traces. The first120 common queued updates yield a combined
diagnostic median1.216344× and p951.900047× the control. Randomized workloads and
capture duration differ; this result grants no unobtrusive-gameplay or semantic
equivalence claim. It does not isolate the cost of the descriptor getters.

The committed `69726578` parent/current review checks22 source versions against
exact receipts, with append-only journal metadata separate. Historical hashes
remain unchanged. Original activation and positive three-journal bindings are now
recorded at their bounded scope. Adversarial joins, source pixel acquisition and
ownership, every presentation, full action/caller/lifetime/routes and the47/21
gates remain open. Step2 is still partial; comparison, integration, replacement
and later steps are not promoted.
