# Display-only viewport scaling and fullscreen

Implemented 2026-10-06 as native presentation policy (`HOST.presentation`), separate
from recovered engine display rebuild behavior. `GlViewport` retains source
dimensions for frame textures and logical input. One physical-pixel image rectangle
drives both OpenGL viewport placement and Qt input mapping, including odd letterbox
offsets and fractional device-pixel ratios. No engine hooks, executable patches,
preferences or simulation code change.

Policies: sharp nearest-pixel aspect fit (unchanged default), smooth bilinear
aspect fit, and integer enlargement rounded down to whole physical pixels.
Integer mode fits smaller windows rather than cropping the source. Both CPU
frame uploads and integer RGBA GPU leases use the same bilinear calculation in
presentation shaders. Integer GPU textures remain nearest-filtered; source
surface pixels, palette conversion, renderer ownership/fences and command CHECK
semantics are unchanged. Switching display policy does not upload a new CPU frame
or read back native storage.

`ViewportPresentation` owns the fullscreen/scaling controls. F11 is intercepted
before the input adapter, including key release and repeats; Escape stays with
the game. Fullscreen hides selected shell chrome and restores prior visibility,
geometry and maximized state. Chrome stays available until the viewport is
visible, initialized and has a nonempty frame without a presentation error, and
returns if the frame disappears or the viewport is hidden for launch/recovery.
The CLI accepts `--fullscreen` and
`--scaling sharp|smooth|integer` for OpenGL hosting and standalone command/surface
previews. Incompatible embedding/menu/diagnostic modes reject these options.
Default startup and frame presentation remain windowed/sharp.

Validation: `tests/viewport-presentation-test.cpp` independently computes every
framebuffer pixel for all policies on both CPU images and shared GPU leases;
bilinear values permit one byte of floating-point rounding difference. It checks
resizing, source destruction, fullscreen/normal/maximized restoration, hidden
docks, F11 repeat/release consumption, Escape delivery, logical pixel centers,
letterboxes and drag clamping. `tests/qt-input-test.cpp` checks actual targeted XCB
input for each policy. The recorder additionally runs existing complete GPU-frame
and upload regressions, CLI command replay under all three fullscreen policies,
shell startup and incompatible-option refusal at device scales 1 and 1.5.

Successful evidence in `viewport-presentation-20261006.json` records
240 complete framebuffer comparisons and 19 successful subprocess checks
at device scales 1 and 1.5. Historical presentation evidence retains its original
hashes; shared-file changes can make earlier records stale. This is synthetic
native integration, not a new original comparison or live rendering replacement.
Original-game fullscreen interaction, other monitors/backends/device scale factors,
and hardware performance remain pending. Higher output resolution interpolates
existing artwork; new world detail/field of view requires separate rendering work.

The toolbar now uses an explicit text push button before the scaling selector,
labelled Fullscreen (Exit fullscreen when active). Button clicks and F11 keep
the checked state and label synchronized. Button entry/exit and geometry
restoration are included in the presentation fixture; F11 remains available
when active gameplay hides the toolbar.

Fresh button execution is retained in `viewport-fullscreen-button-20261006.json`:
240 complete framebuffer comparisons and all existing CLI/input regressions pass.
The earlier display-policy result remains immutable historical evidence.

The standalone renderer `command-replay-test-shell` must compile
`apps/qt-shell/viewport_presentation.cpp` alongside the replay implementation.
The initial fullscreen change omitted that target dependency, so the default
launcher full build failed to link even though `mnm-qt-shell` itself built.
The explicit dependency is now added; launcher-wide build, standalone synthetic
replay and native-command shell startup are checked separately from original
game execution. Evidence: `viewport-presentation-link-fix-20261006.json`.

Fullscreen readiness correction: native-command startup shows an empty GL widget
to initialize its context. The former visibility-only check hid launch controls
and logs before a frame existed. A synthetic game-shaped window reproduces that
failure (`working/tests/viewport-presentation/black-before-fix.stderr`). The
controller now checks frame readiness; chrome/layout mutations are deferred until
the current Qt event/composition completes, followed by a redraw after layout
settles. This is presentation policy, not a recovered engine contract.

The fixture additionally samples the actual composited window without forcing a
framebuffer grab first: fullscreen startup, retained and changing shared GPU
frames, repeated normal/fullscreen transitions, and controls returning after the
frame clears. Xvfb uses an explicit 1920×1440 display so the tested window fits;
the default 640×480 virtual screen clipped captures and was a test setup issue.
Evidence is retained separately in `viewport-fullscreen-ready-20261006.json`.
Original-game transitions and the user's graphics driver remain unverified;
passing synthetic composition does not establish the cause of every black screen.

Fullscreen shortcut update (2026-10-07): F9 now toggles fullscreen, including
exiting during gameplay with hidden chrome. Unmodified F9 press/release and
shortcut override are reserved by the presentation controller; autorepeat does
not toggle. Earlier F11 records remain historical. Original-game and user-driver
interaction remain pending; the existing synthetic fixture now exercises F9.

Fresh F9 execution: `viewport-presentation-f9-20261007.json` retains successful
Xvfb/Mesa normal and high-DPI tests, 240 complete framebuffer comparisons,
F9 consumption/repeat suppression, geometry/chrome restoration, input and
fullscreen CLI regressions. The shell and standalone replay targets rebuilt.


2026-11-07 input alignment correction: Sharp and Integer input now identifies the physical display pixel under the pointer and maps its centre to the displayed source texel. The former corner mapping could pick the neighboring texel at noninteger fit scales. Exact physical borders survive fractional-DPR round trips through one-ULP normalization. Smooth continuous input retains its prior policy. The GPU-backed preimage, corrected CPU/GPU framebuffer and XCB forwarding controls are in [the interaction closeout](rendering-interaction-20261107.md); earlier evidence and hashes remain historical.

2026-11-18 FPS display: the Show FPS toolbar toggle and unmodified F10 control
a mouse-transparent, nonfocusable label at the native viewport's top-left.
It starts disabled. While enabled, a one-second monotonic sample counts completed
Qt frame swaps only when the viewport is visible, initialized, nonempty and free
of presentation errors. This is host presentation FPS, including composition
redraws, rather than distinct simulation frames or physical scanout. An idle
viewport can still swap for UI composition. Disabling stops sampling; hiding or
clearing the viewport resets the sample and hides the label. The control remains
available through F10 in fullscreen, consumes press/release/shortcut override,
ignores autorepeat and leaves modified F10 and other windows alone.

The presentation fixture additionally checks a timed stream, idle-rate decay,
default/toolbar/fullscreen toggling, transparent input, reserved key handling and
hidden/empty frame recovery at normal and fractional DPI. These synthetic host
checks do not establish original-engine, physical-driver or hardware timing
equivalence. Existing engine evidence and its source-drift limitations remain
historical.


2026-11-19 gameplay FPS: the overlay has separate `FPS` and `Gameplay FPS`
lines. The latter samples a session-owned 32-byte version-1 sidecar alongside
the menu channel. The existing guarded original World wrapper at 0046afc0
reads the world counter at 006c4830 immediately before and after its once-only
original call. Only zero/one advances enter the cumulative counter; unexpected
jumps or discontinuity change an epoch and invalidate that rate interval. Both
counters wrap modulo 2^32. Seqlock publication prevents mixed samples. Mapping
setup happens once, no per-tick I/O or drawing counters feed the rate, and the
wrapper preserves the original return value and LastError. This adds no game
clock, update or pacing changes. The optional mapping survives until process
exit; the host closes its view on stop/restart and creates a fresh sidecar for
each new session. Missing/invalid/unobserved data displays a dash. Unchanged
counters settle to zero even while the viewport redraws.

Synthetic normal/fractional-DPI display controls exercise independent 50 Hz
redraw and 20 Hz gameplay sources, stopped drawing with continued gameplay,
paused gameplay with continued drawing, unavailable telemetry, counter wrap,
unknown jumps, malformed versions and torn samples. PE32 synthetic World
forwarding verifies five original counter advances, once-only forwarding and
LastError/return preservation. Real-game telemetry, broader World/mode/reset
semantics and physical hardware timing remain pending; historical engine
evidence is not renewed by these checks.

The reproducible `tools/test-gameplay-fps.py` driver pins sources and prospective
contract/scenario claims before the focused controls. Its bound result is
[validation-bound-02.json](gameplay-fps-20261119/validation-bound-02.json); the
initial unbound controls and sandbox setup failures remain in validation-01.
