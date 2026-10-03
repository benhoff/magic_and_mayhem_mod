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
