# Original API physical witnesses

This additive diagnostic mode is native test policy, not an original game format.
The `MNMDAPI1` session envelope retains version1 and128-byte rows; mode3 is an
explicit opt-in physical acquisition mode beside original readback0, metadata1
and input-only2. Historical modes/artifacts retain their actual boundaries.

After successful original Blt/BltBatch/BltFast/Flip/ReleaseDC/Restore/SetPalette/
Unlock, the observer calls the unchanged descriptor method outside its journal
mutex. Only a positively classified RGB565 primary with bounded full extents can
request a witness. No diagnostic Lock/Unlock is performed. Unknown classification,
unsupported format, record/frame/byte exhaustion and journal gaps prevent complete
admission. Method results, LastError and x87/SSE state remain original.

The journal row's frame ordinal is field12, primary classification field13,
width/height/bits14..16, descriptor caps17, and observer Lock/Unlock fields18/19
remain `ffffffff`. Original caller/thread/entry identities and call-input scalars
retain their existing locations. Terminal byte counts use full RGBA8888 planes
in mode3, instead of RGB565 readback bytes in mode0. `MNM_DRAW_PHYSICAL_MIB`
prospectively configures1..3072MiB of final-plane storage (default1024); that budget
is recorded in the session header. The1024frame cap stays fixed. Modes0..2 retain
their exact historical1GiB envelope; invalid mode/budget combinations refuse.

After publishing the row and releasing the journal mutex, GdiFlush completes and
`api-physical-request-NNNNNN.bin` receives those exact128journal bytes. The original
calling thread waits up to5seconds for a16-byte acknowledgement:
`MNMAPI01`, little-endian frame ordinal and original journal record ordinal.
Malformed/oversized ACKs and timeout terminate the diagnostic. Other threads
continue; this is not whole-process quiescence or unobtrusive performance proof.

The reference-only `mnm-drawing-observe DIRECTORY NEW_REPORT --api` locates the
unique DrawingSession client with the declared extent, requires full physical
screen containment, and acquires two equal full RGBA8888 X11 screen planes5ms apart.
API mode retains each unequal sample as
`api-physical-transient-NNNNNN-SS.rgba`, with full extent/hash and its frame binding,
then waits for two equal samples. At most32 unequal samples per API point and
1..2048MiB prospectively configured total transient planes are admitted
(`--api --transient-mib N`, default256); budget/stability failures terminate.
This accounts for observed asynchronous settling without asserting that every
physical change between samples was captured. Legacy opaque-site acquisition
retains its immediate double-read refusal behavior.
The peer writes `api-physical-oracle-NNNNNN.rgba`, exact identity/hashes/stability and ACK.
`api-physical-stop.flag` retires the peer only after journal completion. Native
rendering receives none of these references or original expected call results.

`tools/drawing_api_physical.py` validates closed session admission, exact original
request bytes, ACK identities, full planes, peer hashes/types/stability and exact
artifact counts. The boundary remains `observed_primary_api_physical`.
Source-input binding, external GDI/window/movie/overlay output, semantic actions,
driver recovery and complete presented journeys remain open. No fallback converts
these witnesses into a complete session catalog.

The PE32 fixture's synthetic ACK peer is expressly not real screen or original
acquisition evidence. Its deterministic fixture planes establish request/ACK and
forwarding guards only. Actual original/X11 acquisition is recorded separately.
