# Native pixel no-forward refusal V1

The synchronous callback writes `native-pixel-forward-refusal.bin` before
exit126. The fixed 128-byte record uses MNMPGRD1/version1. The exact fields and
seven reason codes are in `protocols/include/mnm/native_pixel_guard_v1.h`.
Installed/expected bitmasks describe route counts 3,8,1,7,1,23. Every unused
word is zero. This diagnostic contains no pointers or pixel data. A missing
report cannot turn an exit/fault into a passing guard.
