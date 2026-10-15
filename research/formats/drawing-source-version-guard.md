# Source write-version guard journal

`wine-source-version.bin` is little-endian with a 64-byte header and 128-byte
records. `MNMSVG01`/version 1 admits only closed global write leases.
`MNMSVG02`/version 2 adds explicit open-write boundary records and terminal
counts. Header words 2–7 are version, 128, build 0x40209ca7, maximum 131072 records,
maximum 2048 resource/snapshot slots and 4 mutation kinds; words 8–15 are zero.

| Word | Meaning |
| --- | --- |
| 0–3 | Record ordinal, phase, Win32 thread, GetTickCount |
| 4–9 | Submission, enclosing API, source texture ID/generation, resource ID, subresource |
| 10–13 | Snapshot epoch, current epoch, active writers, owned payload ordinal |
| 14–18 | Exact command, worker, refusal reason, lifetime prefix, mutation kind |
| 19–23 | Destination resource/epoch/writers, current source generation, own destination alias |
| 24–31 | Zero, reserved |

Phases 1/2 are mutation begin/end, 3 snapshot registration, 4 exact emitted packet,
5 worker entry, 6 worker return, 7 terminal, 8 fatal refusal. Wire 2 phase 9 records
an open write lease at the capture boundary. Mutations populate resource word 8,
subresource word 9, current epoch word 11, writer count word 12 and kind word 18, with other payload
fields zero. Kinds 1–4 mean non-read public map, public copy destination,
selected worker destination and public descriptor rebind.

Snapshot/packet/worker rows retain the same submission/API/source/generation/
resource/subresource/snapshot-epoch/payload tuple. Packet command and completed
worker identities are unique. Actual current source generation is checked
against the raw lifetime prefix. Source and destination resource IDs belong to
the resource domain, distinct from texture IDs.

Boundary rows populate resource word 8and current epoch/writers words 11–12 only. They
follow all ordinary events and precede terminal. Terminal word 4 is stop reason,
word 13 is registered snapshot count; wire 2 words 14–15 are total open writers and
resource boundary count. Other terminal payload fields are zero. Every pending
snapshot must have one matched packet, worker entry and return. Every remaining
writer must appear at the wire 2 boundary; malformed, incomplete or refused
journals cannot count as admitted runs.

Refusal reasons 1–10 are active writer after READ map; write during copy/unmap/
retention; texture generation change; duplicate snapshot; snapshot capacity;
missing snapshot; packet/worker identity or subresource; write before worker;
write during worker; pending snapshot at close. Unreportable allocation,
resource capacity, epoch overflow and I/O failures also terminate with exit 99.
Refusal journals remain unclosed negative evidence. No pointer values cross
the wire, and closed observed-version relationships do not prove source bytes
unchanged under unhooked writes or GPU completion.
