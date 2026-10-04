# Native menu effects preferences

## Scope and implementation

Native application policy; not a recovered Windows registry/game configuration
contract. `--menu-audio` now creates an explicit user-scoped INI QSettings store
with organization `MagicAndMayhemMod` and application `QtShell`. On Linux its
normal destination is `$XDG_CONFIG_HOME/MagicAndMayhemMod/QtShell.ini` (or
`~/.config/MagicAndMayhemMod/QtShell.ini` when XDG_CONFIG_HOME is unset).
Other platforms follow Qt's user-scoped INI location rules.

The versioned key is `audio/v1/effectsLevel`, in the native integer attenuation
range -10000 through 0. Missing, noninteger, overflowed or out-of-range values
use the existing -1000 default without repairing or rewriting the file. Qt
settings fallback locations are disabled. The controller accepts an optional
application-owned QSettings object; fixtures inject a temporary INI and existing
callers without a store retain their in-memory behavior.

The controller reads once at construction and applies the accepted gain before
session output starts. The preview receives that semantic volume separately
from persistence and seeds its lazy Preferences screen with it. Opening the
screen does not replace restored volume with its default. Accepted screen
settings remain visible on subsequent visits; draft edits and Cancel do not
write or change the active volume.

Only Preferences acceptance writes and synchronizes the effects key. Other
preferences are not saved by this integration. Acceptance while output is
unavailable still saves and updates the session's retained gain; recovery starts
silently with that gain. A write failure reports through the controller failure
callback (stderr in the shell), keeps the accepted session value, and does not
stop audio. It does not promise that the next process will recover a failed
write. Application lifetime keeps the store alive until after controller teardown;
QPointer prevents dereferencing an injected store after its destruction.

## Offline validation

The shared worktree contained a parallel, unfinished realm-viewer change whose
CMake test source was not yet present. Validation therefore used an isolated
committed-source snapshot with only this chunk's code/test changes. No parallel
feature edits were removed or included in the validation snapshot.

`tools/test-menu-audio.py` passes all 24 scoped CTests, including:

- Generated WAV output proving restored gain is applied to the first cue before
  Preferences opens; a fresh child process reads the saved INI and repeats this.
- Lazy Preferences restoration and repeated navigation without duplicate bindings.
- Accepted gain written to disk, Cancel preserving the saved bytes, no startup
  writes, and music edits leaving effects gain unchanged.
- Missing and eight malformed/out-of-range values, mute/full-volume/intermediate
  boundaries, and unchanged stored bytes on load and cue playback.
- Acceptance while output is absent, subsequent device-availability recovery,
  and a failed settings destination retaining the accepted PCM gain.
- Existing menu, audio-session, recovery, manager and queue regressions.

Evidence: [report](../../working/tests/audio-preferences/source/working/tests/menu-audio/run-58bblhf6/report.json)
and [CTest log](../../working/tests/audio-preferences/source/working/tests/menu-audio/run-58bblhf6/ctest.log).
Inputs are synthetic; no game launch, installed/original asset read or physical
output device was used. No immutable-input experiment was required.

Confidence: high for the tested file, acceptance, synthetic PCM and restart
behavior. Real user configuration-directory permissions and crash/disk-loss
behavior are not measured. A stored level incompatible with a different asset
configuration's narrower Preferences slider range yields the existing bounded
snapshot validation error rather than silently changing volume. Music volume,
other preference persistence, original settings import, physical playback and
live original audio routing remain separate work.
