# Native idle CPU guard diagnostic v1

`native-cpu-guard-fault.bin` is a best-effort local PE32 diagnostic, emitted before
exit124 on the first access violation inside an idle protected owned allocation.
It is96 bytes, little endian. It contains process-local pointer identities and
must never be treated as a cross-process wire contract or stable hook address.
Diagnostic recording does not read the pixel allocation. Modes2/3 inspect the
faulting instruction when checking admitted stores.

| DWORD | Meaning |
| --- | --- |
| 0–1 | ASCII `MNMCPUF1` |
| 2–3 | Version1, size96 |
| 4–7 | Faulting instruction, accessed address, access kind(0 read/1 write/8 execute), thread ID |
| 8–10 | Physical pixel base, pitch×height logical bytes, rounded allocation page extent |
| 11–13 | Width, height, byte pitch |
| 14–17 | Physical history generation low/high, session nonce low/high |
| 18–19 | Cached main executable and MnmDraw.dll module bases for this process |
| 20–23 | Signed Lock balance as DWORD, storage-DC presence, first-fault count1, reserved0 |

Padding and the trailing compatibility WORD share allocation pages and may fault;
the accessed offset determines whether an address is a logical image byte.
Physical identities/generations move with a flip. Resolve instruction addresses
against the exact launched executable/DLL bytes. The probe records their hashes;
no address is assumed stable across launches. Read and write access kinds have
actual fault controls; execute faults remain unexercised.

`native-cpu-guard-refusal.bin` separately reports a permission transition failure:
six DWORDs, magic0x474d4e4d, version1, physical base, requested guarded boolean,
observed old permission and expected old permission. This path is terminal even
if recording fails. PAGE_NOACCESS is1 and PAGE_READWRITE is4; mismatching prior
permissions are never silently accepted.

The guard covers idle stale owned backing, not raw reads during an active
writable CPU lease or a selected internal facade operation. It never resumes an
instruction with stale data and never requests GPU/Qt work from fault dispatch.

Mode2 reuses the fault record unchanged for active writable Lock discovery;
DWORD20 can therefore be positive. The format does not imply that a write fault
is an independent store: unsupported/RMW store encodings also terminate.

`native-cpu-guard-modules.bin` version2 is a separate 56-byte little-endian
initialization-time record. DWORD0–3 are ASCII `MNMCGM02`, version2 and size56.
DWORD4–13 are cached bases for Chaos.exe, MnmDraw.dll, MnmRender.dll, MnmScene.dll,
MnmPanelWriters.dll, MnmPixelWriters.dll, MnmMinimapWriters.dll, MnmColourRect.dll,
MnmRasterWriters.dll and MnmJpegPixels.dll, respectively. Zero means absent when
cached. Classification requires the exact staged PE SizeOfImage and source hash;
a base alone does not prove instruction ownership. Earlier intermediate 20-byte
module records are historical version1 records, not the version2 format.

Protected mode3 retains the96-byte fault format and module ordering, using
`MNMCGM03`, version3 and size56 for the module catalog. It keeps stale public
pages inaccessible during writable Locks, private snapshots and World loans.
Checked scalar MOV/STOS stores write the private mapping and advance the saved
instruction context; unsupported stores and raw loads terminate. During an
unfinished World loan, the mandatory World guard owns faults instead. The older
idle/mode2 limitations above describe those historical modes, not mode3 storage.
Explicit CPU-reader completions have the separate
[reader-kind counter format](native-cpu-reader-counts-v1.md).
