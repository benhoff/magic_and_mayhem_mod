# Windows file API audit and native asset boundaries

## Scope and reproducibility

Reproduce with `python3 tools/audit-file-apis.py`, or run the full native asset
milestone with `python3 tools/validate-native-assets.py`. The audit verifies the
original manifest before/after, checks pinned binary hashes, parses PE32 imports,
and uses `objdump -d -Mintel` to collect IAT references and callers of simple
import jump thunks. Outputs stay under `working/tests/file-api-audit/run-*/`.
No executable is patched, game launched, or original file modified.

| Input | SHA-256 | Preferred image base |
|---|---|---|
| Clean `working/game-clean/Chaos.exe` | `124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214` | `0x00400000` |
| No-CD `working/game-nocd/Chaos.exe` | `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168` | `0x00400000` |
| Supplied `working/game-clean/jpeg.dll` | `a8c2480789bb45ab6ff2d1779e0e7d454b35290f89b20fa3c999a9c0c263328b` | `0x10000000` |

The initial audit is `working/tests/file-api-audit/run-iajegd_n/`. It identifies
52 selected file-related/support imports in the clean executable, 57 in No-CD,
and 20 in JPEG. These broad counts include resources, modules, path encoding,
and standard handles; they are not counts of gameplay file actions or a native
coverage percentage. Every source hash remained unchanged; original-manifest
checks passed against 2,927 files before/after.

`report.json` per binary includes all imports, selected imports with preferred
IAT addresses, instruction/context evidence, simple thunk callers, watched API
absences, and embedded watched names. Updated reports also include path-like
strings with file offsets/preferred VAs, script hash, and disassembly hash.
Full disassembly is an untracked working artifact. Synthetic
`tests/test-file-api-audit.py` checks named/ordinal imports, fallback thunk
tables, reference attribution, path-string locations, and hash rejection.

Final audit evidence: `working/tests/file-api-audit/run-x62yzy6t/`, with the
same selected-import counts, all source hashes unchanged, and both original
checks passing. Its clean/no-CD string records locate `Save\\__temp.vas` at
preferred VAs `0x005c0f1c` / `0x005e4154` respectively. No cross-build address
stability is implied.

The complete workflow passed in
`working/tests/native-assets/run-cva0bd7o/report.json`: all steps exited 0;
raw evidence is `working/tests/asset-files/run-fqkvxpwr/` (4,834 files /
327,419,328 bytes), decoded evidence is
`working/tests/audio-buffers/run-zdfllnsr/` (356 WAVs / 11,908,772 PCM bytes),
and audit evidence is linked above. The audio build's current six CTests and
synthetic audit parser test passed. Fixture-only workflow also passed in
`working/tests/native-assets/run-8ypbbqui/report.json`. The six-test count
includes separately developed audio voice-contract tests; it is not a file
API coverage measurement. Workflow step logs retain all original checks.

## Confirmed gaps relative to the read-only interface

Confidence: high for imports and the inspected instructions below. Whether
each code path runs during gameplay is unknown. Clean and No-CD addresses
must not be interchanged. Representative VAs below use the **clean executable**.

| Action family and imported APIs | Representative evidence | Native milestone boundary |
|---|---|---|
| Open/read/size/close: `CreateFileA`, `ReadFile`, `GetFileSize`, `CloseHandle` | CreateFile calls at `0x0047c038` and `0x0058724b`; size query at `0x0047c047`; ReadFile IAT references at `0x00582882` and another site | Native loose-file operations exist and WAV uses them; original file/CRT wrappers and other loaders are not replaced |
| Write/flush/truncate: `WriteFile`, `FlushFileBuffers`, `SetEndOfFile` | WriteFile reference `0x00582318`; flush `0x005833cf`; truncation call `0x00588ca0` | No writable state/save/config service; read-only AssetStore intentionally exposes none of these |
| Rename/delete: `MoveFileA`, `DeleteFileA` | Direct calls `0x004db726` / `0x004db70f` reference `Save\\__temp.vas` at `0x005c0f1c`; another MoveFile wrapper call at `0x0057dd82` | Native save temporary-file commit/cleanup policy remains unreconstructed |
| Enumeration and metadata: `FindFirstFileA`, `FindNextFileA`, `FindClose`, `GetFileAttributesA`, `SetFileAttributesA`, `CompareFileTime` | First-file `0x004743b4`, next-file reference `0x004a0a49`, attribute-setting call `0x00471d54` pushes `0x80`, timestamp compare `0x0047442a` | Resolver enumerates internally, but exposes no engine listing, wildcard, attributes, or timestamp contract |
| INI/profile: `GetPrivateProfileStringA`, `GetPrivateProfileIntA`, `GetPrivateProfileSectionA`, `WritePrivateProfileStringA` | String API has 115 IAT references including `0x00407f29`; write API loaded into ESI at `0x004a3ca5` | Raw CFG bytes/encoder tools are separate from Windows profile parsing, defaults, section enumeration, and persistence |
| Paths/drives: `GetCurrentDirectoryA`, `SetCurrentDirectoryA`, `GetFullPathNameA`, `GetModuleFileNameA`, `GetDriveTypeA`, `GetDiskFreeSpaceA` | Current-directory reference `0x00407e6f`; full-path call `0x0045fd6a`; SetCurrentDirectory call `0x0057f066` | Explicit-root native policy does not emulate mutable process CWD, drive/CD discovery, free-space checks, or executable-path lookup |
| Seek/type/standard handles: `SetFilePointer`, `GetFileType`, `GetStdHandle`, `SetStdHandle`, `SetHandleCount` | Seek reference `0x00583170`, type query `0x00584259`, standard handle query `0x005842b2` | Native API is absolute, bounded, regular-file-only; no Win32 sentinel/last-error, relative seek origin, console/device, or CRT descriptor emulation |
| Multimedia file/container access: `mmioOpenA`, `mmioRead`, `mmioSeek`, `mmioDescend`, `mmioAscend`, `mciSendCommandA`; No-CD additionally `PlaySoundA` | Clean mmioOpen call `0x00521e8c`; No-CD mmio wrappers `0x005349df` / `0x005349fc`; No-CD PlaySound call/reference `0x0058fef9` | Strict WAV parser/upload supported offline; arbitrary mmio streams, original error behavior, and complete live multimedia loading remain separate |
| File versions, modules/resources, COM, code pages | Both executables import VERSION APIs, LoadLibrary/GetProcAddress and encoding APIs; No-CD additionally names FindResource/LoadResource/LockResource/SizeofResource | Native resource/JPEG/COM-dependent loaders and original path code-page behavior are not established |

The `Save\\__temp.vas` literal and the adjacent delete/move branches are
confirmed static save-file lifecycle evidence. Subsequent pinned No-CD
[persistence research](persistence-progression.md) recovers `.sav` destination
construction, overwrite admission, the intermediate stream and final packing.
It also establishes early destination truncation and unchecked final move/delete
results. Crash cleanup and live durability remain unknown; the temporary name
does not establish transactional guarantees. Do not transfer No-CD addresses
to the clean executable.

## Narrow caller observations and compatibility implications

At clean `0x0047c038`, preceding pushes specify desired access `0x80000000`,
share mode 0, null security, disposition 3, attributes `0x80`, and null template.
These decode as a read-only existing-file open with exclusive sharing. That
observed policy is stronger than the native interface's current file-open
contract. [Microsoft CreateFile documentation](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea).
The second CreateFile site passes computed fields; its complete access/create
policy and the statically linked CRT callers require further tracing.

The imported seek API supports origins relative to start/current/end, while
native AssetFile specifies absolute seeks only and rejects beyond-EOF offsets.
An original wrapper adapter must translate caller semantics rather than assume
the interfaces are interchangeable. Which origins and beyond-EOF behaviors
the game requires remain unconfirmed.
[Microsoft SetFilePointer documentation](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointer).

Clean mmioOpen at `0x00521e8c` passes a null filename, caller-supplied MMIOINFO
pointer, and flags 2. No-CD has both a filename/flags wrapper (MMIOINFO null)
and the null-filename/flags-2 wrapper. This confirms different mmio entry
forms, not that every mmio call opens an installed file. Buffer/custom-I/O and
resource-backed use need caller/MMIOINFO inspection; they remain hypotheses.
[Microsoft mmioOpen documentation](https://learn.microsoft.com/en-us/windows/win32/api/mmiscapi/nf-mmiscapi-mmioopena).

The JPEG DLL independently imports CreateFileA, ReadFile, WriteFile,
SetFilePointer, SetEndOfFile, FlushFileBuffers and standard-handle APIs. The
game also imports its `jpeg_read` export. Audit those DLL/CRT paths when moving
image loading native; replacing only the EXE's IAT cannot establish coverage.
Some write support may be CRT diagnostics or unused branches; no JPEG write
behavior is claimed without tracing export callers.

Windows profile APIs include parsing/default/lookup semantics beyond raw
file reads. Their native replacement should preserve only behavior required
by recovered game callers, with explicit tests rather than a generic INI parser
assumed equivalent.
[Microsoft profile API documentation](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofilestringa).

## Negative findings, limits, and next work

No named imports were found in the audited binaries for watched CopyFile,
CreateDirectory, RemoveDirectory, mapped-file APIs, GetFileTime/SetFileTime,
or mmioClose. No exact embedded names for watched-but-unimported APIs were
found. This is not evidence that the associated behaviors never occur:
dynamic lookup, statically linked helpers, DLL/COM implementations, and indirect
register calls remain incompletely attributed. CloseHandle also closes
non-file objects; an import reference alone cannot identify ownership.

The scanner records `mov`/other IAT references separately from direct IAT
call/jump references; it does not count every reference as a call. It records
callers of simple jump thunks, not complete function boundaries or call graphs.
No live instrumentation or execution frequency was measured. Preferred VAs
identify static bytes in the pinned images, not stable runtime pointers.

The six-chunk milestone is complete for native read-only loose assets and one
offline WAV loader. To move toward a fully native game, next recover config
profile callers and save-file lifecycle contracts, design writable user-state
storage separately from immutable assets, add needed listing/metadata helpers,
and port the next real loader (configuration/world/image) with output evidence.
Avoid expanding AssetStore into broad Windows emulation based on import names.
