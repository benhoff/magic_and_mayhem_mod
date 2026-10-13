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
