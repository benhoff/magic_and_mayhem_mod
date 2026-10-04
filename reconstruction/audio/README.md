# DirectSound setup reconstruction

This directory contains build-specific engine models, not native application
code. `dsound_setup` reconstructs device request parameters, primary and static
secondary descriptors, the primary setup call sequence, and the successful
static sample upload contract.

`DeviceRequest` records the default DirectSound device, no aggregation and
cooperative level 2. It does not reconstruct the manager's configuration/file
parsing or install a Windows hook. `PrimaryBackend` makes the observed API call
order independently testable, including exact nonzero HRESULT propagation.
The outer manager retains cleanup ownership after primary creation fails at
a later step. The original ignores primary GetCaps failure; the model records
unknown buffer size instead of reading uninitialized bytes.

The descriptor uses five DWORDs, including a 32-bit format address. Encoders
produce explicit little-endian 20-byte DSBUFFERDESC1 and 18-byte WAVEFORMATEX
payloads; host pointer size and structure padding do not enter those payloads.
Native PCM metadata is an ordinary C++ value, not a packed Windows structure.

`primaryFormat()` preserves the original fixed 22050 Hz, 88200 bytes/second
and alignment 4, while choosing channels/bits from capability flags. Its
mono/8-bit fallbacks are internally inconsistent; the reconstruction does not
normalize them. The native Device advertises its mono/stereo and 8/16-bit
policy, which selects the consistent stereo/16-bit primary request.

`uploadStatic()` applies the full-buffer, offset-zero, flags-zero path to a
strictly validated native WAV. It preserves all PCM bytes and cleans up failed
native uploads. It does not emulate unchecked mmio reads or arbitrary legacy
error/partial-read behavior. Native storage is in `audio/`; playback and the
game's higher-level voice scheduler are future work.

Build/tests are described in [audio/README.md](../../audio/README.md).
Native `audio::loadWave(AssetFile&)` now feeds the strict parser through the
Qt-backed read-only interface. The offline upload CLI closes input before
`uploadStatic`; reconstructed setup/upload algorithms and native PCM policies
remain unchanged. This is a native input adapter, not reconstruction of the
original manager's file/config loader or WinMM error behavior.
Static evidence and address confidence are in
[DirectSound setup](../../research/runtime/directsound-buffer-setup.md).
