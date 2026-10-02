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

Display modes have short, memorable flags:

```bash
./run-asahi.sh -w                    # 1280x960 window
./run-asahi.sh -s                    # scale to the detected display size
./run-asahi.sh -f                    # native game fullscreen (default)
./run-asahi.sh --window-size 1600x1200
```

Windowed and scaled modes use Wine's virtual desktop, leaving the game's fixed
640x480/800x600 rendering and tracked configuration files untouched. Set
`MNM_DISPLAY_SIZE=WIDTHxHEIGHT` if automatic display detection is unavailable.

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

## Current result

- **Confirmed:** both working installations pass structural and hash checks.
- **Confirmed:** the clean and no-CD manifests differ only in `Chaos.exe` and
  `CDROMDrive.cfg`.
- **Blocked:** no Wine or Proton runtime is currently installed, so a process
  smoke test and visual confirmation of reaching gameplay have not yet run.
- **Confidence:** high for file/layout validation; runtime behavior remains
  unconfirmed.
