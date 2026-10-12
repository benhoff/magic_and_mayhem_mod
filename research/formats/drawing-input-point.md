# Completed original cursor point V1

`DrawingInputPointV1(owner, output, bytes)` is an optional PE32 stdcall export.
It writes exactly 64 little-endian bytes to a validated writable caller buffer.
No COM, actor or host pointer is transported. Invalid owner zero, wrong size,
unwritable output, disabled observation or cache overflow returns zero without
writing. The query preserves LastError and the 512-byte FXSAVE state, takes the
existing journal mutex briefly, and calls no cursor polling function.

`MNM_DRAW_INPUT_POINT=1` enables the bounded cache. Each completed, emitted
original GetCursorPos call updates one of 16 numeric thread slots. Capacity
overflow permanently refuses queries until restart; original calls still forward.
Thread IDs do not prove thread generations or lifetime across reuse.

| DWORD | Meaning |
|---|---|
| 0–2 | Version 1, byte size 64, adapter build marker `40209ca7` |
| 3 | Flags: 1 journal active; 2 last cursor result has valid coordinates |
| 4 | Requested owner thread |
| 5–7 | Completed record prefix, entered-call prefix, physical-plane prefix |
| 8–10 | Drop count, gap count, pending call count |
| 11–13 | Last cursor record, original entry ID, completion GetTickCount |
| 14–15 | Original cursor X/Y words, zero after failure or without coordinates |

An unknown owner returns a valid envelope with no cursor identity. Failed
GetCursorPos retains its record/entry/tick, clears coordinates and omits bit 2.
Inactive, lost or gapped points cannot authorize a current cursor. Pending calls
remain explicit; a completed record prefix is not a global thread freeze or
allocation/physical-frame ownership proof. The marker does not replace the
executable hash verified by experiment staging.

Cross-DLL PE32 fixtures validate original once-only result/error/x87-control
forwarding, query full FXSAVE/error preservation, exact emitted journal links,
owner isolation, failed cursor clearing, output refusal and 17 live owners.
The first fixture's original-FP flag lacked a direct forwarding FP assertion;
the second adds the x87-control assertion. Neither establishes original gameplay
click dispatch or full original call-context equivalence.
