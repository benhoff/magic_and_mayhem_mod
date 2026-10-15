# Source-only native JPEG pixel painting

`0x58d320` locks an original canvas wrapper and calls the JPEG DLL's fastcall
`jpeg_read(filename ECX, destination EDX, width, byte pitch, height, format)`
at `0x58d34b`, through IAT slot `0x5c5308`. Its original Unlock and masked-byte
boolean return remain around the native decoder call. Unsupported inputs can
forward before any destination mutation.

Native host preparation uses the read-only asset store and native JPEG loader,
not an original decoder result, live canvas, GPU history or screenshot. All
installed static JPEG files are decoded into bounded RGB888 source records.
The launcher checks original EXE identity, source paths and hashes, cache hash
and owned DLL hash. The PE32 adapter maps the cache read-only, validates its
complete structure and unique paths, then uses an indexed record for each call.
There is no per-call cache file I/O and no drawing-call limit.

The original JPEG DLL row sink quantizes RGB bytes by shifts into RGB565/555,
clips source columns/rows to target width/height, and advances by target byte
pitch. Native painting follows that sink contract. Independent original DLL execution confirms all752 installed static JPEGs:
3,008 RGB565/555/clipped/larger/padded cases and515,100,744 physical WORDs agree.
A second fixture executes the unchanged77-byte original wrapper through the
production decoder-call patch, preserving its boolean and Lock/Unlock counts,
receivers and pixels. The native/original decoder counters are3,008/0. External
decoder volatile GPR/FX history, dynamic/malformed JPEGs and integrated live
execution remain outside that result.

## Cache file version 1

All integers are little-endian `uint32`, with no serialized pointers. Maximum
file size is 1 GiB; at most 4096 entries; dimensions at most 2048 each. Static
installed corpus requires about381 MB RGB source storage. Mapping is read-only;
RGB source pages are faulted as used rather than eagerly copied into a heap.

The 32-byte header contains eight-byte `MNMJPIX1` magic, version1, header32,
entry count, total byte length and two zero reserved words. Each entry has five
words: total entry bytes, ASCII relative-path byte length, width, height and
RGB byte count, followed by the path without NUL and exactly width*height*3
top-down RGB bytes. Paths are slash-normalized, case-insensitive, nonempty,
without absolute/drive prefixes, empty components, `.` or `..`. Ambiguous
duplicate paths refuse the whole cache. Runtime absolute filenames must begin
with the launch's exact normalized owned game root. No dynamic JPEG, EXIF
transform, malformed codec recovery or arbitrary external path claim is made.
