# Magic & Mayhem Qt shell

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

## Build and open

Requires CMake, a C++17 compiler, Qt 6.8+ Widgets / OpenGLWidgets / OpenGL / Multimedia
libraries, pkg-config and XCB development files. The frame bridge additionally
requires Clang, llvm-dlltool, lld-link and Wine. The current host supplies these
and Qt 6.11.2. No game is launched merely by opening the application.

```bash
./tools/run-qt-shell.sh
```

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

Region Entry's Spellbox icon opens it too. Drag an item onto a talisman, or select
both and Assign. Remove returns a copy; Preview emits local spell intent. OK
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
