# Runtime research

Document runtime structures, functions, addresses, signatures, and hooks here.
For each finding, record the executable hash, discovery method, evidence,
confidence, and whether it remains stable across launches.

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
