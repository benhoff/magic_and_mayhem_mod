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
