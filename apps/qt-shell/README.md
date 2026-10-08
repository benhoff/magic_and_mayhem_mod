# Magic & Mayhem Qt shell

Ordinary launches use native capture/replay presentation. The original game
continues its simulation and drawing; complete live drawing replacement is
unavailable. The [authoritative replacement plan](../../docs/live-drawing-replacement.md)
defines the remaining work and criteria for enabling a complete launcher mode.

A standalone continuous native World shadow viewer is available with:

```bash
./tools/run-native-world.py
./tools/run-native-world.py --verify
```

It renders owned World requests from native sprite assets into the Qt GPU
viewport. Use the original window for menus and input. Ordinary mode uses an
independent zero background and performs no CPU pixel readback; verification
compares a separate original oracle and refuses differences. Some live canvases
contain pixels outside that reconstructed initial state, so full baseline
equivalence and whole-scene bypass remain pending. See the
[implementation](../../research/runtime/native-world-live-rendering.md) and
[initial-background boundary](../../research/runtime/native-world-live-background-gap.md).

An explicit partial sprite raster MVP is available with:

```bash
MNM_WORD_SPRITES=takeover ./tools/run-qt-shell.sh --frame-readback --skip-movies
```

It replaces fully in-bounds direct-word SPR drawing with the native CPU backend.
Indexed sprites, clipping, auxiliary passes and other frame producers retain
original drawing. Use `MNM_WORD_SPRITES=shadow` for native/original comparison.
The launch log identifies the mode; the fresh render experiment contains its
manifest, bypass/fallback counters and eight independent comparison samples.
See [scope, tests and remaining work](../../research/runtime/native-word-sprite-mvp.md).

## Fullscreen and display scaling

```bash
./tools/run-qt-shell.sh --fullscreen --scaling smooth
```

Click **Launch game** to use experimental native command/GPU capture/replay by
default. Add `--frame-readback` to use frames copied from the original renderer.
The **Fullscreen** toolbar button or **F9** enters/exits fullscreen and
restores the previous window size, maximized state and launch log visibility.
Fullscreen hides the shell toolbar, log and status bar while the viewport is
active; launch/recovery controls remain available while waiting for a frame.
Escape keeps its game
meaning. Exit fullscreen to change scaling in the toolbar.

Choose `--scaling sharp` (default) for nearest-pixel aspect fit, `smooth` for
bilinear aspect fit, or `integer` for whole physical-pixel enlargement with
letterboxing. Integer mode falls back to aspect fit when the window is smaller
than the source frame. Resizing and high-DPI displays change only the output
size; input maps back into the original logical canvas. The engine's resolution,
simulation, field of view and assets are unchanged. Upscaling does not add
source detail, and larger output can increase GPU presentation cost.

These controls apply to the OpenGL shell, `--commands` replay and `--surface-demo`.
Wine-window embedding (`--renderer native` / `--live-menus`) retains its existing
fixed viewport. See [presentation policy and validation](../../research/runtime/viewport-presentation.md).

## Live Main / Quick Battle menus

```bash
./tools/run-qt-shell.sh --live-menus
```

Click **Launch game**. Startup may take a few minutes while the launcher verifies
the original files, stages a disposable installation and runs launch preflight.
The viewport displays startup progress; the launch log shows completed checks.
The shell stages a fresh hash-checked installation and
connects native Main Quick Battle/Preferences/Quit, Quick Battle Create Single Player/Cancel,
and Single Player Map/Cancel/Start to the original engine callbacks. Setup uses
the actual map, players, rules and handicaps. Human portrait/colour cycling and
opponent removal retain the original actions; opponent portraits/colours are
read-only. Edits apply on Map, a player action, or Start. Other actions use the
original menus. The shell waits for engine-confirmed readiness.

**Use original menus** exposes the Wine viewport and permanently retires the
command channel for that session. The native Main Quit button and window close use the original Quit callback;
closing from Quick Battle returns to Main first. Closing from Preferences cancels its draft before returning to Main and quitting. After choosing original menus,
exit through the game before closing the shell. Relaunch stages a new copy/channel. This mode uses the Wine viewport;
it is separate from OpenGL capture and the standalone preview options.
Original menu logic, fades and drawing remain active.

**Start** opens the native pre-battle spell selector when required. It reads
the engine inventory, recipes and talismans. **Reset edits** restores the local
draft; **Start battle** commits it through original callbacks and exposes the
original loading/play viewport. The original selector has no cancel-to-setup
action. Commit before the original timer expires; unsubmitted drafts stay
local. Closing from selection exposes the original controls. Campaign/multiplayer
selection remains outside this bridge.
Battle and result controls use the original viewport. When the engine returns
to Main or Quick Battle, Qt automatically restores that menu with fresh state.
Choose **Use original menus** to disable restoration for the session. The viewport stays at 800x600
inside the shell to avoid stale pixels from enlarging the Wine desktop.
See [Single Player bridge](../../research/runtime/single-player-menu-bridge.md)
and [spell-selection bridge](../../research/runtime/spell-selection-menu-bridge.md)
for the validated scope and remaining boundaries.

Run `./tools/test-live-menus.py` for the bounded isolated Xvfb/Wine round trip
and fallback check. See [the bridge contract and validation](../../research/runtime/menu-action-bridge.md).

## Native main menu preview

```bash
./tools/run-qt-shell.sh --main-menu
```

`MainMenuWidget` loads the installed 800x600 JPEG background, menu rectangles
and labels through the read-only AssetStore. It preserves the 4:3 canvas with
letterboxing and uses native focusable buttons (Tab/Shift-Tab and Space).
Typography uses a system serif font; original SFT fonts are not reproduced.
Missing/invalid assets print a diagnostic and exit with status 9.

Use `--menu-assets DIRECTORY` for another installation. CommandLine Battle is
hidden by default; `--menu-command-line` shows it for inspection. Version text
is supplied by a future controller and is blank in the preview.

The preview never launches the game. Buttons emit `actionRequested(Action)`;
the preview reports selections in its status bar, and Quit closes the preview.
Engine actions and live menu transitions still require the
[menu adapter work](../../research/runtime/main-menu-qt-migration.md).
The ordinary shell startup screen remains available with no preview option.
Run `ctest --test-dir working/build/qt-shell -R qt-main-menu --output-on-failure`
for synthetic asset loading, layout, presentation and action checks.

## Native Quick Battle menu

```bash
./tools/run-qt-shell.sh --quick-battle-menu
```

The main menu preview now opens Quick Battle when its Quick Battle button is
selected. Cancel or Escape returns to the main menu and restores focus. The
four Quick Battle buttons expose semantic actions; Create Multiplayer, Join
Multiplayer, and Create Single Player currently report their selection without
launching a game. Both entry points require assets for both menus.

The widget uses the installed background, heading, labels and button rectangles,
with native keyboard navigation and scaled letterboxing. Shared menu loaders
retain bounded read-only AssetStore access and transactional widget reloads.
Use `--menu-assets DIRECTORY` to select another installation.

See [scope and validation](../../research/runtime/quick-battle-qt-menu.md).

## Native Mini Menu previews

```bash
./tools/run-qt-shell.sh --mini-menu campaign
./tools/run-qt-shell.sh --mini-menu battle
```

Campaign shows Load Game, Save Game, Preferences, Quit Game and Cancel.
Battle shows Preferences, Quit Battle and Cancel. Cancel/Escape returns to the
main-menu preview; other buttons report semantic intent without game actions.
The inactive variant's controls are hidden and disabled, including during
keyboard navigation. These previews do not pause or resume the game.

The installed 600x400 BMP panel is centered in an 800x600 canvas with scaled
buttons and letterboxing. Original panel placement and typography still need
live comparison. Mini Menu assets load only when requested; main and Quick
Battle assets are also needed for preview navigation. See
[scope and validation](../../research/runtime/mini-menu-qt.md).

Native Qt application code, separate from reconstructed engine algorithms.
The default viewport presents captured engine frames through an OpenGL 3.3
texture shader. A native Wine-window embedding backend remains available.
The host can be x86-64 while the injected frame bridge and game are PE32 i386.

See [project architecture](../../docs/architecture.md) for the proposed separation
of QWidget presentation, session orchestration, and legacy adapters. The
[main-menu migration assessment](../../research/runtime/main-menu-qt-migration.md)
records the remaining integration work; those boundaries are not yet fully
extracted from the current shell.

## Build and open

Requires CMake, a C++17 compiler, Qt 6.8+ Widgets / OpenGLWidgets / OpenGL / Multimedia
libraries, pkg-config and XCB development files. The frame bridge additionally
requires Clang, llvm-dlltool, lld-link and Wine. The current host supplies these
and Qt 6.11.2. No game is launched merely by opening the application.

```bash
./tools/run-qt-shell.sh
```

For the original-renderer frame-copy path, launch with `--frame-readback`.
Click **Launch game** to build the render DLL, stage a hash-checked disposable
game copy, and run it through `tools/run-game.sh`. The game uses a separate
800x600 Wine desktop. Its primary DirectDraw surface is copied after successful
Blt/BltFast/Flip calls, converted to RGBA, and displayed in Qt via OpenGL with nearest
filtering and aspect-preserving letterboxing. The launch log and normal launcher
logs remain available. **Check installation** uses the existing launcher checks.

**This first migration changes final presentation, not engine drawing.** The
original DirectDraw renderer remains active. On X11/XWayland, click the OpenGL
image to forward keyboard/mouse input to the matching new game client. Shared
polling state supplies the bridge's Windows key/cursor API hooks. Focus loss
releases held inputs. The separate Wine window remains available; actual game
input/focus behavior still needs validation. Live map rendering remains unvalidated; synthetic producer-to-Qt
readback passes. The native game output is retained during that validation.

Every launch gets a new shared frame file in `working/runtime/render/` and a
fresh game copy in `working/experiments/opengl-render/`, with EXE / DLL hashes
and build provenance. Source game files and `original/` are not modified.
Exit the game and wait for the launcher to finish before closing the shell;
closure is blocked while the launcher runs. The default persistent prefix is
`working/wineprefix-x86_64`.

For the original embedding backend:

```bash
./tools/run-qt-shell.sh --renderer native
```

That backend requires Qt and Wine to share an X11/XWayland display. It snapshots
window IDs before launch and embeds exactly one new visible `MagicMayhem`
desktop. **Detach game** restores a separate native window, and **Attach game**
retries discovery. Click that viewport to focus game input. Resizing that
container does not replace the game's renderer or change its internal resolution.
The launcher script defaults `QT_QPA_PLATFORM=xcb`; explicitly select a different
Qt backend only for the OpenGL stream viewport. Native Wayland embedding is
unsupported.

## Verification

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --parallel 4
ctest --test-dir working/build/qt-shell --output-on-failure
./tools/test-render-bridge.py
./tools/test-render-input.py
```

CTest checks pixel conversion (palette, RGB565, RGB24, RGB32, negative pitch,
padding and invalid masks), frame-stream guards, headless shell startup, external
window embedding/detachment and known-pixel OpenGL framebuffer readback.
The GUI tests use isolated Xvfb displays; OpenGL readback uses Mesa software
rendering when requested by the test. Local X server sockets must be permitted.

The Wine integration test calls the PE32 surface hooks using a synthetic
DirectDraw interface, verifies original return values / last-error state and
lock/unlock pairing, checks mapped RGBA bytes, then feeds those exact bytes
through Qt/OpenGL framebuffer readback. It uses a dedicated test prefix and
never launches the game. It does not prove that the original game's surface
presentation or input works.

For opt-in drawing evidence, open `./tools/run-qt-shell.sh --capture-draws` and
click **Launch game** when ready. It records a bounded API inventory and one
small opaque/source-keyed copy for offline pixel comparison. The launch log
prints its capture directory. This is a drawing reconstruction aid; see
[capture and replay procedure](../../research/runtime/render-drawing-inventory.md).

For game-owned pixel capture without extra observer surface locks, use
`./tools/run-qt-shell.sh --capture-locks`. Diagnostic files stop at their count
and byte budgets; supported owned checkpoint updates and primary publication
continue within bounded memory. Restart the game to load bridge changes.
Continuous live presentation remains to be validated; see the
[continuity fixes and live protocol](../../research/runtime/render-startup-black-screen.md#2026-10-04-continuous-owned-checkpoint-fixes).

CLI: `--repo DIRECTORY`, `--renderer opengl|native`, `--capture-draws`, `--smoke-test` (no game),
`--opengl-test` (known pixels) and `--stream-test FILE` (synthetic readback).
Use `--renderer native --smoke-test` with the offscreen Qt backend.

## Boundaries and references

- `apps/qt-shell/`: UI, shared frame reader and OpenGL presentation.
- `runtime/render/`: PE32 DirectDraw frame capture.
- `renderer/`: native OpenGL drawing components and offline capture replay.
- `runtime/shadow/`: PE32 pathfinding instrumentation.
- `reconstruction/pathfinding/`: original engine models.
- `tools/`: builds, staging, launches and verification.

See [OpenGL migration scope and evidence](../../research/runtime/opengl-presentation.md)
and [shared frame format](../../research/formats/render-frame-stream.md).
Qt interfaces: [QOpenGLWidget](https://doc.qt.io/qt-6/qopenglwidget.html),
[foreign windows](https://doc.qt.io/qt-6/qwindow.html#fromWinId),
[QProcess](https://doc.qt.io/qt-6/qprocess.html).

The [native blit renderer](../../renderer/README.md) now executes captured
opaque/source-keyed copies through OpenGL and compares native pixels. Run
`./tools/replay-render-capture.py CAPTURE_DIRECTORY --backend opengl --headless`
for that offline check. The shell's live viewport still presents original
engine frames; routing live drawing through the new renderer is later work.

The shell now links the renderer's persistent-surface module. Its independent
demo opens with `./tools/run-qt-shell.sh --surface-demo`, performs native indexed
updates/copies, and cycles a palette entry without launching a game.
`--surface-test` verifies its displayed pixels through Qt framebuffer readback.
See [surface API and evidence](../../research/runtime/opengl-persistent-surfaces.md).

If Wine opens a black window, try
`./tools/run-qt-shell.sh --software-rendering --capture-history` in a new shell.
It selects Mesa software GL before Qt starts and passes that environment to Wine.
The viewport now reports hook/DirectDraw startup stages after ten seconds without
a frame, and reports creation/hook failures immediately. No registry changes are
made. See [startup evidence and limitations](../../research/runtime/render-startup-black-screen.md).

## Native movies and file sounds

Opt in with `./tools/run-qt-shell.sh --native-media`. Movies and supported
WinMM WAV file calls then run through Qt, with movie frames in the OpenGL
viewport. Escape skips movies; game input is suspended during playback.
DirectSound effects/voices remain in Wine. Default launches retain legacy
multimedia. Live startup and audible output still need validation.

Preview media without launching the game:

```bash
./tools/run-qt-shell.sh --media working/game-nocd/FMV/Intro0.avi
./tools/run-qt-shell.sh --media "working/game-nocd/Sounds/Spell click.wav"
python3 tools/test-native-media.py
```

Standalone playback supports Space (pause), Escape (stop), and +/- (volume).
The offline test compares exact synthetic movie pixels and PCM samples and
checks the x86 broker lifecycle. `--media-test --media-report FILE` silently
measures a preview; add `--media-probe` to stop after a movie frame and PCM
buffer. See [runtime scope/evidence](../../research/runtime/qt-native-media.md)
and [channel layout](../../research/formats/render-media-channel.md).

Battle result previews use the installed Victory/Defeat artwork and layouts,
with sample results and native OK/Enter/Escape navigation:

```bash
./tools/run-qt-shell.sh --battle-results victory
./tools/run-qt-shell.sh --battle-results defeat
```

Fonts and text/button colors remain native approximations; original background
artwork is reused, but visual equivalence has not been verified. See
[result scope, validation and fidelity](../../research/runtime/battle-results-qt.md).

Quick Battle result previews provide four sample players and mutually exclusive
Continue/Spectate states:

```bash
./tools/run-qt-shell.sh --quick-battle-results continue
./tools/run-qt-shell.sh --quick-battle-results spectate
```

Continue opens the native Quick Battle menu; Quit returns to Main. Spectate
emits intent with the engine adapter pending. Portraits use text placeholders.
See [layout, validation and remaining boundaries](../../research/runtime/quick-battle-results-qt.md).

Map Selection previews three sample entries with native list selection and
OK/Cancel navigation:

```bash
./tools/run-qt-shell.sh --map-selection
```

OK emits an opaque map ID; both actions return to the Quick Battle preview.
Installed map enumeration and engine selection remain pending. See
[Map Selection scope and validation](../../research/runtime/map-selection-qt.md).

Load Game provides a sample save list, editable filename and Load/Cancel actions:

```bash
./tools/run-qt-shell.sh --load-game
```

It also opens from the native Main menu. Exact supplied filename matches select
a save; Load emits its opaque ID with engine integration pending. Cancel returns
to Main. See [Load Game scope and validation](../../research/runtime/load-game-qt.md).

Save Game previews supplied filenames with Save/Delete/Cancel actions:

```bash
./tools/run-qt-shell.sh --save-game
```

The campaign Mini Menu Save button also opens this screen. New names emit Save
intent; existing-name overwrite and Delete require local confirmation. No files
are written or deleted. See [Save Game scope and validation](../../research/runtime/save-game-qt.md).

Preferences provides five radio groups and two sliders with local apply/cancel:

```bash
./tools/run-qt-shell.sh --preferences
```

It also opens from Main and either Mini Menu. OK accepts the local snapshot;
Cancel rolls back edits. Engine settings and file persistence remain pending.
See [Preferences scope and validation](../../research/runtime/preferences-qt.md).

Join and Create Multiplayer use installed layouts and sample names:

```bash
./tools/run-qt-shell.sh --join-multiplayer
./tools/run-qt-shell.sh --create-multiplayer
```

Both also open from Quick Battle. OK emits a typed request; Cancel returns to
Quick Battle. Networking stays pending. See [multiplayer setup scope and validation](../../research/runtime/multiplayer-setup-qt.md).

Preview Multiplayer Game Selection with supplied sample sessions:

```bash
./tools/run-qt-shell.sh --multiplayer-game-selection
```

Join OK also opens this screen; Cancel returns to Join with its draft retained.
Session OK emits intent only. Discovery and joining remain pending. See
[scope and validation](../../research/runtime/multiplayer-game-selection-qt.md).

Preview Single Player Battle Setup with sample players and configured sliders:

```bash
./tools/run-qt-shell.sh --single-player-battle
```

It also opens from Quick Battle. Map Selection returns to this setup with edits
retained; portrait/colour controls cycle local samples. Start emits a setup
request without launching a game. See [scope and validation](../../research/runtime/single-player-battle-qt.md).

Preview the multiplayer host or guest lobby:

```bash
./tools/run-qt-shell.sh --multiplayer-lobby host
./tools/run-qt-shell.sh --multiplayer-lobby join
```

Create OK opens the host lobby; Join → session selection opens the guest lobby.
Chat echoes locally. Host settings/Map and guest Ready use local sample state;
networking and battle launch remain pending. See [scope and validation](../../research/runtime/multiplayer-lobby-qt.md).

Preview Region Entry with a sample Celtic region and installed illustration:

```bash
./tools/run-qt-shell.sh --region-entry
```

Four difficulty choices emit typed entry intent; G/S/C stand for Grimoire,
Spellbox and Character. Cancel returns to Main Menu. Campaign flow and those
engine screens remain pending. See [scope and validation](../../research/runtime/region-entry-qt.md).

Preview Character Improvement with sample stats and supplied upgrade costs:

```bash
./tools/run-qt-shell.sh --character-screen
```

Region Entry's Character button also opens it. Plus purchases an increment;
minus undoes a draft purchase. OK accepts locally; Cancel restores the snapshot.
Engine progression remains pending. See [scope and validation](../../research/runtime/character-screen-qt.md).

Menu backgrounds now use the native BMP/JPEG loader APIs. Character Screen also
uses its installed stat-bar BMPs and caller-selected WizardFace JPEGs (the sample
chooses face 0). Other sprite controls and original fonts remain pending. See
[image integration and fidelity boundaries](../../research/runtime/menu-image-integration.md).

Character Screen now renders original SPR talismans and adjustment controls;
Region Entry renders its original face/book/portmanteau icons. Single Player and
host/guest lobbies render supplied SPR portraits/colour tokens and boot controls.
Fonts, sliders and engine actions remain pending. See [SPR integration](../../research/runtime/menu-sprite-integration.md).

Preview the installed Grimoire with chapter contents, entries and original art:

```bash
./tools/run-qt-shell.sh --grimoire
```

Region Entry's Grimoire icon opens it too. Double-click or Enter opens an entry;
Previous/Next and Page Up/Down browse pages. Artwork/Read toggles companion art.
Close/Escape returns to the caller. Campaign knowledge and dynamic values remain
pending. See [scope and validation](../../research/runtime/grimoire-qt.md).

Native audio catalog/manager preview (no game):

```sh
./tools/run-qt-shell.sh --audio-catalog working/game-nocd/Sounds \
  --audio-path-policy dequote-missing-leaf --audio-sound 812
```

The window lists ready/unavailable entries and provides Play, Stop Session and
Restart Session. Omit the policy option for literal filenames. Add
`--audio-preflight --audio-report working/audio-catalog.json` for a read-only
catalog check without an audio device; exit 3 means a valid catalog with missing
or unsupported entries. `--audio-map ID` selects classifications/permanent
preload. [Session scope and offline checks](../../research/runtime/qt-audio-manager-session.md)
keep this separate from live DirectSound replacement.

Preview Portmanteau with sample inventory and supplied spell mappings:

```bash
./tools/run-qt-shell.sh --spellbox
```

Region Entry's Spellbox icon opens it too. Select an ingredient to see all three possible spells in the header. Click to
carry it with the pointer, or drag it onto a talisman; hover previews the supplied
spell artwork and placement commits it. Pick up a filled talisman and return it
to the right-hand shelf to remove the spell. Right-click/Escape deselects a carried
item. Assign/Remove remain available; Preview emits local spell intent. OK
accepts locally and Cancel restores the accepted loadout. Original BMP/SPR art
is used; sample recipes and native controls remain approximations. See
[scope and validation](../../research/runtime/spellbox-qt.md).

Opt in to native menu click/page-turn cues through the reconstructed audio manager:

```sh
./tools/run-qt-shell.sh --main-menu --menu-audio
./tools/run-qt-shell.sh --grimoire --menu-audio
```

One session follows navigation and accepted Preferences sound volume; closing the
preview stops it. Defaults 822/830 are native preview choices. Select other
catalog IDs with `--menu-click-sound ID` / `--menu-page-sound ID`; choose filename
compatibility with `--menu-audio-policy dequote-missing-leaf`. Music routing and
original cue mappings remain pending. [Scope and offline tests](../../research/runtime/menu-audio-integration.md).

Menu audio now reports output failures in a persistent status-bar label with
**Retry audio**. It retries when Qt reports an available device, follows the
current default output, and restores accepted effects gain without replaying
old cues. Closing the preview cancels pending recovery. See
[recovery scope and evidence](../../research/runtime/audio-output-recovery.md).

## Persistent menu effects volume

`--menu-audio` restores accepted effects volume before its first cue and saves it
when Preferences is accepted. Cancel leaves the saved value unchanged. The
user-scoped QSettings INI uses `MagicAndMayhemMod/QtShell` and the versioned key
`audio/v1/effectsLevel`; other preferences remain local. See
[implementation and offline restart evidence](../../research/runtime/menu-audio-preferences.md).

Preview the campaign maps with installed region names and supplied availability:

```bash
./tools/run-qt-shell.sh --realm-viewer
```

Main Menu's New Game opens this preview too. Select a flag and OK, double-click,
or press Enter to open Region Entry; Cancel returns to the selected map flag.
Difficulty is remembered per region. Direct Portmanteau/Grimoire/Character and
Options previews return to the map. All sample regions are available; campaign
progression and engine commands remain pending. See
[scope and validation](../../research/runtime/realm-viewer-qt.md).

## Native menu music

`--menu-music /absolute/path/to/track.wav` opts into a looping local track in any
native menu preview, with or without `--menu-audio`. Music continues across menu
navigation, stops on closure, and has its own status and saved accepted volume.
Cancel keeps the accepted value. No default track is selected. See
[controller, policy and offline evidence](../../research/runtime/native-menu-music.md).

## Shared original menu fonts

All migrated menus use the installed SFT heading, body, tooltip and quantity
font shapes through a shared native loader/Qt font bridge. Buttons, labels,
lists, editable fields, local chat and Grimoire prose use common font roles and
canvas scaling, without synthetic bold. Sprite lettering retains its original
image. All four installed font inputs must be valid; an entirely absent catalog
uses an explicit common fallback for synthetic fixtures.

```bash
cmake --build working/build/qt-shell --target menu-font-test --parallel 4
QT_QPA_PLATFORM=offscreen working/build/qt-shell/menu-font-test working/game-nocd
```

This integrates recovered transparency masks and origins into Qt's text system.
Original palette shading, contour kerning, code pages and pixel-perfect text
flow still require comparison. See
[font integration evidence and policy](../../research/runtime/menu-font-integration.md).

Menu music also monitors output loss/default changes and shows **Retry music**.
An availability event or explicit retry reopens the selected track from the
beginning with its latest accepted volume; closure cancels recovery. See
[music recovery and synthetic validation](../../research/runtime/menu-music-recovery.md).

## Realm Viewer region shapes and animation

The Realm Viewer now highlights regions with their original PCX border art,
selects map regions through their silhouette pixels and animates the green flags.
Selecting a region does not display an automatically walking character; campaign
position/travel needs a caller-supplied contract. Keyboard-accessible flag
shortcuts and availability guards remain active; hidden screens stop animation.
Flag playback timing is a native presentation policy.

```bash
QT_QPA_PLATFORM=offscreen working/build/qt-shell/realm-viewer-visuals-test working/game-nocd
```

See [visual asset integration and verification](../../research/runtime/realm-viewer-visuals.md).

## Native Spell Research preview

```bash
./tools/run-qt-shell.sh --spell-research
```

Spell Research is also available from the Realm Viewer's original Research
button. The native view uses Grimoire book art and 42 installed spell descriptions,
shared SFT fonts, filtering and caller-supplied availability/learned states.
Research emits an owner/spell request; campaign costs and progression are pending.
Close returns to the map with focus restored. No separate original research
layout was found; see [scope and evidence](../../research/runtime/spell-research-qt.md).

## Dormant Mini Menu bridge

The V4 Mini Menu bridge remains behind `MNM_EXPERIMENTAL_MINI_MENUS`, which defaults to **OFF**. Isolated callback/wire checks passed, but live Quick Battle Escape reaches Game Over instead of Mini. Campaign ingress, confirmation and pause remain unvalidated. See the [recovery and boundaries](../../research/runtime/mini-menu-engine-bridge.md).

Normal `--live-menus` sessions use V6 Main Preferences alongside V5 Quick Battle results. The Qt Game Over screen reads the engine's displayed player/stat rows; Continue returns to original gameplay and Quit follows the engine back to Quick Battle. Window close on a ready results screen follows original Quit through Quick and Main to shutdown. Spectate/multiplayer and campaign results remain in the original viewport, and portrait sprites are still placeholders. See [results integration and evidence](../../research/runtime/quick-battle-results-engine-bridge.md).

### Live Main Preferences

Main **Preferences** reads the engine's current settings and available controls.
Audio sliders preview through the original services; radio edits apply on OK.
Cancel restores entry audio through the engine. The original engine handles the
settings writer and display rebuilding. Accepted settings are validated against the original-written file and saved for
future launches in the user-scoped engine Preferences store. Mini and
campaign callers remain outside this adapter. Validate without manual interaction
with `python3 tools/test-live-menus.py --preferences`; see the
[Preferences bridge and evidence](../../research/runtime/preferences-engine-bridge.md).

The [cross-launch persistence policy](../../research/runtime/preferences-persistence.md)
imports only the seven exposed settings into each fresh session. Cancel, previews
and startup do not save. Failed persistence keeps the current game session usable
and reports the reason in the launch log. Run `python3 tools/test-live-menus.py
--preferences-restart` for automated save/relaunch/restore/Cancel validation.

`python3 tools/test-live-menus.py --preferences-display` automatically cancels a
resolution draft, applies four alternating High/Low changes and checks original
game client sizes, leave flags, font modes and reopened controls before Quit.
The forwarding leave observer is enabled only for this test. See
[UI25 scope/evidence](../../research/runtime/preferences-display-rebuild.md).

## Direct GPU command replay

`--commands FILE` now displays each bounded replay PRESENT directly from the
renderer-owned shared texture. It avoids PRESENT pixel readback and viewport
re-upload; explicit pixel/color CHECK commands remain diagnostic readbacks.
The last texture survives stream END and source destruction. Image exports
retain the existing readback path. This is offline replay, separate from live
game capture or draw replacement.

Run `python3 tools/test-gpu-presentation.py working/build/qt-shell` for independent
complete-frame, lifetime, normal/high-DPI and shell-entry checks. See
[ownership, synchronization and evidence](../../research/runtime/opengl-direct-presentation.md).

## Incremental command replay

`--commands FILE` retains one consumer across 32-command Qt timer batches.
Ordinary replay counts CHECK/CHECK_RGBA as skipped and performs no diagnostic
readbacks; add `--command-checks` for explicit comparison. The complete bounded
file is still validated before playback. This is an offline service, separate
from a live engine channel. [Lifecycle and validation](../../research/runtime/opengl-incremental-commands.md).

Ordinary OpenGL shell launches default to continuous live command/GPU presentation from the
owned PE32 hooks and implies `--capture-locks`. It uses a fresh mapped command
channel per launch, processes at most 32 commands per poll and refuses incomplete
sessions. After terminal native refusal, the shell reconnects the original Wine
window and switches to direct game input. Forwarded input and native polling stop;
missing or ambiguous original windows retain the Attach control. This fallback
does not repair incomplete native drawing history. See
[input fallback evidence](../../research/runtime/native-command-input-fallback.md).
See [scope and validation](../../research/runtime/opengl-live-command-transport.md).

`--native-commands` remains available to select this path explicitly.
`--frame-readback` selects original-renderer frame copies from game-owned
Lock/Unlock buffers and held DCs. It also implies `--capture-locks`, so it takes
no extra DirectDraw surface locks. It cannot be combined with `--native-commands`.
Both ordinary presentation modes avoid observer locks; see
[surface contention diagnosis](../../research/runtime/render-surface-contention.md).
Explicit `--capture-draws`,
`--capture-history`, `--capture-locks` and `--no-readback` diagnostics keep their
existing path unless `--native-commands` is also supplied. `--renderer native`
and `--live-menus` continue to use Wine-window embedding. Driver selection is
independent: `--software-rendering` still explicitly requests Mesa software rendering.

Native command presentation queues the latest completed GPU frame instead of
repainting synchronously for every captured PRESENT. Known command backlog drains
through bounded Qt continuations, allowing input events between batches instead
of adding a16 ms frame-timer wait to each batch. This addresses shell queueing;
producer refusals and game/driver latency remain separate issues. See
[responsiveness policy and tests](../../research/runtime/native-command-responsiveness.md).


`./tools/run-qt-shell.sh --native-commands` selects a v2
channel and the continuous owned producer by default. Its command archive defaults off;
`MNM_RENDER_SESSION_ARCHIVE=1` retains a bounded diagnostic prefix independently
of live progress. `MNM_RENDER_CONTINUOUS=0` explicitly selects the bounded
diagnostic preview, which can stop on a black startup frame. Direct experiment
launches retain their bounded sample policy unless opted into continuous mode. This
mode retains original drawing and refuses unsupported ownership/operations;
resource release and guarded orderly exit are implemented. See the
[producer contract](../../research/runtime/opengl-continuous-producer.md) and
[startup/exit orchestration](../../research/runtime/opengl-command-orchestration.md).

For an isolated native-command launch with the existing opt-in drawing-order
policy, software rendering and movie playback disabled:

```sh
MNM_RENDER_ORDERED_COPIES=1 ./tools/run-qt-shell.sh \
  --native-commands --software-rendering --skip-movies --scaling smooth
```

Drawing ordering coordinates concurrent copies and surface handoffs. It changes
drawing-call scheduling and remains opt-in; simulation rules and logical game
resolution are unchanged. A GAP refusal means the producer lost complete surface
history. Software rendering alone does not repair that history. See
[failure diagnosis and validation boundaries](../../research/runtime/native-command-refusal.md).

Qt continuous mode now negotiates recovery through a fresh versioned control
channel. It cancels the failed consumer, clears presentation, creates a fresh
higher-session command file, and resumes input only after producer admission
and a complete new frame. Replies time out after 6 seconds, frames after
10 seconds, and at most three recoveries are allowed per launch. Refusal or
exhaustion preserves original-window fallback. Synthetic validation uses the
same Qt viewport/context; actual-game recovery remains pending. See
[host recovery](../../research/runtime/opengl-command-host-recovery.md) and the
[producer recovery contract](../../research/runtime/opengl-command-recovery.md).

Continuous native command window close requests terminal producer stop while Qt
keeps draining cleanup/END. Stop and END acknowledgment can arrive in either
order; recovery stays disabled and input stays suspended. The original game's
Quit action still owns game process exit. The PE32 normal ExitProcess wrapper
joins both producer workers and releases rendering storage after borrowed owners
return; refusal retains storage for retry or OS process teardown. Dynamic DLL
unloading remains unsupported; installed hook code is pinned. See
[terminal ownership and synthetic validation](../../research/runtime/opengl-application-shutdown.md).
