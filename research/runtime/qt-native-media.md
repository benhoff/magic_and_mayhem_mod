# Native Qt movies and file sounds

This opt-in chunk replaces the movie wrapper and supported `PlaySoundA` file
calls with Qt Multimedia. Movies reach the existing OpenGL viewport through
`QVideoSink`; their soundtrack and the separate WAV channel use `QAudioOutput`.
DirectSound effects, voice buffers, Windows window/message APIs and remaining
DirectDraw calls continue through Wine. This is not a complete audio port.

## Static findings

Pinned working no-CD `Chaos.exe` SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses below apply only to that executable at base `0x00400000`.

| Address | Finding | Evidence / confidence |
|---|---|---|
| `0x00469a00` | Movie wrapper; path in ECX, DirectDraw surface in EDX, four stack arguments: HWND, x, y, signed control byte | `llvm-objdump` entry, argument accesses and `ret 0x10`; high static confidence, synthetic fastcall test |
| `0x004696e0` | Creates the multimedia stream, converts/open path, configures video/audio | Static COM call sequence; high for role, not all error branches reconstructed |
| `0x00469830` | Plays stream samples, blits to DirectDraw, polls Escape | Static call/branch inspection; high |
| `0x004e992b`, `0x004e996c` | Startup calls for Intro0 and Intro1 | Static callers; high |
| `0x005fbf50`, `0x005fbf54` | Original movie destination x/y offsets | Wrapper stores; high |
| `0x005fbf58` | Original skip flag; wrapper returns its inverse as a boolean | Playback Escape branch and wrapper return; high |
| `0x005fbf5c` | Sign-extended movie control byte | Wrapper store and playback branch; high |
| `0x005c52d8` | `WINMM.PlaySoundA` IAT slot | PE import table; high |

The native movie hook handles control byte zero only. Other control values
retain the original wrapper. Qt uses aspect-preserving presentation in its
viewport; the original x/y placement is not applied to that separate viewport.
The volume of actual game calls through PlaySound versus DirectSound is still
unknown: imports establish available paths, not their runtime frequency.

## Installation and behavior

Run `./tools/run-qt-shell.sh --native-media` and click Launch game when able to
test. Opening the shell alone does not start the game. Default launches retain
legacy multimedia. The launcher validates the pre-created media file, stages
the hash-checked working installation and sets `MNM_RENDER_MEDIA` only for
that child. No original artifact or source game executable is modified.

The optional DLL hook verifies the original `PlaySoundA` import pointer and the
movie prologue `51 8d 44 24 00`. It installs a five-byte jump and a ten-byte
fallback trampoline containing those whole, relocatable instructions. The
trampoline becomes executable before installation and instruction caches are
flushed. The hook is installed at initial DLL load, not by debugger attachment.

The [shared media channel](../formats/render-media-channel.md) returns movie
completion as 1, Escape/stop as 0. Decoder/path failures before acceptance use
the original wrapper; failures after acceptance abort without duplicate movie
playback. Lost hosts produce bounded cancellation/fallback. The game thread
waits synchronously while the separate Qt process runs playback.

Supported file sounds require `SND_FILENAME`, no module, and only `SND_ASYNC`,
`SND_NODEFAULT`, `SND_LOOP`, `SND_NOSTOP`. Loop requires async. The Qt channel
replaces its previous sound, rejects busy `NOSTOP`, and honors a null-path stop.
Unsupported calls retain the original WinMM function. Switching to legacy
signals cancellation of a Qt sound; successful native calls retire legacy async
sounds. DirectSound has its own channels and is not intercepted. Original API
arguments, return/error behavior on fallback, and LastError on native success
are preserved by the tested wrappers.

While a movie plays, game-frame polling pauses and forwarded keys/buttons are
released. Qt consumes movie input; Escape skips. Game input resumes afterward.
This prevents movie dimensions or Escape from controlling an unseen game frame.

Standalone preview, without the game:

```bash
./tools/run-qt-shell.sh --media working/game-nocd/FMV/Intro0.avi
./tools/run-qt-shell.sh --media "working/game-nocd/Sounds/Spell click.wav"
```

Space pauses/resumes, Escape stops, +/- changes volume. Qt 6.8+ Multimedia is
required because `QAudioBufferOutput` measures decoded audio independently of
audible device output. RGBX/BGRX movie frames explicitly ignore the unused X
byte; the local Qt FFmpeg conversion otherwise exposed it as alpha in the
synthetic FFV1 fixture. Other frame formats use Qt's conversion.

## Verification and remaining work

`python3 tools/test-native-media.py` builds known stereo 22050 Hz PCM and a
lossless AVI with ten colored frames. It compares exact decoded PCM and packed
RGBA SHA-256 against independently generated source bytes, then drives the
actual native wrappers through synthetic x86 Wine calls. It tests completion,
skip, async/loop/stop/busy behavior, native-to-legacy sound replacement, missing
files, traversal rejection, unsupported memory sound fallback, stale host,
LastError, stack cleanup, trampoline execution and prologue rejection.

Successful synthetic evidence: `working/tests/native-media/run-7dt8szqc/`.
The installed Intro0 probe at
`working/tests/native-media/run-opn8oqfo/indeo-probe.json` confirms Qt can decode
its Indeo 4/YUV410P video at 640×360 and PCM s16le stereo soundtrack. It stopped
after two frames and 3072 audio frames. Original-manifest verification passed
before/after that asset experiment. Sixteen existing Qt/render checks passed;
the movie suspension extension passed both normal and high-DPI input tests.
The DirectDraw-to-Qt regression passed at `working/tests/render/run-qj4_bcmi/`;
media-channel staging passed without launching the game at
`working/experiments/opengl-render/run-z8sfwnpg/`, with original-manifest
verification before and after. The production bridge is PE32 i386.

Confidence is high for offline decode and synthetic ABI/lifecycle behavior.
Audible output, full movie playback, live startup and return to a game/map have
not been validated. Next audio reconstruction is DirectSound device/buffer
creation, sample ownership, play/stop/loop cursors, volume/pan/frequency and
voice retirement, followed by an independently testable native mixer. CD music
is already disabled in disposable OpenGL staging; this chunk does not replace
the original CD playback path.
