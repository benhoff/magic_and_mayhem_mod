# Public launch preparation

`HOST.startup-preparation` records an intentional host policy, independent of
recovered engine behavior. Source inspection confirms the branches below; the
bounded checks cover preparation rather than live engine equivalence.

Ordinary `tools/run-qt-shell.sh` launches set `MNM_CACHE_ADAPTERS=1` and
`MNM_VERIFY_ASSETS=0` unless explicitly overridden. Standalone preparation and
experiment helpers default to fresh builds and full asset scans. The separate
original-media verification option remains documented in the Qt shell guide.

The shell reuses a CMake configuration with the same source directory, build type
and test setting when a generated build system is present. It always runs the
incremental build, including CMake's regeneration checks. Changes to the selected
settings, missing configuration/build files or `MNM_QT_RECONFIGURE=1` trigger an
explicit configure step.

The optional adapter cache covers production menu, rendering, native graphics,
audio, scene/glyph and direct-word DLLs. Its key includes conservative shared
runtime/rendering/protocol source content, builder/cache code, variant options
and compiler/linker/tool executable paths, sizes and modification/change times.
Unrelated research and test-harness edits do not invalidate it. Cache reuse
checks DLL and build-manifest hashes. Builds whose inputs change while compiling
refuse publication; a mismatched build manifest also refuses. A builder lock
serializes cache misses, and the receipt is published last. Cache artifacts live
under `working/build/launch-adapters/`; no historical artifacts are deleted.
`MNM_CACHE_ADAPTERS=0` forces fresh builds. Toolchain identity uses file metadata,
not an assertion of identical compiler contents under arbitrary metadata spoofing.

With full scans disabled, menu/OpenGL staging omits the bulk installed-asset
fingerprint file, and its runner omits the corresponding full-tree rehash.
The manifest binds that choice as an explicit Boolean; absent fields retain the
historical full-scan default and invalid field types refuse. Repeated Python
hashing of prepared music tracks is similarly omitted. Music catalog identity,
track names/TOC, file presence and symlink refusal still run; the native music
catalog loader retains its strict WAV/hash validation. No normal launch is
evidence that every installed asset matches a previous result.

Executable, callback-byte, adapter, staging-configuration and fresh wire-channel
admission remain mandatory. Native graphics, input, audio and media selection
are unchanged. `MNM_VERIFY_ASSETS=1` restores the full preparation/runner scans
for diagnostics. Experiments consuming original artifacts still require
before/after original-manifest verification.

The Qt launch log is initially hidden behind **Details**. Preparation/startup
messages remain visible, and failures point to Details. Logs still accumulate
with the existing 2000-block bound and can be shown or closed without restarting.

## Bounded checks and remaining boundaries

The source-bound [validation summary](public-launch-startup-validation-20261012.json)
retains fresh test/CLI results and the hashes/content of the preparation reports.

- `tests/test-launch-build-cache.py`: reuse, variant/source/tool invalidation,
  corrupted/missing/symlink artifacts, standalone defaults, explicit bypass,
  changing compiler inputs and mismatched manifest refusal.
- `tests/test-qt-launcher-startup.py`: configuration reuse with incremental builds,
  setting changes, missing/wrong configuration, forced configuration, argument
  preservation and build failure preventing launch.
- `tests/test-native-music-launch.py`: full-scan admission/refusals, scan-disabled
  staging and admission without hashing PCM, missing files and invalid policy.
- The Qt application builds and passes its shell smoke check; existing audio,
  media and voice admission regressions retain their selections/refusals.
- `working/tests/startup-adapter-cache-final-20261012.json`: real production
  builds and verified reuse for five default-route adapters. Reuse totals about
  0.16 seconds versus about 7.27 seconds preparing the uncached artifacts on this
  host. This is adapter preparation timing, not end-to-end startup or frame rate.
- `working/tests/startup-menu-staging-final-20261012.json`: production menu/native
  staging with full scans disabled/enabled, preserved pinned source and original
  manifest checks before/after. Both cases preserve executable admission and the
  chosen scan policy. One cold and one warm filesystem run do not establish an
  end-to-end speed comparison.

The default `working/game-nocd/Chaos.exe` currently hashes to
`1060a739ae220f4dfcf0e86b848df229fd5e37ab253d3d000e39ca2601c6c4cb`, whereas
staging requires `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Its unsupported-build refusal was retained, and that installation was left
unchanged. Offline staging used the existing preserved
`working/experiments/native-music-input/run-fliws_ct/game` instead.

No fresh Wine/gameplay comparison, live integration or replacement promotion is
claimed. Shared launcher/UI edits leave some older registered source fingerprints
stale; historical evidence hashes remain unchanged. The incoming committed file
review is retained separately in
`coverage/public-launch-startup-incoming-history-20261012.json`.

A concurrent hard-restart change subsequently edited the shared
`apps/qt-shell/main.cpp` after the retained startup support checks. That result
continues to describe its recorded source hash; it does not validate the later
shared-file version. Current UI integration remains pending until rebuilt and
checked with those independent session changes.

The final combined-source rebuild was refused by the retention preflight: free
space was about 100.24 GiB, and a 0.5 GiB reservation would cross the required
100 GiB reserve. The next space reading was below the reserve due to concurrent
writers. No cleanup, archive, budget override or further build was performed.
The full gate also observes those independent source/accounting changes; the
startup scope retains an exact pending review without asserting a whole-tree pass.

After free space recovered, a later guarded incremental Qt build succeeded.
Fresh CLI-policy and shell-construction smoke checks passed with stable combined
UI sources; the new result is retained separately in
[the final UI check](public-launch-startup-ui-final-20261012.json). This resolves
the startup UI rebuild boundary for that recorded source version, without
claiming live gameplay/hard-restart validation or a whole-tree accounting pass.

Commit scope: startup preparation and the Details panel are independent of
the separate hard-restart feature. The first support record fingerprints the
startup-only source version. The later combined UI report is retained as
historical context, with its independent process/session source dependencies;
it is not registered as validation of this isolated commit.
