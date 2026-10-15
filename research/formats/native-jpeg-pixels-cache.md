# Native JPEG source cache v1

This toolkit-owned file is distinct from original game formats. It serializes
source RGB bytes decoded by the native asset service, with no addresses or
original-output samples. See [runtime cache layout](../runtime/native-jpeg-pixels.md#cache-file-version-1).

The32-byte little-endian header stores MNMJPIX1/version1/header32/count/length/
two zero reserved words. Each entry stores five32-bit words: entry bytes, path
bytes, width, height, RGB bytes; then normalized ASCII relative path and exactly
width*height*3 top-down RGB888 bytes. There is no NUL terminator, row padding or
inter-entry alignment. Limits:1 GiB,4,096 entries, dimensions1..2,048. Duplicate
case/slash-normalized paths, dots/parents/absolute paths, malformed lengths and
trailing bytes reject the whole catalog before painting. Inputs/cache hashes
are stored separately in the launch manifest, not appended to this binary.
