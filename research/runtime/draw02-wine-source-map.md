# Diagnostic source pixels through a public map

This native policy follows the fresh descriptor findings: target the observed
RGB565 GPU40×40and400×280sources with public read mapping. Initial scope is16map
attempts per shape, not complete source acquisition. Each successful mapping is
copied to owned CPU memory and closed before file retention and original blit.
Unknown sources, unsupported formats/access/geometry, prior observed mappings,
budgets, failed maps, invalid ranges and changed generations remain explicit.
Unmap failure or an overlapping diagnostic invocation stops the capture.

The [Wine11.16 public ABI](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h)
places row pitch, slice pitch and data in the map descriptor; READ is0x80000000.
The [public map implementation](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/resource.c)
delegates to the device context. Mapping may synchronize driver work and change
residency. Only explicit diagnostic runs enable this feature. Owned forwarding,
copy/unmap guards and pinned export checks must precede original activation.
No all-alias ownership, physical/native equivalence or whole step2 claim.

## Original mapped pixels and cursor-region association (2026-10-12 UTC)

Owned fixture02 passes 27 original forwards, 17 map attempts and 16 unmaps.
Eleven retained buffers match independent padded-row patterns after the fake
unmap overwrites the entire backing buffer. Disabled, unsupported descriptor,
existing map, failed map, invalid range, changed generation and quota branches
are covered. Failed-unmap and reentrant children exit99 before any original
forward. Seven decoder guards pass. Build01 failed on a mock-name collision
with the existing key mapper; its log remains retained.

Original attempt01 fails during wineboot53, before game launch. The old prefix
has 1,616 catalogued cold files, including kernel32. Its bytes are not treated
as available engine input. Retry02 clones the intact recent prefix from
`working/experiments/drawing-api-session/run-31dicfcb/wineprefix`; that dependency
and the new raw run `run-4lepb81x` carry `.retain` pins. Original02 verifies
unchanged immutable manifests and executable sections, stable current sources
and normal original Quit. It retains 32 successful closed map/copy/unmap samples:
16 at40×40 and16 at400×280, totaling3,635,200 bytes. All2,361 map-journal samples
bind to original submission/API/thread/source generation/subresource/lifecycle
identities. The collector records1,824 unsupported descriptors and505 quota
refusals; no source omission is disguised as complete acquisition.

The original short run also retains437 physical API witnesses and155 consecutive
advancing complete update windows. It still lacks every-presentation retention,
all-source ownership and semantic action completeness. Current control01 retains
274 complete original windows with counter/menu/input observation only. Strict
actual-schedule/settings/input/counter-binary/raw-trace comparison covers120
common queued updates: combined diagnostic median1.266315× and p952.161535×
the control. This includes all diagnostic mechanisms and randomized workloads;
unobtrusive gameplay and identical work remain unproved.

The400×280 payload has one SHA-256 across its16 samples and visibly contains a
10×7 cursor atlas. The40×40 class has six hashes across16 samples: copied
background regions, a clock image and cursor imagery. On this launch the common
40×40 scratch is object12/generation18 and the atlas is13/generation19; another
40×40 source is16/generation30. These are launch-local IDs. The image inspection
is numeric RGB565 unpacking, preserving raw files; it does not identify an asset
file or prove general cursor semantics. The installed `cursors.spr` header has20
indexed frames and is only a candidate source, not a demonstrated atlas match.

Observed calls2/3/4/6 save a40×40 region from object9/generation15 into scratch12,
key-draw atlas13 into the same destination region, copy the full surface through
the primary API parent77, then restore scratch12 to the original rectangle.
The physical witness for this first candidate is black and the ROI model fails
for1,463 of1,600 pixels. That mismatch remains recorded. Calls21/22/23/25 give
the same object/generation/rectangle chain and match the physical parent154 ROI
within one channel value. The scoped reference-only model finds13 such matching
cycles (20,800 pixels), one retained mismatch and one sampled restore whose
overlay has already hit the quota. It never filters these into an all-match claim.

The model uses an explicit inferred blue31 key, consistent with the atlas and
the Wine SRC_CKEY0x8000 flag. It requires the latest observed scratch writer,
exact generations/rectangles/thread/call order, sampled overlay and physical
input hashes. Seven independent red-background/green-overlay and adversarial
identity/write/order/geometry/hash/path/black-mismatch methods pass. Captured
original pixels are diagnostic-model inputs, never production native-app inputs.
No native-renderer or full-frame comparison is asserted. Internal writes, key
getters/overrides, clipped cases, startup delivery and whole source/action/frame/
caller/lifetime/routes remain open. Step2 remains partial; all47/21 gates and
later phases remain unchanged.

Final cursor analysis05 also replays the raw Wine submission/lifecycle records
with the capture decoder's API-entry keyed raw-row interface. It revalidates
physical request/ack/peer-report/image provenance against the recorded original
result before composing ROIs. The13matches/20,800pixels, one black startup
mismatch and one missing sampled overlay remain identical. Fresh owned fixture05
passes seven methods with stable final sources. Analysis02–04 retain failed
adapter attempts; no original inputs or historical result hashes are edited.
The earlier exploratory01 remains historical under its narrower adapter.
