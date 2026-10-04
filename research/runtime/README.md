# Runtime research

Document runtime structures, functions, addresses, signatures, and hooks here.
For each finding, record the executable hash, discovery method, evidence,
confidence, and whether it remains stable across launches.

- [Windows file API audit](windows-file-api-audit.md): pinned clean/No-CD/JPEG
  imports and references, save temporary-file evidence, and native asset gaps.
- [MMSprite versus original sprite routines](sprite-binary-comparison.md):
  indexed and direct 16-bit colour, empty frames, version/palette contracts,
  and isolated original drawing/conversion comparisons.

- [Engine modernization coverage ledger](coverage-ledger.md): subsystem scope,
  evidence, live replacement status and rules for measuring progress.
- [Threading evidence](threading.md): inspected timer/thread interfaces and
  limits of current simulation ownership findings.

- [Executable hook candidates](hook-candidates.md): build-specific route,
  configuration lifecycle, and DirectDraw interception candidates; no hooks
  installed yet.
- [Route reconstruction and decompiler workflow](route-decompilation.md):
  partial manual pseudocode, search-state observations and automated export setup.
- [Tracked raw decompilation](decompiled/nocd/README.md): seven unedited
  Ghidra exports preserved separately from future readable reconstructions.
- [Readable route-request milestone](../../reconstruction/pathfinding/README.md):
  reproducible annotations, packed layout prefixes and tested host-side models.
- [Qt/OpenGL presentation](opengl-presentation.md): PE32 frame capture and native viewport.
- [Drawing inventory and replay](render-drawing-inventory.md): build-specific
  surface wrappers and bounded opaque/source-keyed copy evidence.
- [OpenGL blit replay](opengl-blit-replay.md): native integer shader copies,
  three-way pixel comparison and synthetic x86 capture validation.
- [Persistent OpenGL surfaces](opengl-persistent-surfaces.md): retained native
  textures, ordered updates/copies, palette cycling and Qt presentation.

- [Game-owned RGB blit propagation](opengl-game-owned-blits.md): retained Lock/Unlock
  checkpoints, bounded Blt/BltFast replay without observer surface locks, and
  independent synthetic x86/CPU/OpenGL checks.

- [Game-owned primary presentation](opengl-game-owned-primary.md): known primary
  RGB blits reach the shared frame stream and Qt/OpenGL viewport, validated
  offline with independent engine pixels and per-operation frame counters.

- [Primary bootstrap from application descriptors](opengl-primary-bootstrap.md):
  never-Locked RGB initialization by complete opaque copies, with 800x600 offline
  x86/CPU/OpenGL/Qt validation.

- [Owned RGB Flip routing](opengl-owned-flips.md): observed two-buffer chains,
  native pixel rotation and Qt publication without observer COM calls.

- [Owned indexed primary presentation](opengl-owned-indexed.md): raw Lock/Unlock
  indices, observed 256-color palettes and recoloring without observer calls.

- [Owned indexed copies and Flips](opengl-owned-indexed-copies.md): raw index/key
  propagation, destination/front palettes and native OpenGL replay evidence.

- [Partial CPU Lock reconstruction](opengl-partial-locks.md): detached complete
  checkpoints, rectangle merges, successful Unlock commits and offline x86/Qt tests.

- [Qt movies and file sounds](qt-native-media.md): optional x86 movie/WinMM hooks,
  native decoding and bounded broker lifecycle; DirectSound remains in Wine.

- [DirectSound setup and sample ownership](directsound-buffer-setup.md): pinned
  primary/secondary creation, native PCM uploads and duplicate buffer lifecycle;
  offline reconstruction without game interception or audible output.

- [DirectSound voice controls](directsound-voice-controls.md): observed playback,
  volume/pan, status/reset and scheduler contracts, with offline tests and a
  bounded frequency/cursor call audit.

- [Native audio voice state](native-audio-voice-state.md): independent secondary
  cursors, controls, loops/completion and ownership, tested without audio output.

- [Native stereo PCM mixer](native-audio-mixer.md): fixed output clock, interpolation,
  volume/pan and clipping, tested offline without an audio device.

- [Native Qt audio output](native-audio-output.md): QAudioSink format negotiation,
  bounded push writes, lifecycle fixtures and host-backend tone/restart evidence.

- [Native DirectSound voice bridge](native-audio-voice-bridge.md): optional x86 COM
  routing into the mixer/Qt output, with ABI fixtures and guarded staging evidence.


- [Primary audio manager](primary-audio-manager.md): saved volume, selected startup/disable/shutdown ordering, native master attenuation and x86 fixture evidence.

- [Native SPR rendering](native-sprite-rendering.md): owned mask/RGB565 uploads,
  origin placement, offline previews and selected original draw comparisons.

- [Audio voice lifetimes](audio-voice-lifetimes.md): retirement versus destruction, duplicate cleanup and circular reusable scheduler records; offline contracts and fixtures.
