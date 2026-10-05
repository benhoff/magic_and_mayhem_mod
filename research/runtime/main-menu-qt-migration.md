# Main menu migration to a Qt widget: assessment

## Subsequent live integration

The bounded Main/Quick action bridge is now implemented separately; see
[the contract and current validation](menu-action-bridge.md). The assessment
below preserves the earlier standalone milestone and its original boundaries.

## Native widget milestone (2026-10-04)

`apps/qt-shell/main_menu_widget.*` now implements the first bounded step below.
`--main-menu` previews the installed JPEG and CFG layout/labels without launching
the game. Reads use AssetStore with size limits; unsuccessful loads retain the
prior widget state. Original font rendering is approximated with a system serif
font. CommandLine Battle remains hidden unless explicitly enabled by the caller;
version text is caller-supplied. `actionRequested(Action)` exposes semantic
intent without engine addresses or process ownership in the widget.

`tests/main-menu-test.cpp` uses only temporary synthetic CFG/JPEG assets. It
checks label lookup, six action signals, Space activation, conditional visibility,
wide/tall geometry, JPEG display/letterbox pixels, invalid rectangles, corrupt
images, missing roots and preservation of prior state after failed reloads.
This is native UI/synthetic evidence, not original menu behavior equivalence.
The preview closes on Quit; a live controller must instead request normal engine
exit. Engine observation, menu adapter and live transition validation remain
outstanding. The assessment below describes that remaining work.

Validation: all 20 Qt-shell/renderer/asset CTests passed after permitting Xvfb
local display sockets (the sandboxed GUI runs could not create listeners).
The installed-asset offscreen `--main-menu --smoke-test` passed. A standalone
Xvfb preview was visually inspected and captured at
`working/tests/main-menu-preview/preview.png`; no game process was started.
Original-manifest verification passed before asset use and after the preview.
System-font appearance is confirmed only for this host.

## Initial assessment

Assessed 2026-10-04. Scope assumes the original game's title/main menu;
the shell's pre-launch screen is already a QWidget/QLabel with Qt toolbar
controls. No game launch, hook installation, binary patch or implementation
was performed for this assessment.

## Existing foundations

- `apps/qt-shell/main.cpp`: QMainWindow, process launch/check handling,
  viewport, logging, frame polling and launcher-exit cleanup.
- `apps/qt-shell/input_forwarder.hpp`: input release/suspension for changing
  ownership between native UI and the game.
- `apps/qt-shell/media_broker.*`: movie playback and completion handling;
  the media channel is a useful lifecycle pattern, not a menu-action protocol.
- `assets/`: read-only loose-file resolution/access. The shell does not
  currently link this module; native menu loaders would need build wiring.
- [Main menu layout inputs](../formats/main-menu-layout.md): two installed
  JPEG backgrounds, text rectangles and label IDs. A plain Qt font would
  permit an initial widget without implementing SPR/SFT typography.

There is no established semantic menu-action adapter or recovered menu-state
contract in the inspected source/research. Existing channels carry frames,
input state and media requests, not New Game/Load Game commands. Pinned No-CD
executable strings reference the exact main-menu CFG path; the menu function,
calling convention, state variables and action dispatcher remain to be found.

## Proposed bounded implementation

1. Build `MainMenuWidget` in `apps/qt-shell/` with the installed JPEG, labels,
   focusable buttons and aspect-preserving layout. Initially expose semantic
   signals to a fake controller. Decide whether exact original fonts are
   required; SFT decoding/palette/glyph metrics need separate validation.
2. Give the shell explicit startup/movie/menu/legacy-screen/game/error states
   and a stacked central widget. Today each received game/movie frame directly
   shows the GL viewport; those callbacks must respect UI ownership. Keep
   consuming frames as appropriate without letting them displace a native menu.
3. Recover menu entry/exit and button dispatch for the pinned executable using
   static cross-references to the CFG path, followed by bounded observation.
   Record executable hash, addresses, expected bytes, ABI, original return
   values, thread ownership and behavior of each action. Observe before replacing.
4. Add an opt-in PE32 menu adapter and a versioned Qt command/state channel.
   Execute accepted actions at a verified engine-thread point, with request IDs,
   acknowledgements, state generation, duplicate/stale rejection and bounded
   disconnect behavior. Do not call game functions directly from the Qt process.
   Hook installation must verify the executable and expected bytes; pointer
   values must be discovered for each run. Preserve an original-menu fallback.
5. Validate widget behavior and synthetic channel/ABI behavior independently,
   then opt into live transition checks before suppressing original menu work.

Native buttons could provisionally drive original menu clicks through the input
bridge, but this would require new dispatch wiring and verified live coordinates,
focus and menu readiness. It would be an input automation prototype, not a
semantic replacement; current forwarding is tied to viewport events. It should
not become the permanent state/action contract.

## Scope and validation

First replace only the main menu. Delegate New Game setup, Load Game, Quick
Battle and Preferences to their existing screens; return to the Qt menu on
verified engine transitions. These actions can open further screens, so do not
assume that selecting New Game immediately loads a map. CommandLine Battle
visibility and activation require investigation. Quit must use the engine's
normal exit path and wait for launcher completion, consistent with the shell's
current close guard. Existing save/config persistence can remain in the engine.

Check label/layout mapping, resizing/high DPI, keyboard activation, input release
and focus transfer. Synthetic adapter checks should exercise unavailable or
mismatched hooks, stale/duplicate actions, acknowledgement timeouts, lost hosts,
and fallback without executing an action twice. Live evidence must cover each
visible action, cancellation/back, movie completion/skip, return from gameplay,
normal exit and adapter failure. Do not use image recognition alone to infer
menu readiness. Rendering startup/capture limitations remain independent work.

Record offline reconstruction, synthetic checks, live observation and live
replacement separately in [the coverage ledger](coverage-ledger.md), whose
GP09 row retains original interface logic and records the standalone widget
milestone separately from live replacement. Gameplay balance is outside the migration.

The native visual prototype is a small UI task. Reliable integration is the
larger reverse-engineering task; an implementation schedule would be speculative
until the menu dispatcher and state boundaries are recovered. Full migration
of save dialogs, preferences and campaign/battle setup is additional scope.

If the intended target is only the shell's pre-launch landing screen, use step
1 with launch/check signals connected to existing process handling and return
to that widget on launcher failure/exit. No game menu adapter is needed.
