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
