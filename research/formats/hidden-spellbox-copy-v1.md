# Hidden Spellbox redraw diagnostic

`native-hidden-spellbox-copy.bin` is a128-byte first-hit native diagnostic,
written in the private game directory only when the declared hidden redraw
policy suppresses an operation. It is not an input or shared channel. Serialize
32 little-endian DWORDs; process-local lifetime IDs are not cross-launch IDs.

| Words | Meaning |
|---|---|
|0..1|Eight-byte magic `MNMHCP01`|
|2..5|Version1, size128, original build tag40209ca7, original return5770fa|
|6..9|Source width/height and destination width/height|
|10..13|Original source left/top/right/bottom|
|14..17|Destination X/Y, wrapper index15, native no-op decision1|
|18..21|Source/destination bit depths and native backing lifetime identities|
|22..31|Reserved zero|

The finite policy requires both640x48016-bit owned canvases, an opaque40x40
rectangle at identical source/destination coordinates, nonnegative signed
coordinates and no intersection with either canvas. The exact original call
instruction at5770f5 must be `E8 A6 53 01 00`. There is no memory access to
the rectangle's pixels. Visible or partially visible copies and other call
sites retain their existing path; direct surface APIs still return INVALIDRECT.

This is an intentional native safety policy, not original-driver equivalence.
An absent/truncated diagnostic supplies no positive branch observation. The
complete report and exact raw diagnostic are retained through SQLite.
