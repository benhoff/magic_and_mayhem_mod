# Recorder-only whole-update producer stream V4

V4 is an opt-in diagnostic source protocol, not a recovered original file format.
It retains producer operations after the requested final World queue return until
the next accepted-update marker. Legacy V1/V2/V3 envelopes and replacement
budgets retain their existing validation. Original pixel oracles remain separate.

The 64-byte header begins `MNMPRO04`. DWORDs 2–8 are version 4, header size 64,
build tag `0x40209ca7`, record limit 1,048,576, requested queues 1–120,
whole-update flag 1 and byte limit 536,870,912. DWORDs 9–15 are zero. These finite
limits are native diagnostic policies. Source definitions retain the existing
V4-only4,096-entry / 16 MiB cache (legacy V3retains2,048 entries) and raw-input fallback. Oracle files keep their existing
1–3 GiB configured budget and 9,999-file refusal; the protocol does not waive them.

Existing 96-byte producer rows and owned operations 1–26 retain their field
contracts. New operation 27 ends the source interval with no raster effect.
It has no payload and fields 3–13 are zero. Field 14 is the completed queue
count, 15 the last accepted sequence, 16 the counter before the next update,
17 the next accepted sequence (unsigned last+1), 19 the exact cadence event
ordinal, 20 its mode byte, 21 the engine owner thread and 22 flags 3 (no pending
source frames and no recorded failures); field 23 is zero. No records may follow
this marker. Complete decoding requires matching entry/return counts and this
final marker. Partial prefix decoding remains explicit.

The metadata producer/update side journal retains each exact queue event point.
Offline whole-update analysis additionally matches operation 27 to the original
cadence accepted-update event and last bound queue. Completion marker
`canvas-producers.done` retains its existing 32-byte count/byte-length provenance fields.
The marker establishes this recorded source boundary, not whole physical output,
hardware completion, source lifetime ownership or semantic journey coverage.
