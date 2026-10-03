# Launch baseline

## Host and executable

- Host observed: Fedora Asahi Remix 44, `aarch64`, 16 KiB-page Asahi kernel.
- Game executable: PE32 Intel i386 Windows GUI application.
- No-CD executable SHA-256:
  `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
- Retail executable SHA-256:
  `124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`.

## Launcher

`tools/run-game.sh` validates the selected executable, DLL, CD configuration,
required asset directories, display access, and runtime before launch. It writes
timestamped, non-secret environment and process logs under `working/logs/`.
It launches a writable copy under `working/runtime/`; the manifested installation
trees remain rebuildable baselines because the game decrypts CFG files in place
and updates AI and preference state while running.

The no-argument default launches `working/game-nocd/`. Other useful invocations:

```bash
./tools/run-game.sh check
./tools/run-game.sh smoke --seconds 20
./tools/run-game.sh --clean
```

The launcher now wraps Wine (or a custom runner) with gamescope by default:
an 800x600 virtual display, fullscreen output, aspect-preserving fit scaling,
and nearest-neighbor filtering. Windowed output defaults to 1280x960. The
virtual display is independent of the game's own resolution setting; no game
configuration or executable bytes are changed.

```bash
./run-x86.sh                         # gamescope fullscreen (default)
./run-x86.sh -w                      # 1280x960 gamescope window
./run-x86.sh -w --size 1600x1200      # larger output window
./run-x86.sh -f --game-size 640x480   # alternative virtual display
./run-x86.sh -s                      # output matches detected display size
./run-x86.sh --window-size 1600x1200 # same as -w --size 1600x1200
./run-x86.sh --gamescope-arg -r --gamescope-arg 60
./run-x86.sh --no-gamescope -w        # original Wine desktop window
```

The same flags work through `tools/run-game.sh` and `run-asahi.sh`. `--size`
changes output dimensions without changing the gamescope display mode;
`--game-size` changes the virtual display. Extra gamescope arguments are appended
after defaults, one argument per `--gamescope-arg`; arguments after `--` are
passed to the game. For example, use `--gamescope-arg -F --gamescope-arg linear`
for smooth filtering. `MNM_GAMESCOPE_BIN` selects an alternative gamescope
executable. Missing gamescope is a preflight error with a `--no-gamescope` escape.
Set `MNM_DISPLAY_SIZE=WIDTHxHEIGHT` if automatic display detection for `-s` is
unavailable. Super+F toggles fullscreen while gamescope is running.
See the [upstream gamescope documentation](https://github.com/ValveSoftware/gamescope)
for compositor options and shortcuts.

With `--no-gamescope`, windowed and scaled modes retain Wine's virtual desktop
behavior. The existing `--runner` restriction on Wine desktop flags applies only
in this direct mode. Gamescope wraps the whole runtime command, including
`muvm` when needed; visual behavior on Asahi/muvm still needs confirmation.

For this Fedora Asahi host, the single entry point is:

```bash
./run-asahi.sh
```

It prompts before installing packages or enabling the experimental third-party
ARM64EC Wine repository, then prepares, validates, and launches the game. Use
`./run-asahi.sh check` for preflight only or `./run-asahi.sh smoke` for the
bounded smoke test.

On an x86 host the launcher discovers `wine` from `PATH`. On this aarch64 Asahi
host, it can use native ARM64EC Wine directly. It can also accept an x86 Wine
distribution that FEX can execute:

```bash
MNM_WINE_BIN=/absolute/path/to/x86-wine/bin/wine ./tools/run-game.sh check
MNM_WINE_BIN=/absolute/path/to/x86-wine/bin/wine ./tools/run-game.sh smoke
```

The launcher invokes that binary through `muvm`, which supplies a 4 KiB-page
microVM for FEX. A custom Proton or Wine wrapper can instead be supplied with
`MNM_RUNNER` or `--runner`; it must accept the executable path as its first
argument.

## Earlier Asahi preflight result

- **Confirmed:** both working installations pass structural and hash checks.
- **Confirmed:** the clean and no-CD manifests differ only in `Chaos.exe` and
  `CDROMDrive.cfg`.
- **Blocked:** no Wine or Proton runtime is currently installed, so a process
  smoke test and visual confirmation of reaching gameplay have not yet run.
- **Confidence:** high for file/layout validation; runtime behavior remains
  unconfirmed.

## x86 host validation — 2026-10-02

- **Confirmed, high confidence:** the current host is Arch Linux `x86_64`.
  System Wine `11.16-1` provides an x86-64 ELF loader and both i386 and
  x86-64 Windows runtime DLLs. Both game executables and `jpeg.dll` are
  PE32 Intel i386 binaries; no game recompilation is required.
- The existing `working/wineprefix/` contains ARM64 `system32/ntdll.dll`.
  A fresh `working/wineprefix-x86_64/` was initialized for this host;
  the previous prefix was retained. Use `./run-x86.sh --window` to reproduce
  the successful launch, or `./run-x86.sh check` for preflight.
  Arch's Wine package supports 32-bit applications through
  [WoW64](https://archlinux.org/news/transition-to-the-new-wow64-wine-and-wine-staging/);
  do not force `WINEARCH=win32` for this package.
- **Confirmed:** the complete `tests/test-tooling.sh` suite passed, including
  the C++17 reconstruction with ASan/UBSan and synthetic route replay tests.
  These validate host tooling, not live engine equivalence.
- **Confirmed:** the 45-second Wine smoke run completed successfully at
  `working/logs/run-20261002T143743Z.hzqSRi/`. Since first-launch prefix setup
  and Wine's desktop wrapper can stay alive without a functioning game,
  this result alone was not used as proof of game startup.
- **Confirmed, high confidence:** the subsequent launch kept `Chaos.exe`
  running and visually rendered the Magic & Mayhem v1.00 main menu.
  Evidence: `working/logs/run-20261002T143843Z.pwseBe/environment.txt`,
  `stderr.log`, and `window.png` (game window only).
- **Limitations:** Wine logged Mesa/EGL, pixel-format, and Quartz video
  playback errors despite the visible menu. In-map gameplay, audio, intro
  movie playback, and live debugger capture were not validated by this run.
- Full installation verification passed for `source-disc` and `game-clean`,
  but detected existing drift in `game-nocd`: `AI/Brain.dat`, eleven encrypted
  CFG files, and `CFG/prefs.cfg` differ from its manifest. Sampled Brain/prefs
  modification timestamps are from October 1, before this session; source and
  runtime copies have distinct inodes. These existing files were preserved.
  The executable, JPEG DLL, and CD configuration still pass launcher hash
  validation. Do not treat the entire no-CD source tree as a pristine baseline.
- The workspace Ghidra installation also contains an ELF x86-64 decompiler
  under `Ghidra/Features/Decompiler/os/linux_x86_64/decompile`, despite its
  installation directory having an `arm64` suffix. Ghidra execution was not
  part of this launch check.

## Gamescope launcher check — 2026-10-02

- **Confirmed, high confidence:** command-composition tests cover the default,
  output and virtual dimensions, windowed/fullscreen flags, additional compositor
  arguments, game arguments with spaces, custom runners, missing-compositor
  diagnostics, and direct Wine fallback. Evidence: `tests/test-launcher.py`.
  The complete `tests/test-tooling.sh` suite also passed.
- **Confirmed:** installed gamescope `3.16.25` accepted the generated command.
  `./run-x86.sh smoke -w --seconds 10` remained alive for the bounded interval.
  Evidence: `working/logs/run-20261002T145023Z.dB6p4U/environment.txt` and
  `stderr.log`. Timeout required its kill-after fallback to stop the process.
- **Limitations:** this checks process startup, not visual rendering, input,
  audio, or in-map gameplay. Wine still reports pixel-format and Quartz playback
  errors. Asahi/muvm composition has not been exercised.
