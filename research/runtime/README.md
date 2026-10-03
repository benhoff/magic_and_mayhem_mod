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
