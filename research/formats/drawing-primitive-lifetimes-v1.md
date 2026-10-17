# Primitive lifetime observation v1

Intentional diagnostic protocol, not original engine storage. The pinned build
is No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`--primitive-lifetimes` requires original-active physical producer observation,
whole updates and update routes. The runner explicitly sets or clears
`MNM_DRAW_PRIMITIVE_LIFETIMES`; malformed values and absent prerequisites refuse.
The ordinary native renderer does not enable this journal.

`primitive-lifetimes-0000.bin` has a 64-byte little-endian header: magic
`MNMPLIF1`, version1, row256, build40209ca7, capacity2097152, Windows QPC
frequency low/high, then eight reserved zeros. Capacity and write/clock failures
terminate the opted-in experiment with exit99. A private 64KiB buffer flushes at
callbacks and closure; incomplete runs cannot acquire clean-closure credit.

Each row is 64 little-endian u32 words:

| Words | Meaning |
| --- | --- |
| 0–5 | Ordinal, phase, unique call ID, local thread, producer hook tag, original caller PC |
| 6–9 | QPC low/high, closed producer prefix, entered queue prefix |
| 10–13 | Parent call on this thread, primitive flag, thread primitive depth, active call count |
| 14–15 | Reserved zero |
| 16–23 | Original ECX, EDX, six argument words |
| 24–47 | Full canonical 24-word owned request header when present; otherwise zero |
| 48–50 | Original return value when observed; cadence record and accepted sequence for callback rows |
| 51–58 | Known wrapper object, generation, pixels, width, height, WORD stride, dirty and sampled generations; zero when unassociated |
| 59 | Owned request header present, boolean |
| 60–63 | Reserved zero |

Phases1/2/3/4 are before request acquisition, after acquisition, observed original
return and recorder retirement. The first two bracket the recorder's input-copy
work; they do not timestamp individual original memory reads or prove an atomic
snapshot. The original-execution envelope also includes intervening hook work.
Phase4's emitted request sequence/header is checked against the independently
decoded producer journal when a request remains attached to that frame.

Phase5 records every accepted callback, including an empty pending set. Exactly
its active-count phase6 rows follow, naming every pending call and retaining
its current request and known canvas metadata. Callback time is the observation
under the recorder mutex, separate from the earlier engine cadence timestamp.
Phase7 closes with zero active calls and the exact final producer prefix.

Call IDs, pointers and thread IDs are local to a run. The parent is the newest
active frame on that same thread, independent of other threads. Hook tags0–56
name the exact-byte guarded producer entries in `canvas_producers.c`; tag100
is the selected World queue. Recording uses the existing recursive recorder
critical section and does not hold it across original bodies. LastError is
preserved. No row introduces a shared host pointer or C++ layout contract.

This trace identifies outstanding hook invocations and conservative acquisition,
return and retirement envelopes. Known canvas metadata, copied request headers
and equal output planes cannot establish all backing aliases/writers, immutable
original allocation ownership, global presentation completeness or native parity.
Requests may be published by an enclosing minimap callback; an absent direct
completion header does not prove that such input was never used.
