# Protected source writer trace files

All files are little-endian. IDs and thread numbers are process-local. No raw
pointers are emitted in this new format; the legacy `MNMSLFT1` file is preserved.

`source-write-trace-NNNN.bin` has a 64-byte `MNMSWT01` header and 64-byte cache
rows. Header words 2–4 are version 1, row size 64 and build 0x40209ca7. Words
5–9 identify lease, source canvas, owner thread, queue and sequence. Words
10–14 contain source offset within the first page, 960000 logical bytes,
protected page extent, cache count and maximum 128 caches. Word 15 is zero.

Each cache row contains ordinal, phase 1, canvas and flags, then direction and
absolute offset from the protected page base, width, height, pixel stride,
bounding byte span, dirty/sample counters and memory query state/protection/type.
Word 15 is zero. Flag bits are 1 source cache, 2 equal pixel address, 4 logical
span overlap, 8 protected-page overlap, 16 shared virtual reservation, 32 known
layout, 64 successful memory query and 128 nonzero pixel address. Direction 1
is a nonnegative offset. All cached objects are retained, including unknowns.

`source-write-armed-NNNN.bin` (`MNMSWA01`) and `source-write-done-NNNN.bin`
(`MNMSWD01`) are 32 bytes: version 1, size 32, lease, canvas, owner and phase
2 or 3. Done follows verified restoration of the original page protection.

`source-write-fault-NNNN.bin` is 128 bytes with magic `MNMSWF01`. Words 2–7
contain version 1, size 128, lease, canvas, writing thread and owner. Words
8–15 contain target page offset, logical-source-pixel flag, lease phase,
exception code, access type, target protection, protected extent and source
offset. Words 16–23 contain writer-query flags, instruction RVA, PE machine,
timestamp, image size, checksum, memory type and instruction-byte count.
Flags are 1 query known, 2 PE32 image known and 4 instruction bytes known.
Words 24–27 contain 16 instruction bytes; words 28–29 are acquisition sequence
and queue; words 30–31 are zero. Unknown image fields and unknown bytes are zero.

Fault phases 1, 2 and 3 mean preparing/arming, armed and retiring. Faults remain
negative execution evidence; neither a fault nor an incomplete acquisition
counts as a normally restored lease. Decoder validation recomputes overlap and
checks exact marker/source identities, reservations and complete capture joins.
