# PCM WAV input for native buffers

The engine's selected WAV path uses WinMM mmioDescend/mmioRead to find RIFF
WAVE, `fmt ` and `data` chunks; inspected helpers are `0x0058ff10`,
`0x005900d0` and `0x00590140` in the pinned no-CD executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The static secondary wrapper clears cbSize before CreateSoundBuffer. The read
helper returns its requested bounded count without checking mmioRead's actual
return. That is confirmed original behavior, not replicated by the strict
native reader.

Native `audio::readWave` reads fields explicitly as little endian:

| Format payload offset | Type | PCM field |
|---|---|---|
| 0 | u16 | Tag, 1 |
| 2 | u16 | Channels, 1 or 2 |
| 4 | u32 | Sample frames per second |
| 8 | u32 | Bytes per second |
| 12 | u16 | Frame alignment in bytes |
| 14 | u16 | Bits per channel sample, 8 or 16 |
| 16 | u16, optional | Extra size, zero for supported PCM |

The RIFF size bounds chunk walking. Chunk payloads are padded to even byte
boundaries; padding is not PCM. Unknown chunks are skipped. Format and data can
appear in either order. A 16-byte PCM format or an 18+-byte format with zero
extra size is accepted. The reader requires consistent alignment and byte rate,
nonempty data and whole frames. Truncated containers/chunks/padding, duplicate
format/data chunks, non-PCM, nonzero extra size and other channel/sample widths
are rejected. These are native validation policies, not a claim that the old
engine had identical validation.

PCM samples are retained without conversion or resampling: unsigned 8-bit or
little-endian signed 16-bit, interleaved when stereo. Playback conversion/mixing
is a later layer. The upload CLI caps input files at 32 MiB and native buffers
at 16 MiB by default.

Evidence: `tools/test-audio-buffers.py` compares all 356 installed Sounds WAVs
against Python `wave`, with byte-exact native dumps. Report:
`working/tests/audio-buffers/run-b2dc_l93/report.json`, 11,908,772 PCM bytes.
All inspected Sounds files were mono PCM; observed formats were 8-bit/22050 Hz
and 16-bit/11025, 22050 or 44100 Hz. Synthetic tests cover stereo and malformed
RIFF input. Confidence: high for these files and the validated native parser;
other game audio containers and compressed WAV formats remain unexamined.
