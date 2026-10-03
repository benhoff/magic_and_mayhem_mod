# Real-game black-screen startup investigation

## Confirmed evidence (2026-10-03)

User run: `working/experiments/opengl-render/run-2lazh703/`.
Launcher log: `working/logs/run-20261003T154714Z.Tfdjsx/`.
The frame header has status 4 (old bridge loaded/waiting), sequence 0, frame
counter 0 and width/height 0. `draw-capture/events.bin` contains only its 16-byte
header. No blit checkpoint or history exists. Therefore this run has not reached
an observed surface operation or published any pixels; there is no evidence
that flip replay caused the black screen.

The log records repeated Mesa EGL driver-initialization warnings for NVIDIA PCI
ID `10de:1e02` and Wine pixel-format fallback messages. These are clues, not proof
of the cause. The staged executable's image base and guarded thunk match the
expected values (`0x00400000`, thunk `ff 25 14 50 5c 00` at RVA `0x19755a`). The
old loaded status does not prove that the runtime hook was installed or called.

On the same X11 display (`:1`), glxinfo reports NVIDIA TITAN RTX, NVIDIA 610.57.04,
and OpenGL 4.6. The Qt `--opengl-test` known-pixel fixture exits successfully on
that display. With software rendering selected, glxinfo reports Mesa llvmpipe
LLVM 22.1.8 / Mesa 26.2.1 and the same Qt pixel fixture passes. The required Mesa
and NVIDIA GLX/EGL libraries are present in both `/usr/lib` and `/usr/lib32`.
Thus Qt presentation and a software GL alternative work on this host; Wine's
actual game startup remains unverified.

## Implemented diagnostic/retry path

The bridge now distinguishes DLL load, failed hook installation, armed hook,
entry into DirectDrawCreate, success, HRESULT failure and failed interface
interception. It records the last HRESULT and intercepted create-call count.
Qt shows hard failures immediately and startup-stage diagnostics after ten
seconds without a frame. Older status-4 writers remain readable. The fixture
checks successful, failed and null-result creation while preserving arguments,
HRESULT and entry/final LastError. Existing surface/frame tests still apply.

Run a fresh shell after closing the failed game through its normal controls:

```bash
./tools/run-qt-shell.sh --software-rendering --capture-history
```

This selects Mesa before Qt creates a graphics context, using
`LIBGL_ALWAYS_SOFTWARE=1`, `__GLX_VENDOR_LIBRARY_NAME=mesa`, and the installed
Mesa EGL vendor JSON if present. The Wine child inherits this environment;
no prefix registry or system driver configuration is changed. Experiment metadata
records the selected graphics variables. The shell still waits for **Launch game**.
The option is a diagnostic fallback, not a confirmed fix for this game's startup.

Environment behavior is documented by [Mesa](https://docs.mesa3d.org/envvars.html)
and [GLVND's EGL vendor discovery](https://github.com/NVIDIA/libglvnd/blob/master/src/EGL/icd_enumeration.md).
For a hardware comparison, omit `--software-rendering` in a new shell.

If it still stalls, the new placeholder/status message separates a never-called
hook (status 5) from initialization inside Wine (6), a DirectDraw HRESULT (8),
hook incompatibility (9/10), or initialized DirectDraw with no observed primary
presentation (7). These cases require different follow-ups. No automatic
real-game relaunch was performed in this investigation.

Validation report: `working/tests/render/run-c9vopdrc/report.json` includes all
three startup fixtures and the existing Wine/native/Qt frame and draw checks.
All eight Qt shell CTests passed, including the explicit software-rendering
fixture and frame-reader diagnostic coverage. Production PE32 builds succeeded.

## Follow-up: armed hook with no calls

The user's software-rendering run `working/experiments/opengl-render/run-rh6zsxhs/`
has status 5, zero create calls, zero frames and an empty event inventory. The
user reports a black window throughout and subsequently closed it. A live stack
could not be obtained because Chaos.exe was no longer running. No new full-game
launch was performed automatically.

`./tools/test-render-import.py` now stages a **fixture**, not a playable game:
it verifies the pinned source SHA, adds the same bridge import, and replaces the
entry point with a small scripted sequence of API calls followed by ExitProcess.
The original game entry and game loop never run. The original source is checked
unchanged afterward. This tests the real PE base, thunk/IAT locations and loader
ordering that the older mock fixtures bypassed.

The original creation-only fixture passed at
`working/tests/render-import/run-_ckhm_uj/report.json`. Expanded fixtures passed
both imported EnumerateA and dynamically resolved EnumerateExA, followed by
DirectDrawCreate, under Xvfb/software rendering. The same sequence also passed on
the active NVIDIA display at `working/tests/render-import/run-68kjmrix/`. Thus
an import overwrite or universally broken adapter initialization has not been
reproduced. The actual game's earlier initialization still needs a live stack.

Static evidence: `0x0058f0c0` resolves DirectDrawEnumerateExA with GetProcAddress
(IAT `0x005c50d4`), calls it at `0x0058f102`, or falls back to the imported
DirectDrawEnumerateA thunk `0x00597560` at `0x0058f10c`. Actual drawing setup later
calls DirectDrawCreate at `0x0058a9c3` / `0x0058aaa4`. The game creates drawing
surfaces at startup `0x004e986a` before optional Intro0/Intro1 movie calls around
`0x004e992b` / `0x004e996c`; skipping movies is not justified as a fix for a
never-called drawing setup hook.

The bridge now observes both enumeration paths without changing their callbacks
or flags. Its GetProcAddress hook substitutes only the named EnumerateExA export
from ddraw.dll. Expected creation/enumeration thunks and the pinned image base are
checked before any import writes. Qt reports enumeration in progress, completion
or failure, and replaces a stale waiting placeholder when the launcher ends.
Microsoft documents the dynamic API lookup in
[DirectDrawEnumerateExA](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-directdrawenumerateexa).
Confidence: confirmed static calls and controlled API/loader fixtures; the
specific real-game stall remains unconfirmed.

Reopen the updated shell with the same software/capture command above. If the
black screen recurs, leave the Wine window running so a live stack can be read.
Changing more startup settings without that evidence would be guesswork.

Final callback/context fixture evidence:
`working/tests/render-import/run-6sehwgda/report.json` (software/Xvfb) checks that
both real enumeration callbacks ran before the intercepted creation. Existing
bridge regression evidence: `working/tests/render/run-0_vw3pl_/report.json`.
All eight Qt shell regression CTests pass with the extended diagnostic reader.

## Follow-up: frames captured, then a surface ownership error

The user's `run-g34xhs0b` manifest points to
`working/runtime/render/frame-6858c311-72fe-474c-b712-c606e34becc3.bin`.
Its header records one successful DirectDrawCreate, status 1, and 49 published
800x600 RGBA frames. This is a later failure than the zero-call run above.
The recorded graphics variables are empty: this run did not select the explicit
software-rendering fallback. Its capped event inventory has successful surface
creation, locks/unlocks and blits. History ends with GAP reason 6 (unsupported
operation), not reason 1 (concurrent history access).

The user recalls an error inside the Wine window about another thread owning
rendering. The pinned executable contains error descriptions saying access to a
surface or palette is refused when already locked by another thread. This matches
the recollection but does not establish the actual HRESULT or offending call.
The Wine log `working/logs/run-20261003T162725Z.YPJcO9` records Quartz media-type
failure `0x8007000e`, allocator decommit waiting and sample failure `0x80040211`.
Movie decoding is therefore a candidate; capture readback also briefly locks
surfaces, and its involvement has not been ruled out. No ownership checks have
been removed or application calls serialized as a speculative fix.

`--skip-movies` now sets `PlayFMV` and `PlayFMVOut` to FALSE only in the disposable
experiment's `[VIDEO]` preferences. It updates both plaintext and encrypted
preferences if present, validates both before writing either, verifies encoded
round trips, and records before/after hashes in the experiment manifest. It
changes no source installation or game balance settings. Start a fresh shell:

```bash
./tools/run-qt-shell.sh --software-rendering --skip-movies
```

Click **Launch game**. Leave draw/history capture off on this first retry to
remove its additional observer locks. The frame bridge still performs readback;
this is not a capture-free baseline. If this succeeds, repeat with
`--capture-history` to separate movie and observer interactions. If it still
hangs, keep the Wine window open for inspection. The bypass is a diagnostic
workaround, not a confirmed resolution of the underlying ownership failure.

Validation: `tests/render-movie-preferences-test.py` covers independent plaintext
and encrypted edits, preservation of unrelated settings/comments/line endings,
idempotence and rejection of ambiguous preferences without partial writes.
Stage-only experiment `run-ea231lr3` verifies the actual working preferences and
unchanged source hash without launching the game. All eight Qt shell CTests pass.
The user's `commands-0001.bin` replays successfully through Qt/OpenGL under
Xvfb/software rendering with framebuffer sample checks. This validates the
captured checkpoint, not uninterrupted gameplay or the incomplete history.

## Follow-up: movie bypass does not resolve startup

User runs `run-f9hulkut` and `run-g8y5d95e` both record the movie bypass,
software GL environment, no history capture, status 5, zero frames, zero
DirectDrawCreate calls and zero observed enumeration calls. Their initial logs
are empty beyond launcher messages. The user confirms a fresh plain
`./tools/run-game.sh` reaches the menu. Therefore the bypass is not a resolution;
the replacement launch path must be investigated before further movie tuning.
The plain comparison does not yet separate staging, import hooks, desktop
wrapping and the software-rendering environment.

A live WineDbg attachment to Wine PID 0x13c actually targeted the newer
`run-g8y5d95e`, not the already closed earlier run. Its initial snapshot stopped
on a newly created thread 0x188 at inaccessible EIP `0xffbb10ec`, and the launch
log subsequently recorded that page fault. No original game-thread stack was
collected. The timing and new thread make a debugger-induced attachment fault a
candidate, not proof of the original hang. Evidence is in the earlier run's
`live-debug/debugger.log`; the second attempted attachment records access denied
in the newer run's `live-debug/debugger.log`. Native GDB inspection was blocked
by Linux ptrace permissions. A temporary PE32 context-reader could not inspect
the target after it closed; it supplies no evidence about the startup cause.
No automatic full-game relaunch was performed.

`tools/debug-render-startup.py` prepares a copied PE32 i386 WineDbg and launches
an existing hash-verified staged executable from the outset. It checks both game
and bridge hashes and the mapped stream, restores the experiment's recorded
graphics environment, and leaves extra draw/history capture off. Its debugger
DLL override forces the copied x86 executable. It does not attach to a live game,
change system/prefix settings or patch another binary. It omits the explorer
desktop wrapper, an explicit diagnostic difference to keep in the comparison.

After closing other game/debugger windows, run:

```bash
./tools/debug-render-startup.py working/experiments/opengl-render/run-g8y5d95e
```

The tool prints debugger commands for startup `0x004e8d80` and drawing setup
`0x004e986a`. At stops, record `bt` and `info reg`; use `cont` to reach the next
stop. Avoid Ctrl+C on this WoW64 runtime: it also produced the inaccessible
`0xffbb10ec` debugger-thread fault described below. If already stopped there,
use `info thread`, select an original game thread with `thread 0xID`, and collect
`bt`, `info reg`, and `x /32x $esp`. Use `detach`, then `quit` to leave the game running. Terminal
output may be saved using `script`; preparation metadata is in `startup-debug/`.

Validation: `--prepare-only` verifies the actual staged game without launching
it. The copied PE32 debugger launched the pinned-image API-only probe, read
`$eip`, obtained a stack and continued to a normal exit under Xvfb in
`working/tests/render-import/run-6sehwgda/startup-debug-test/`. The probe does not
execute the game loop. This validates the debugger launch path, not the cause or
resolution of the user's hang. Confidence in the startup root cause remains low.


## Follow-up: startup breakpoints work; debugger interrupt faults

The user launched the copied x86 debugger and reached both startup breakpoints.
At `0x004e986a`, ESP is `0x0022f9f4` and EBP is `0x0000010a`; EBP is not a
usable frame-chain pointer, so the single-frame WineDbg backtrace does not prove
stack corruption. After continuing, the user reports a menu with incorrect
colors and a freeze. The mapped header at inspection records status 2 (primary
surface readback Lock failed), one successful creation and zero published frames.
There is no evidence that Qt pixel conversion produced those menu colors.

The user's subsequent Ctrl+C produced `0xffbb10ec` on a new stack at
`0x06eeff44`, with only ntdll/thread startup frames. This repeats the attachment
fault and is temporally associated with the debugger interrupt. It does not
identify the original game thread's stall. Confidence: high that interrupt and
attachment inspection are unreliable in this runtime; the underlying game freeze
and primary Lock HRESULT remain unresolved. The launcher now warns against
Ctrl+C and explains how to select an existing game thread if already stopped in
this fault. `info thread` lists threads; bare `thread` is not a listing command.
