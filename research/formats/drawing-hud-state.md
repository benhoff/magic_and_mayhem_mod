# Original HUD hit/camera observation journal

`runtime/menu/drawing_hud_state.h` writes a bounded, read-only observation journal.
This is a diagnostic wire format, not an original game file or recovered C++
object layout. `tools/drawing_hud_state.py` validates and decodes it. The producer
buffers samples on the existing menu owner thread and flushes on normal detach;
forced termination can leave an empty file, which is not admitted.

The 64-byte little-endian header starts with `MNMDHI01`. DWORDs 2–9 are version
1, row bytes 128, build marker `40209ca7`, capacity 4096, actual row count, dropped
count, owner thread and minimum sample interval 50 ms. DWORDs 10–15 are zero.
The marker is not an executable hash; staging separately pins the complete
No-CD executable SHA-256. Any dropped sample rejects journal admission.

Each row has 32 little-endian DWORDs:

| Word | Meaning |
|---|---|
| 0–3 | One-based ordinal, GetTickCount milliseconds, owner thread, observed menu screen |
| 4 | Validity bits: 1 candidate globals; 2 pool shape/base; 4 pool-slot-validated actor; 8 readable camera fields |
| 5–6 | Candidate hit class at `6a5134`, draw-entry index at `657d40` |
| 7–8 | Observed numeric pool-base ID, capacity (1–256) |
| 9 | Actor slot plus one; zero without actor validity |
| 10–13 | Actor type, owner, active word, health word |
| 14 | Camera orientation at object `689930` +31 |
| 15–17 | Camera whole X/Y/Z at +39/+3d/+41 |
| 18–20 | Camera fractional X/Y/Z at +45/+49/+4d |
| 21 | Candidate pointer classification: 0 null, 1 validated actor, 2 nonnull but unvalidated |
| 22–31 | Zero; reserved |

The candidate pointer comes from `656600`. Actor admission requires a readable
pool header at `6def58`, positive bounded capacity, exact stride `e4b`, an
in-range slot, readable actor bytes and an actor slot word equal to that slot.
No pointer is serialized. Pool IDs increment on observed numeric base changes;
unobserved changes, reuse of the same base, actor generations and actual
allocation ownership remain unproved. Invalid actor data cannot become a hit.

Camera validity establishes readable scalars only. The independently documented
primary scene caller supplies ECX `689930`; current World activation is joined
separately against the semantic sampler. Camera orientation/ranges are retained
as observed words rather than silently normalized. The earlier semantic journal
words named `camera` were actually pacing globals `6dbe95/6dbe99`; these journals
and fields remain separate.

The analyzer pairs only identical thread/tick/menu identities and reports
unpaired samples. It does not interpolate state or infer input causality,
persistent selection, tooltip contents, frame ownership or whole pause behavior.
Physical acquisition, source-owned replay and native parity are independent.
