# Runtime research

Document runtime structures, functions, addresses, signatures, and hooks here.
For each finding, record the executable hash, discovery method, evidence,
confidence, and whether it remains stable across launches.

- [Windows file API audit](windows-file-api-audit.md): pinned clean/No-CD/JPEG
  imports and references, save temporary-file evidence, and native asset gaps.
- [Save/load and campaign progression](persistence-progression.md): named save
  dispatch, packed envelope and serializer owners, realm initialization,
  battle-return ownership changes and next-realm flow; static research only.
- [MMSprite versus original sprite routines](sprite-binary-comparison.md):
  indexed and direct 16-bit colour, empty frames, version/palette contracts,
  and isolated original drawing/conversion comparisons.

- [Engine modernization coverage ledger](coverage-ledger.md): subsystem scope,
  evidence, live replacement status and rules for measuring progress.
- [Threading evidence](threading.md): inspected timer/thread interfaces and
  limits of current simulation ownership findings.
- [Original world tick loop](world-tick-loop.md): No-CD message-loop/gameplay
  dispatch, world and creature updates, budgeted scheduling, pacing and separate
  timer callbacks; hash-checked static export, without live validation.
- [Original entity lifetimes](entity-lifetimes.md): creature slot allocation,
  cleanup versus release, reuse and reference repair; secondary missile/effect
  admission, third map-linked pool, and world teardown dependencies. Static only.
- [Creature AI, combat and spells](creature-ai-combat-spells.md): command ingress,
  behavior/action dispatch, staggered targeting, event-driven damage, cast and
  effect paths, dependencies and remaining validation. Static only; includes a
  [104-ID spell dispatch checklist](spell-dispatch-inventory.md).

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

- [Audio voice scheduler](audio-voice-scheduler.md): free/expired selection, tail eviction, old-volume ordering and assignment; offline clock/ring/PCM fixtures.

- [Positional audio](positional-audio.md): wrapped coordinates, approximate distance, signed map-byte attenuation, orientation pan and existing voice updates; offline fixtures.

- [Selected ANI forward controller](animation-forward-contract.md): bounded native
  selection/timing model and isolated original start/tick comparisons.

- [Audio camera projection](audio-camera-projection.md): camera origin, four-orientation screen-to-map projection, wrapping and listener integration; offline fixtures.
- [Audio attenuation map](audio-attenuation-map.md): signed-byte lookup, enable/distance gates, shared world ownership and bounded owned snapshots; offline fixtures.
- [Voice admission](audio-voice-admission.md): manager start gates, source lookup, duplicate retention, scheduler publication and source-ring rotation; offline/native PCM fixtures.
- [Source cache](audio-source-cache.md): backward scoring, pinned/busy/duplicate guards, group preloads, destructive replacement and Qt-backed synthetic WAV admission; offline fixtures.
- [Audio manager configuration](audio-manager-configuration.md): profile tables, randomized groups, map class lists, source budgets/preloads and scheduler pool initialization; offline fixtures.
- [Audio manager lifecycle](audio-manager-lifecycle.md): aggregate startup, exact failure ownership, shutdown/destructor ordering and retained primary/scheduler resources; offline fixtures.
- [Native audio manager backend](native-audio-manager-backend.md): Qt profile/WAV input through recovered startup/admission to native PCM, duplicate ownership and teardown; offline integration.

- [Bounded native ANI/SPR scene](native-animation-scene.md): explicit sequence
  playback, owned upload cache, Qt preview and offline composition evidence.

- [MPS placement reader and callers](mps-placement-loading.md): hash-pinned version-1 reader, 40-byte records, named kinds and selected section-coordinate consumers; static/offline evidence.

- [Installed native audio manager](installed-native-audio-manager.md): catalog/upload/admission, independent PCM previews, completion and cleanup; quoted-path/missing-file gaps remain explicit.

- [EVT event-area reader/writer and callers](evt-area-loading.md): hash-pinned 72-byte schema, section-coordinate consumers and offline scope.
