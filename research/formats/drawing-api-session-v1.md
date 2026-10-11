# Original-active drawing API session V1

`MNMDAPI1` uses a64byte little-endian header and128byte records. Header words2..7
are version1, record size128, NoCD tag0x40209ca7,262144application records,
1024primary planes and1GiB pixel storage. Header word8 is mode0 (full readback),1 (API metadata only) or2 (input calls only); words9..15 are zero. Metadata/input-only modes refuse any pixel plane, and input-only mode refuses surface/factory records.
Records have an ordinal, kind, method/phase, logical surface generation, related
generation, original caller, thread, entry LastError, actual result, exit LastError,
entry/return ticks, oracle ordinal, primary classification, extent/bits/caps and
observer Lock/Unlock results. Words20..28 hold bounded scalar call observations;
29is entry identity,30is completed primary-plane count and31is validity flags.
Local pointers retained inside the observer never become portable native inputs.

Kinds1..5 are surface methods, original input polls/MapVirtualKey, applied menu
phases, terminal accounting and DirectDraw factory/mode observations. Surface
slots0..35 follow the common SDK interface. Source/destination rectangles and
copy flags are observed where defined; this is an observation journal, not a
complete owned-command replay protocol. Batch contents, callback enumeration,
overlays, extended surface methods, palettes and external GDI/movie publication
need separate admission. Unsupported observed routes/aliases remain gaps.

In full-readback mode, every successful primary mutation through Blt/BltBatch/BltFast/Flip/ReleaseDC/
Restore/SetPalette/Unlock attempts an unthrottled independent RGB565 readback.
`api-oracle-NNNNNN.565` contains every pixel in row order. Observer readbacks call
saved driver entries and preserve application HRESULT/LastError/x87/SSE state.
Primary Lock failure, bad format/extent, Unlock failure, storage limits
and write failures are losses. They cannot silently turn into successful reference
coverage. Terminal reasons1/2 mean process detach/explicit stop; budget reason3
refuses complete admission. `session.done` must equal the journal's terminal record.

Menu requests require matching same-thread entry/return identities. API input
observations prove returned polling values or actual scan-code conversion; they
alone do not prove selection, picking, camera state or combat outcomes. Full-readback mode has boundary `observed_primary_api_mutations`; metadata/input-only modes are `api_results_without_driver_readbacks`/`input_calls_only`. Unknown primary classification refuses full-readback admission. Global presented-frame completeness
and semantic-input completeness remain separate acceptance gates. All recorded
primary pixels and API outcomes are comparator references, never native inputs.
