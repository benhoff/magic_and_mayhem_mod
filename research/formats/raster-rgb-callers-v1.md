# RGB caller observer, version 1

`raster-rgb-callers.bin` is a bounded 512-byte little-endian record written by
the owned PE32 debugger. It is separate from the unchanged 256-byte CPU proof.
The file uses CREATE_NEW. Complete success requires the same normally exited
child and all three CPU initializer observations. Failure/partial files grant
no execution credit. No file is written by a game drawing callback.

The first eight bytes are ASCII `MNMRGB01`. The next 62 DWORDs form the header:

| DWORD index | Meaning |
| --- | --- |
| 0..6 | version 1, size 512, complete, error, child exit, PID, initializer TID |
| 7..9 | observed RGB site hits, eligible empty hits, stored sample count |
| 10..11 | original prepared site `0x545ce9`, capacity 8 |
| 12..61 | reserved, zero |

Eight fixed 32-byte samples follow. Each has eight DWORDs: one-based hit
sequence, prepared bottom (EAX), top (EBX), ESP, EFLAGS, signed right, signed
skipped-left, and TID. Only eligible empty geometry is sampled. Sample order is
strictly increasing; stored count is `min(empty hits, 8)`, and unused rows are
zero. Eligibility matches the bounded no-op geometry, including positive rows,
0..2048 top/bottom/skip, -2048..2048 right and right <= skip.

The optional fourth instruction hardware breakpoint observes only the original
initializer thread. It reads three prepared stack DWORDs at ESP+0x28, including
skip and right; it reads no source descriptor or pixels. General registers and
prepared inputs remain unchanged. RF/debug-register updates resume execution.
Counts remain in the debugger's memory and are flushed on completion/failure.

An observer record alone cannot establish native admission. Actual-game evidence
must also prove candidate selection, exact equality with the RGB producer epoch,
zero forwarding, accepted normal shutdown and matched PID/TID. Fixture RET stubs
validate sampling without granting original-game caller credit. Runtime stack
and thread identities are observed each launch, never assumed stable.
