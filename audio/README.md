# Native audio storage, voice state and mixing

Native audio implementation belongs here, separate from the pinned engine
models in `reconstruction/audio/`. It owns PCM buffers, sample uploads and
independent voice state and offline stereo PCM mixing. It does not implement
an audible device or a live DirectSound replacement. The game still uses Wine DirectSound.

```bash
cmake -S audio -B working/build/audio
cmake --build working/build/audio --parallel 4
ctest --test-dir working/build/audio --output-on-failure
python3 tools/test-audio-buffers.py
```

The integration check reads every installed WAV through AssetStore/loadWave
using mixed-case relative and mapped drive-absolute Windows requests, compares
with Python's independent WAV reader, uploads through the reconstructed static path, duplicates the voice,
releases its source, and compares the survivor's sample dump byte for byte.
It verifies the immutable original manifest before/after and does not launch
the game. Evidence is under `working/tests/audio-buffers/`.

The build requires Qt 6.8+ Core for asset input and the CLI, plus Python 3 for
the fixture tests. `mnm-audio` and the reconstruction retain standard C++ APIs;
no Qt types enter those algorithms. The combined audio build runs eight CTests,
including the three asset tests and an input-adapter fixture check. No Wine,
display server, or audio output device is needed for native builds/tests.

`Device` is thread confined. IDs identify voices within one Device; they are
not process pointers. Secondary duplicates share committed storage. A lock
provides one or two temporary write regions, and a valid unlock commits only
the reported written bytes. Pending writes are not visible in snapshots.
Releasing the lock owner abandons its pending write without corrupting other
voices. Releasing the original voice does not retire surviving duplicates.

Native policy is explicit: one primary metadata object, PCM mono/stereo 8/16-bit,
the observed `0x81` primary / `0xea` secondary creation flags, at most 128 IDs,
and 16 MiB per sample buffer by default. Multiple simultaneous writers to shared
storage return busy. Write-cursor locks and other creation flags return
unsupported. These policies do not claim full DirectSound emulation. Primary
metadata has no sample allocation until a mixer/output path is designed.

For a single offline upload:

```bash
working/build/audio/mnm-audio-upload "working/game-nocd/Sounds/Spell click.wav" \
  --dump working/tests/spell-click-pcm.bin
```

The JSON output describes format, copied bytes, revision and retirement. The
dump is raw PCM, not another WAV container. Source WAVs are read only.

For installation-relative Windows requests:

```bash
working/build/audio/mnm-audio-upload --assets working/game-nocd \
  --path 'sOuNdS\SPELL CLICK.WAV' --dump working/tests/spell-click-pcm.bin
working/build/audio/mnm-audio-upload --assets working/game-nocd \
  --prefix 'C:\MagicMayhem' --path 'c:\magicmayhem\Sounds\Spell click.wav' \
  --dump working/tests/spell-click-pcm.bin
```

The positional host-path form remains supported: its parent becomes the asset
root and its filename the request. Use either one host path or the paired
`--assets`/`--path` options. `--prefix` is repeatable in installation-root mode.
Output must be outside the configured asset root, protecting input files from
accidental replacement. Existing dump output is replaced only after successful
input parsing and upload. Invalid CLI combinations exit 2; input/parser/upload
failures exit 1; success exits 0.

The Qt-backed input adapter is specified in the
[asset file interface contract](../research/formats/asset-file-interface.md).
`mnm-audio-loader` implements `loadWave(AssetFile&)` by bounded `readWhole`
followed by the existing strict `readWave` parser. It rewinds the file and
retains the 32 MiB input limit; Device retains the 16 MiB sample limit. Asset
failures throw `AssetInputError` carrying structured diagnostics; parser
failures preserve existing runtime errors. Returned PCM owns its bytes.
The upload CLI closes its input before calling reconstructed `uploadStatic`,
then checks duplication and source-voice retirement exactly as before.

Chunk 5 completes this first native input/loader connection. Fixture tests
check legacy and explicit request modes, nested case matching, exact PCM after
input closure, malformed files, the input/sample limits, missing/traversing
paths, rejected output locations, and CLI errors. Raw installed-file checks
remain separate from decoded PCM comparisons. See
[WAV integration evidence](../research/formats/pcm-wav-loading.md).

Selected play/stop/loop, volume/pan, status/reset and deadline contracts now have
[offline reconstruction tests](../research/runtime/directsound-voice-controls.md).
Frequency changes and arbitrary cursor queries remain conditional.
Native `Device` now supports play, stop, zero reset, volume/pan and independent
source-frame advancement. `voice(id)` reports state by value; `advanceFrames`
does not produce samples. Use `python3 tools/test-audio-voice-state.py` for
fixture-only validation. See [state policies and evidence](../research/runtime/native-audio-voice-state.md).

Use `Device::mixStereo(frames, output)` to produce interleaved signed 16-bit
stereo values at the fixed output rate (48 kHz by default). It advances voices,
resamples with linear interpolation, applies volume/pan and clips the final sum.
Use `python3 tools/test-audio-mixer.py` for fixture-only validation. See
[mixer policies and evidence](../research/runtime/native-audio-mixer.md).

Next: Qt audio output, then a live game adapter. See [reconstruction boundaries](../research/runtime/directsound-buffer-setup.md).
