# Guarded original drawing publication observation V1

`MNM_DRAW02_PUBLICATION=1` adds an opt-in original-active observation to the
No-CD producer collector. The staged executable must have SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
In-memory hooks guard the complete five bytes at `58c03a` (`ff5214eb1e`) and
`58c05a` (`ff521c8bf0`) inside opaque wrapper copy `58bf40`. They execute the
original indirect Blt/BltFast calls and preserve their HRESULT, flags, LastError,
registers and floating-point state before resuming the original retry loop.
The observer never replaces a drawing body. This is a build-specific observation
contract, not a new general DirectDraw adapter.

`drawing-publication-0000.bin` starts with eight little-endian u32 words: magic
`MNMDP001`, version1, record size96, build token40209ca7, maximum65536 calls,
maximum1024 readbacks, reserved0. Every subsequent record has24 u32 words:

| Word | Meaning |
| --- | --- |
| 0 | Contiguous one-based actual COM attempt ordinal |
| 1 | Route1 Blt or2 BltFast |
| 2–3 | Original HRESULT bits and post-call LastError |
| 4–5 | Producer sequence already emitted before this call, current World queue |
| 6–7 | Source/destination process-local canvas generation IDs |
| 8 | Destination is the currently observed primary wrapper at6f98c0 |
| 9 | Guarded original flags:1000000 or10 |
| 10–11 | Observer Lock/Unlock results; ffffffff when not attempted |
| 12–18 | Readback width,height,bits,R/G/B masks and byte pitch |
| 19 | One-based readback oracle ordinal;0 if unavailable |
| 20–21 | Cumulative dropped readbacks and retained readbacks |
| 22–23 | Reserved0 |

Successful primary calls attempt a read-only nonblocking Lock with flags4810.
Only positive-pitch RGB565 with f800/07e0/001f masks and complete readable rows
is retained as tight little-endian `publication-oracle-NNNN.565`. Maximum extent
is2048×2048, pitch8192 bytes,1024 planes and the configured oracle byte budget.
Readback failure, unsupported layout, exhausted storage, write failure or failed
Unlock increments explicit loss accounting. Failed application calls retain
HRESULT and LastError but do not become successful publications. Observer calls
are separate from application results. Successful pixel capture is evidence of
the original primary's bytes, not a monitor/desktop screenshot.

The legacy producer stream remains V1/V2, but closes after the successful opaque
primary publication following the requested final World return, retaining the
intervening HUD/cursor producers and final copy. The legacy done marker counts
all records; publication-mode admission permits the trailing producer records.
Ordinary collectors still stop at World returns and retain their previous ABI.
Raster bypass, extended32-queue batches and live producer viewers are refused
when publication observation is requested.

This observes two opaque-copy sites only. Alternate wrapper routes, direct
primary writes, aliases/Flips, palette formats, loss/Restore, readback effects on
broader applications and applied action/frame identities remain unresolved.
`observed_primary_blt` is deliberately narrower than the complete
`presented_frame` contract used to close DRAW-02 journey gates.
