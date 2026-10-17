# Common menu return copy investigation

Inspect the pinned No-CD original around full-copy return address `0x5597fd`,
retaining raw bytes and exact disassembly. The existing shared tick entry
`0x5595d0` has only an eight-byte execution guard; this inspection does not
expand that recovered range or declare a complete function. New branch-body
addresses remain original candidates. Static control flow and actual original
execution require separate evidence. No operation reorder is authorized by a
static call listing.


`draw02-menu-return-copy-static-03.json` retains five bounded windows and exact
15-byte argument/receiver/CALL checks at `0x5597f8`, `0x46b9fc` and `0x4a4c21`.
All immutable inputs verify before and after. This is disassembly, not execution.
Reports01/02 retain earlier tool versions and partial helper windows; no old hash
is refreshed. The current windows use existing inventory bounds for `0x4a4380`
and `0x58c360`; the shared menu body remains undiscovered by that inventory.

The full copy returning to `0x5597fd` occurs on the shared menu redraw branch,
after an indirect slot `+0x28` draw or alternate fade path. This is distinct from
the earlier pop-request branch (`+[0x43]`), which returns before this copy.
The redraw requires `+[8]` nonzero and `+[0x12]` zero. It clears `0x642c60`,
conditionally calls foreground cursor `0x4a45b0`, then copies receiver/source
`0x6a49e0` to destination argument `0x6a4fe0`, calls helper `0x4a4380` and sets
`0x642c60` back to one. These guards are exact instructions; their complete menu,
thread and backing semantics remain hypotheses. Confidence is high for bytes,
provisional for their relationship to the historical failed runtime interval.

The restore helper waits on `0x642c58` and checks saved-background state before
its copy. Its ordinary rectangular route copies receiver/source `0x641fc8` into
`0x6a49e0` through `0x58c4a0`. A separate `0x58d5b0` route remains independently
conditional. Static source/destination wrappers do not prove allocation aliases.

The historical cursor-client report02 reversed source/destination labels at
`0x4a4b01`. In `0x58c360`, EBX retains ECX (source), EDI loads the first stack
argument (destination), and the destination interface call receives the source
surface. Future reports now label source `0x6a4fe0` and destination `0x641fc8`.
Historical report bytes, hashes and any remaining source freshness limits remain.
The runtime recorder already uses this receiver/source convention; this fixes
inspection metadata and changes no engine operation or native replay ordering.
