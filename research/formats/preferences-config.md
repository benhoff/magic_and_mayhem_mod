# Original Preferences configuration keys

Recovered 2026-10-05 from the pinned no-CD executable (SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`).
The original settings constructor `0x004c1b70` sets filename `cfg\prefs.cfg`.
Reader `0x0054be80` uses Windows profile strings and original integer conversion;
writer `0x0054c890` emits profile keys. Case handling and formatting are delegated
to Windows profile APIs; no new native file parser or writer is implemented.

| Section/key | Receiver offset | Menu value / selected reader behavior |
| --- | --- | --- |
| VIDEO/IsHighRes | +0x0d | TRUE for High, FALSE for Low; renderer mode 2 forces Low on read |
| VIDEO/WindowSize | +0x11 | Menu writes border On=1, Off=0; reader admits integers 0..15 |
| VIDEO/CutDownAnims | +0x29 | TRUE for Cut, FALSE for Full |
| VIDEO/DialogSpeed | +0x2d | Fast=0, Medium=1, Slow=2; reader clamps 0..2 |
| VIDEO/MaxFramesPerSec | +0x35 | Menu writes 20/17/14; reader clamps 1..200 and derives integer 1000/rate at +0x31 |
| SOUND/MusicVolume | +0x41 | Original UI level 0..15; missing key uses 9, present value uses original integer conversion without this reader clamping |
| SOUND/SFXVolume | +0x45 | Original UI attenuation -2500..0; missing key uses -250, present value uses original integer conversion without this reader clamping |
| SOUND/SoundEnabled | +0x39 | Writer emits TRUE unconditionally |
| SOUND/CDMusicEnabled | +0x3d | Writer emits TRUE unconditionally |

The writer additionally emits mouse controls, FMV flags, transparency/alpha,
terrain light levels, network identity strings and tooltip delays. They are not
editable through the recovered Preferences screen. Future menu integration must
preserve their original receiver state and use the original writer. Terrain
count has its own [recovered contract](terrain-palette-preferences.md).

The supplied layout's -5000..0 slider ranges are overridden by engine control
initialization and do not define the live preference units. The native menu
sound INI is a separate application store and must not be treated as this file.

Evidence and validation boundaries are in the
[Preferences engine contract](../runtime/preferences-engine-contract.md).
Original writer calls were executed against a private profile recorder, including
an API-failure case. No original file-write/read-back or restart durability is
claimed; profile API errors are ignored by the original writer.

Subsequent [Main Preferences integration](../runtime/preferences-engine-bridge.md)
validates original writes to a disposable session file and reopened engine values.
This does not establish restart durability or persistence across launcher sessions.
