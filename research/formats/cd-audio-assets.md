# Native CD audio assets

The supplied CUE/CCD names one BINARY image with data track1 and audio tracks2
through13. The image contains 340224 sectors of2352 bytes. Audio is stereo
signed16 PCM44100: each sector has588 stereo frames. CUE BINARY stores samples
in little-endian order ([GNU CUE reference](https://www.gnu.org/software/ccd2cue/manual/html_node/FILE-_0028CUE-Command_0029.html)).

`tools/extract-cd-audio.py` reads the image without modifying it and writes new
WAVs under `working/`. It refuses overwrite, traversal, symlink image inputs,
unsupported CUE statements, partial sectors and unordered/out-of-range indices.
Each output records its sector range, frame count and raw PCM/WAV SHA-256, with
original CUE/image hashes checked before and after. Immutable manifest checks
surround preparation. Bounded streaming avoids loading the full disc into RAM.

The explicit native span policy is INDEX1 to the following INDEX1, or image
leadout for the final track; it retains the next track's preceding pregap in the
current WAV. Original MCI track addressing, looping, end notifications, volume
and event routing remain unvalidated. These are native assets, not proof that
the game's CD player has been replaced. Confidence is high for the declared
sector/PCM extraction; original playback semantics remain provisional.
