# DRAW-02 public Wine client memory binding

Step2 remains in progress. Known public map points cover canvas2 and UI canvases; protected source IDs7/8/9/10/13 remain unlinked. Wine11.16 public declarations and DirectDraw implementation lead to a texture descriptor-update API taking texture, subresource, client memory and pitch. The installed ddraw import is RVA0x6d700 and wined3d export RVA0x221460; hashes and runtime slot/export checks must precede observation.

Source leads: [public header](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h), [DirectDraw surface implementation](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/ddraw/surface.c). These identify a route to observe, not installed execution or upload proof.

A separate optional observer will forward the original exactly once outside the journal lock, preserve HRESULT/LastError/FP, retain a local successful memory binding and query its address against known live canvas cache at a submitted source texture. The client API supplies pitch without slice extent; the exported point must mark that limitation explicitly. Fixtures precede original execution. No private object layouts or source bytes are read.

The observer uses a separate MNMWCM01/version1 journal:64-byte header,192-byte
rows. Setter entries/returns preserve the exact original inputs and result,
plus portable before/after cache points. Successful setters assign a local
binding version; failures retain the previous binding. Source queries link the
current known texture generation/subresource to an exact blit submission and
its raw journal hash. They record a current pitch-only cache point or explicit
unknown. Point flag64 means the client API provided no slice extent. No address
or source pixels enter the wire; local data IDs do not imply allocation epochs.

The fixture first failed compilation due to a helper name conflict, then failed
its FP comparison with an uninitialized buffer, and then reached exit0 but failed harness
decoding because its lifetime input was missing. These are retained negatives.
The corrected fixture initializes comparison storage and records synthetic
creation/retirement/recreation through the actual lifetime observer. It passes
four client setter forwards, five blit submissions, failed setter retention,
null reset, generation reuse refusal, new binding and FP/LastError checks. Five
decoder methods and the full-extent/pitch-only export fixture pass. No cause is
assigned to the first FP failure and no original-game run used those fixtures.

Original01 completes with 52 setters and 1595 source queries:145 known UI cache
points across canvases12/14/15/22/53/54,342 unbound, 493 unknown and 615 stopped.
It also closes118 protected pairs / 223 source commands with 329 API / 328 EGL witnesses
and normal Quit exit0. The protected full-frame source canvases2/7/8/9/10/13
remain unlinked through this selected client-memory route. Several source
textures have a single public map before their canvas cache is known; a later
query of that last observed mapping is a new experiment, not present evidence
of retained allocation ownership after unmap. Native parity was not rerun for
this change. Whole upload, lifetime and complete drawing acceptance remain open.

A final source-only build manifest repair adds the two included observer headers.
The rebuilt standard drawing DLL is byte-identical to the original01 staged
observer; all manifest source hashes match current files. This is provenance
validation, not a fresh engine run. Original01 and prior fixture source
declarations retain their historical hashes after the builder change.
