# Original-executable import call census

This opt-in observation counts calls through the pinned NoCD
executable's190 import slots after native adapters install. It will retain
original import module/symbol labels and resolved target DLL basenames, with
separate classifications for Windows/Wine targets, toolkit native shims and
the legacy JPEG codec. Static imports establish availability, not execution.

Each proxy saves flags, atomically increments a low/overflow counter, restores
flags and tail-jumps to the unchanged target. Registers and caller arguments
are untouched. Original target pointers stay private to the PE process; shared
records contain build-specific RVAs, names and counters. Final counts require
the observed game process to stop; a DLL-detach marker alone is insufficient.

This is an original executable import boundary. It excludes dynamic calls,
COM/vtables, native-shim forwarding through cached pointers and transitive Wine
or prefix activity. It cannot establish a complete Wine API total or absence.
The forwarding/decoder implementation and bounded native campaign pass.
Current counts and explicit exclusions appear in the milestone below.
The initial full campaign result is retained in
wine-import-census-initial-provenance-failure-20261012.json; its compiler closure
used a noncanonical header path. Canonicalization fixed that provenance failure,
and the fresh run passes without rewriting its predecessor.

## Observed original import call census (2026-10-12)

The optional pinned PE32 observer installs after existing native adapters and
forwards all190 original import slots with saved flags, atomic low/overflow
counters and unchanged registers/arguments/stack/targets. Target pointers remain
private; version1 MNMAPI01 stores RVAs, labels, classifications and counters.
Staging checks original hash/catalog and runtime guards/readability/protection
before mutation. Final decoding requires stopped private process and exact
original metadata/reserved/header/classification checks. See wine-import-census.md.

The fresh synthetic PE checks pass stdcall/cdecl/fastcall, arithmetic flags,
ECX/EDX/LastError, overflow and10000 repeats, plus ten malformed/active decoder
refusals (wine-import-census-data-synthetic-20261012.json). Five shared protocol checks
pass. Current full native default voice/WAV/factory/media/CD/render/menu/campaign
and normal Quit pass with all190 counters installed and DSOUND disabled.
wine-import-census-data-public-pass-20261012.json records 89
distinct exercised direct Windows/Wine import targets, 13756840
calls, 8 exercised native shim imports and
1 legacy codec import. Counts are read after owned
Wine shutdown. This is a defined import boundary, not a full Wine API count:
dynamic, COM/vtables, shim forwarding/cached pointers, transitive and prefix
activity remain excluded. Audio adapter unsupported/fallback/errors remain zero.
Exact0144cb6c history reviews28 transitions without receipt gaps; no historical
validation is refreshed. Native manager/config/cache/events, initial no-device
policy, media/physical recovery, long playlist/waveform/listening and broader
executed API coverage remain pending.

The initial code-address classification for the import table was corrected to a
data relationship before this fresh prospective run. The earlier successful
public result retains its original claim fingerprint and is historical.
