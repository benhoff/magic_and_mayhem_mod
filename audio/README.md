# Native sample storage

Native audio implementation belongs here, separate from the pinned engine
models in `reconstruction/audio/`. This first chunk owns PCM buffers and sample
uploads. It does not implement a mixer, an audible device, or a live DirectSound
replacement. The game still uses Wine DirectSound.

```bash
cmake -S audio -B working/build/audio
cmake --build working/build/audio --parallel 4
ctest --test-dir working/build/audio --output-on-failure
python3 tools/test-audio-buffers.py
```

The integration check reads every installed WAV with Python's independent WAV
reader, uploads through the reconstructed static path, duplicates the voice,
releases its source, and compares the survivor's sample dump byte for byte.
It verifies the immutable original manifest before/after and does not launch
the game. Evidence is under `working/tests/audio-buffers/`.

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

The planned Qt-backed input adapter is specified in the
[asset file interface contract](../research/formats/asset-file-interface.md).
It will replace direct upload-CLI file input while preserving the WAV parser,
sample ownership, and upload checks. The contract and standalone path resolver
are complete; the file backend and audio integration remain pending.

Next: reconstruct play/stop/loop, frequency/volume/pan and playback cursors;
introduce independent voice state and an offline mixer before a Qt audio sink
or live game hook. See [evidence and boundaries](../research/runtime/directsound-buffer-setup.md).
