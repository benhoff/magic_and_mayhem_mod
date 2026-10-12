# Original cursor mutex call observation

`MNMDMX01` version 1 is a separate optional PE32 observation module. Capture
enables it with `--cursor-mutex`, which requires original cursor-point capture
and is excluded from counter-only controls. Import staging pins the original
No-CD executable hash and leaves every original section byte unchanged.
At runtime, module base and four IAT cells must match their resolved original
Kernel32 exports before the private IAT cells are changed.

The observer forwards each original `CreateMutexA`, `WaitForSingleObject`,
`ReleaseMutex` and `CloseHandle` call once. It preserves stdcall arguments,
the raw result, LastError and 512-byte FX state. No wait or input poll is added.
It queries `DrawingInputPointV1` outside its recorder critical section, then
records bounded memory rows. The recorder lock never spans an original call.

The 64-byte little-endian header is magic (8 bytes), then 14 DWORDs: version 1,
row size 128, build marker `40209ca7`, capacity 65536, emitted rows, dropped
rows, pending calls, call count, identity-cache overflow, highest created ID,
and four zero words. Complete decoding requires zero drops, pending calls and
identity overflow, and exactly two rows per call. Normal module/process teardown
writes the memory buffer; missing or forced teardown is not completion evidence.

| Row DWORDs | Meaning |
| --- | --- |
| 0–6 | Row ordinal, call ID, operation (create/wait/release/close = 1/2/3/4), entry/return phase, thread, tick, original caller RVA or zero if outside code range |
| 7–10 | Numeric mutex ID or zero for unknown, initial-owner/timeout parameter, raw result (creation represented as nonnull), LastError |
| 11–12 | Create security/name nonnull indicators; pointers are not serialized |
| 13–14 | Reserved zero |
| 15 | Point query success/refusal (1/2) |
| 16–31 | Exact V1 API/cursor point, or zero on query refusal |

The bounded 64-cell identity table assigns IDs to successful observed creates,
retires them after a successful observed close, retains failed closes and gives
a new ID when a retired numeric handle is reused. Unknown handles remain zero.
Named or external aliases and concurrent close/use remain unresolved; these IDs
are recorder identities, not proof of underlying kernel object uniqueness.

The decoder pairs returns with their exact entry call IDs and arguments,
permits interleaved completion order, checks count/reserved/point identities,
and preserves untracked calls. Raw Wait/Release responses are uninterpreted.
Constructor/call observations do not establish ownership, abandoned handling,
complete lock ordering, stable source bytes or safe added capture synchronization.
Sampled API prefixes precede observer row publication; they are not atomic
call-entry/frame identities. No pixel/native equivalence or drawing suppression
is claimed.
