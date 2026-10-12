# Selected native audio configuration reads

Prospective native policy: session-owned read-only Sounds.ini snapshot, using
native AssetStore/ProfileSnapshot; independent of widgets and recovered manager
rules. Preserve installed section ordering, string quote/default/capacity results
and configured unsigned values. Malformed/duplicate/encoding/unsigned integer
rejections and snapshot rather than live reread are intentional native policies.

Pinned NoCD build40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168:
selected section calls56e03a/56e20b/56ea77, string calls56ebd9/56f6cb and cached
EBX string import load56e41f, integer call56e589. Original IATs5c50f0/5c507c/
5c50ec are retained for other callers. Evidence: guarded assembly exports under
working/decompiled/audio-support-objgyz0h and audio-support-_p0gupwd, existing
installed native/Wine profile comparison. Scope: these selected calls, not
complete functions or general Win32 profiles/registry/encoding semantics.

The existing MNMAUD01 v5 optional capability model can admit profile reads with
bit32; opcodes23..25 will carry bounded NUL-separated ASCII request strings and
copy returned bytes under the same single-request lock before reuse. Exact
instruction guards must precede all seven call/load mutations. Integration and
live replacement remain pending. Cache/scheduler/event rules still run in the
original executable; they must be compared independently before replacement.

## Installed transport validation

[Final installed result](native-audio-profile-installed-pass-20261012.json)
passes497 queries through the actual PE32 adapter/native worker against Wine's
original profile reader.8,144,836 result bytes match exactly, including normal
sections, strings, configured unsigned integer and missing names. Native
LastError-preservation policy passes; it is not a full Win32 error-semantics claim.
Seven individual altered-site tests each refuse before any mutation; a correct
synthetic image installs and a second installation refuses. The synthetic image
uses an allocated address, while production DllMain separately verifies the
pinned0x400000 executable base before reaching this helper.

67 current audio CTests and12 admission tests pass. Original manifests verify
before/after, input/source hashes stay stable, readiness withdraws and private
Wine stops. Compile-include failure run-_owa5e6o and compiler-closure failure
run-fu5rze99 are preserved. Runs run-mburrdft/run-yg08uxhd already matched all
profile results but failed the assumption that0x400000 was free for a fake
image; VirtualQuery reports an existing allocation at0x390000. Address-independent
synthetic image tests resolve that fixture limitation. Earlier passing
run-nom0r92i remains unchanged; final scope includes all test/campaign links
under run-3sv6spye. Live game and import-retirement validation are pending.

## Current live profile reads and import census

[Current full game](native-audio-profile-public-census-pass-20261012.json)
passes native intros/WAVs/voices/CD/render/campaign and normal Quit with all7
profile hooks,76 string reads,4 section reads and1 integer read.194 native
request ACKs, zero failed/fallback/unsupported audio calls and stable source/input/
original/private cleanup checks pass. Wine DirectSound remains disabled.

The final original-IAT census observes88 distinct
direct Wine-targeted imports (12781970 calls), out of190 original
imports. Dynamic/cached calls, COM/vtables, shim forwarding and transitive/prefix
Wine calls remain excluded. This is a measured boundary, not a total Wine API
retirement count. Native manager selection/cache/scheduler/event decisions still
execute in the original game and require separate migration/comparison.
