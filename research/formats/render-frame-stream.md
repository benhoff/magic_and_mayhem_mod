# Shared render frame: MNMGL001

Toolkit-designed Wine/native interprocess format. Writer:
`runtime/render/bridge.c`; reader: `apps/qt-shell/frame_stream.cpp`.
Fixed file length: `64 + 2048*2048*4` bytes. All header words are little-endian;
RGBA bytes are top-row first, tightly packed, opaque alpha. Confidence: synthetic
Wine writer to native Qt/OpenGL reader verified; real engine capture pending.

| Offset | Type | Meaning |
|---|---|---|
| 0 | 8 bytes | `MNMGL001` |
| 8 | DWORD | Version 1 |
| 12 | DWORD | Header length 64 |
| 16 | DWORD | Sequence: odd during write, even after publishing |
| 20,24 | DWORDs | Width and height, each 1..2048 |
| 28 | DWORD | Row byte stride, exactly width * 4 |
| 32 | DWORD | Pixel format 1 = RGBA8888 |
| 36 | DWORD | Status: 0 not loaded; 1 frame published; 2 lock failed; 3 rejected surface; 4 DLL loaded; 5 hook armed; 6 inside DirectDrawCreate; 7 drawing interface intercepted; 8 DirectDrawCreate failed; 9 hook guard/install failed; 10 drawing interface interception failed; 11 enumerating display adapters; 12 enumeration completed; 13 enumeration failed |
| 40 | DWORD | Count of published frames |
| 44 | DWORD | Last DirectDrawCreate HRESULT (unsigned bit pattern) |
| 48 | DWORD | Number of intercepted DirectDrawCreate calls |
| 52 | DWORD | Last adapter-enumeration HRESULT |
| 56 | DWORD | Count of intercepted enumeration calls |
| 60 | DWORD | Last enumeration kind: 1 EnumerateA, 2 EnumerateExA |
| 64 | width * height * 4 bytes | Most recently published frame |

Qt creates a unique sparse file before launch. The Windows bridge opens its
Wine `Z:` path and maps the same file with CreateFileMapping / MapViewOfFile;
Qt maps it with QFile. The tested host is little-endian x86-64; the writer is
little-endian PE32 x86. It is not a generic architecture-independent IPC API.

The producer serializes capture with an atomic flag, writes an odd sequence,
converts the payload, updates metadata and publishes an even sequence with
release ordering. The reader loads the sequence with acquire ordering, refuses
odd values, validates bounded dimensions/stride, copies the pixels, and accepts
only if the sequence still matches. Repeated frame counts are ignored; stale
frames are dropped rather than queued. Status is diagnostic and may update
outside the frame sequence. A failed capture does not count as a new frame.

No pointers are transmitted. The mapped file is sized and magic/version checked
on both sides. The reader rejects malformed sizes, header magic, dimensions and
stride. A fresh file is used on each Qt launch; files are retained as experiment
artifacts after shutdown. There is no input-command channel in this version.

Startup status and HRESULT/count fields are diagnostic, not frame pixels. A new
reader remains compatible with an older writer (status 4 does not imply its
hook failed). Status 6 is published before entering the original API, allowing
Qt to report initialization that has not returned. Status 7/8/10 is published
after the HRESULT. The original call's arguments, HRESULT and LastError are
preserved. Qt shows startup diagnostics after ten seconds without a frame and
shows hard failures immediately. Successful frame capture restores status 1.

The adapter extension observes the imported DirectDrawEnumerateA and the
DirectDrawEnumerateExA pointer returned by the pinned executable's GetProcAddress
slot. It substitutes only that named export from the already loaded ddraw.dll;
ordinals, missing exports and unrelated lookups return their original pointers.
Enumeration callbacks, contexts, flags, HRESULT and LastError are forwarded.
Status 11 is visible while the original API (including application callbacks)
is executing; 12/13 and the last enumeration result are published afterward.
These states identify a stage, not the particular blocking instruction.
