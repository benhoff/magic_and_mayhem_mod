# CPU-reader journal v1

Authored diagnostic wire contract, not a recovered original file format. All
words are little-endian uint32. Extent is 8,388,736 bytes: 128-byte header and
65,536 immutable 128-byte slots. Original addresses are process-local tokens.

| Header word | Meaning |
|---|---|
| 0–1 | ASCII `MNMCRJ01` |
| 2–5 | Version 1, header bytes 128, record bytes 128, capacity 65536 |
| 6–9 | Reserved count, committed count, dropped count, error bits |
| 10–11 | Closed flag, command session |
| 12–15 | Executable base/extent, render adapter base/extent |
| 16–18 | Start/end TickCount32, normal completion flag |
| 19–22 | Ended contexts, requests, completions, created contexts |
| 23–25 | Active DC borrows, active contexts, exact gate bits 7 |
| 26–31 | Zero |

Error bits: 1 capacity, 2 reservation contention, 4 write after closure,
8 context collision, 16 outstanding ownership at close, 32 DC tracking ambiguity.
Any error or drop refuses complete evidence. The writer attempts one CAS, copies
words 1–31, then publishes ordinal word 0 with release semantics. No wrap or slot
reuse. Header counts and all published slots must agree; unused bytes stay zero.

| Record word | Meaning |
|---|---|
| 0–6 | Commit ordinal, event, thread, TickCount32, session, context ID, reason |
| 7–13 | Direct caller PC, allocation base, RVA, image extent, PE timestamp, mapped header FNV-1a32, header bytes |
| 14–18 | Canonical surface object token, logical surface ID, source physical generation, epoch, CPU ticket |
| 19–20 | Width/height; BEGIN/END instead carry explicit-reader family/route |
| 21–24 | History generation low/high, nonce low/high |
| 25–27 | DC handle token, original result (or validated helper result), kind/flags |
| 28–30 | Backing token, pitch, expected/validated physical generation |
| 31 | Parent context on BEGIN/END; acquisition context on DC events; otherwise zero |

Events: BEGIN=1, REQUEST=2, COMPLETE=3, DC_RETURN=4, DC_RELEASE=5, END=6,
DC_TRACKED=7. Reasons: GetDC=1, Preferences=2, Save=3, Load=4, explicit reader=5,
ReleaseDC=6, other materialization=7. Kind is bits 0–1 of word 27, matching the
existing CPU completion classes; flags are validated=256, checkpoint=512,
tracked=1024. Epoch zero is valid. COMPLETE records the pre-transfer generation
actually checked, while the source's current generation may have advanced.

An unresolved caller has base zero, RVA FFFFFFFF and zero words 10–13. A matched
PE header identifies a unique pinned file candidate; it does not hash mapped
code, prove an upstream original caller, or authorize code replacement. Clocks
are ordered only within this journal; no Qt/Wine cross-clock latency is inferred.
Unclosed prefixes, ambiguous identities and missing acquisition/checkpoint links
remain explicit failures or limitations. Synthetic validation and the single
diagnostic journey are recorded separately in the runtime research and register.
