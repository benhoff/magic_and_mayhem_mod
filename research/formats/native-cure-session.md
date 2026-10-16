# Native Cure lab continuation, version1

This is an authored lab format, independent of original saves and the native
campaign envelope. It stores owned values only, never C++ object bytes, pointers
or original addresses. Magic bytes are `MCUR`, followed by version DWORD1.
All integer fields are little-endian DWORDs; a64-bit tick is low/high DWORDs.
Signed integers preserve two's-complement bits; Booleans are0/1; absent logical
indices are `ffffffff`. Vectors have a DWORD count followed by explicit fields.

`apps/cure-lab/session.cpp`'s `content` function owns the field order: header;
tick/configuration/dimensions/resource counter; actors (position, complete
ordinary health state, maximum, ongoing damage, resource/status/audio reference,
visibility); effect slots (active/visible/generation and full Cure projection);
audio manager flags/scratch, buffers, external outputs and eight-field schedule
records; learning flags/notices; observed-owner table. Ring links are zero-based
node indices, while voice/output IDs are one-based with zero absent.

The final DWORD is FNV-1a32 over all preceding bytes for accidental-corruption
detection. Restore checks the checksum, version, lengths/counts, trailing bytes,
Boolean/learning values, actor/config domains, generation/effect references,
unique status voice ownership and valid associated scheduler rings before
constructing the candidate. Maximum file size is8MiB, actors/effects4096,
audio vectors65536 and notices20. Counts are checked before allocation.

The continuation retains the deferred phase and pool generations; resumed input
must supply a current generation handle and explicit animation result. No guessed
wall-clock deadline, decoded asset, physical audio state or external file path is
saved. Source-bound native tests compare serialized state and ordered events
after admission, deferral, waiting and completion, including blocked/dead targets
and failed audio operations. Complete original-save/campaign compatibility is not
claimed.
