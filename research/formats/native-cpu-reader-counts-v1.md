# Native CPU reader counts V1

`native-cpu-read-counts.bin` is a48-byte process-local diagnostic emitted at
successful joined shutdown (explicit RenderShutdown or automatic RenderStop),
after application admission closes and the
publication worker joins. Successful readers update memory counters only;
diagnostic file I/O never runs inside reader, copy or tracker ownership. An
interrupted run may have no counter file and receives no completion credit.
It has12 little-endian DWORDs:

| Word | Meaning |
| --- | --- |
| 0–1 | ASCII `MNMRDC01` |
| 2–3 | Version1, byte size48 |
| 4 | Last completed CPU_READ ticket |
| 5 | GetDC snapshots |
| 6 | Typed reader snapshots inside a writable Lock |
| 7 | Partial-WORD Unlock merge snapshots |
| 8 | Other typed snapshots outside a Lock |
| 9–11 | Zero |

The four caller counts sum to the last completed ticket and must match the
retained64-byte native CPU reply header. The file contains no process pointers.
It is per-launch acquisition metadata, not a new command-stream wire version.
Ordinary World imports/returns do not increment these counters. Diagnostic GL
presentation comparisons are counted separately by their test consumer.
