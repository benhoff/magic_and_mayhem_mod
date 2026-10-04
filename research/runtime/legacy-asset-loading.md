# Legacy asset reader boundaries

Reviewed 2026-10-04. Static preferred-image addresses for No-CD Chaos.exe
SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`;
not verified live process addresses. High confidence for selected ANI conversion
instructions; no original legacy file-reader execution is recorded.

`0x004644d0` reads the 44-byte ANI header/table and dispatches using version
minus one through the five-entry DWORD jump table at `0x00464ab4`.
Entries for versions 3, 4 and 5 select `0x00464758`, `0x004648af` and
`0x004646f0`. Versions 1/2 select the error-reporting branch `0x00464726`.
The v3 path divides source bytes by 28, allocates 44-byte destination records,
and uses 28-byte source/44-byte destination strides. Instructions at
`0x00464860..0x00464875` zero the four trailing DWORDs. The v4 path uses
36-byte source records; `0x004649bd..0x004649c4` zeroes the final two DWORDs.
`0x004649fd` changes the in-memory header version to 5 after conversion.

Both paths copy old name fields with NUL-terminated string instructions.
Native records retain all eight source name bytes and source version provenance,
with zero-filled absent metadata. This does not reproduce uninitialized legacy
name padding or malformed-input memory accesses. The selected original
version-5 controller harness remains separate from file-reader execution.

`0x0057d310` requires SPR version 4 and uses a 24-byte envelope. It cannot
validate old SPR v2 acceptance or palette-word semantics. The shorter v2
stored layout is confirmed by all seven installed files and independent full
palette/metadata/pixel/mask comparison, not an executed original reader.
The opaque +28 word is preserved; no old address is dereferenced in native code.

`tools/export-legacy-asset-support.py --decompile` retains hash-pinned ANI
loader/jump-table and modern SPR loader disassembly, Ghidra pseudocode and
references in a read-only project export, with before/after manifest guards.
Stored layouts, repeatable comparisons and remaining boundaries are documented
in [legacy ANI/SPR loading](../formats/legacy-ani-spr-loading.md).
