# Threading evidence

Inspected 2026-10-03 using `objdump -p` and `objdump -d` on working
executables. No original artifacts were consumed or modified.

Follow-up static recovery, 2026-10-04: the No-CD message-loop dispatch,
gameplay world-update entry `0x0046afc0`, creature passes, pacing and separate
timer callbacks are now documented in [world tick loop](world-tick-loop.md).
The timer wrapper/IAT addresses below describe the **clean** build only;
No-CD equivalents are `0x00534ad0` / `0x00534b60` and `0x005c52e0` /
`0x005c52b8`. Live gameplay thread ownership remains unobserved.

## Confirmed static findings (high confidence)

- `working/game-clean/Chaos.exe`, SHA-256
  `124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`,
  imports `timeSetEvent`, `timeKillEvent`, `CreateMutexA`, `ReleaseMutex`, and
  `WaitForSingleObject`. It has no named `CreateThread` import.
- Its timer-start routine at preferred VA `0x00521f40` calls `timeSetEvent`
  through IAT slot `0x005a3280` at `0x00521f63`. Arguments are delay from
  object offset 0, resolution from offset 4, caller-supplied callback, user data
  0, and flags 1 (`TIME_PERIODIC`, default function callback). The returned
  timer ID is stored at offset `0x0c`. The routine at `0x00521f90` calls
  `timeKillEvent` through `0x005a327c` at `0x00521f9e`.
- Microsoft documents that [multimedia timers run in their own thread](https://learn.microsoft.com/en-us/previous-versions/ms713423%28v%3Dvs.85%29).
  Thus this timer path allows game callbacks on a supporting thread when
  successfully activated. Its activation and callback identity were not traced.
- The No-CD executable, SHA-256
  `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
  also imports these timer/mutex/wait APIs and has no named `CreateThread`
  import. `LoadLibraryA`/`GetProcAddress` are present, so import absence is
  not proof that no threads can be created.
- The tracked No-CD route-request pseudocode at `0x00512800` directly calls
  search `0x0054b800`, then copies shared result storage at `0x00690148`
  into the requester. This inspected path is synchronous; it does not enqueue
  work to a worker thread. See `decompiled/nocd/raw/00512800.c`.

## Interpretation and limits

A predominantly single-threaded gameplay simulation is plausible, but not
confirmed by the limited functions inspected. A strictly single-threaded
process should not be assumed: the executable contains a threaded multimedia
timer path, and supporting DLLs can create threads. Shared route-search storage
also must not be assumed safe for concurrent calls.

The existing Wine startup snapshot
`working/experiments/opengl-render/run-5bwpx2m5/thread-contexts.txt` lists three
threads, but that is a startup/error-dialog sample under Wine with our runtime
environment. It does not establish how original gameplay work is distributed.
See `render-startup-black-screen.md` for snapshot provenance.

To confirm simulation ownership, trace timer callbacks and gameplay update,
rendering, and route-search entries with thread IDs during actual gameplay.
Use the build-specific observation candidates and scenarios in the
[world-update recovery](world-tick-loop.md#smallest-next-live-observation).
All VAs here refer to the identified executable's preferred image layout;
they are not claims of stable runtime pointers across launches.
