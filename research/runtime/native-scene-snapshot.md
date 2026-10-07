# Original display queues connected to native assets

The [complete live drawing replacement plan](../../docs/live-drawing-replacement.md)
owns implementation order and replacement criteria. This document owns the
bounded scene snapshot contract, evidence and unsupported modes.

Step 3 adds bounded observation of original simulation display queues and native
asset-backed replay. The original engine still schedules simulation and draws
its own window. Captured queues can be rendered through `SceneRenderer` using
native SPR loading and resource IDs; no original runtime pointers are retained
by the host. This milestone does not claim continuous native presentation or
replacement of original drawing.

## Recovered boundary and evidence

The pinned No-CD executable is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
At consumer `0x005002a0`, ECX identifies the queue: base at +0, next at +4,
count at +8, capacity at +12, grid at +16, view at +20. This boundary follows
sorting and visibility; records preserve consumer order. The existing sprite
queue research supplies the selected field layout, not complete renderer
recovery. Original records are 36 bytes: depth +0, SPR frame pointer +4,
anchors +8/+12, packed shade/secondary parameters +16, owner pointer +20,
kind +24, depth sentinel WORDs +28, and an untouched word +32. Owner pointers
and the untouched word are not exported. Original viewport globals observed
are `0x006a49b8`/`0x00656618`; the consumer mode flag is `0x005e1404`.

`runtime/scene/observer.c` validates readable extents, copies bounded unique
frames, normalizes only the palette pointer, and forwards the original consumer.
Preparation verifies the executable hash and nine complete entry instruction
bytes `83 ec 18 53 b8 00 6c ca 88` before staging a scripted import addition.
The trampoline preserves those instructions; the entry saves/restores integer
registers, flags, x87/SSE state and LastError. The PE32 self-test executes an
independent fake consumer with the exact prologue: signature refusal, repeated
installation refusal, unreadable queue forwarding, sample bounds, unchanged
queue/frame pointer, original return/LastError and x87/XMM/MXCSR preservation.
This validates forwarding within the fixture, not every real call-site ABI.

Live captures use opt-in `MNM_SCENE_DIR` and `MNM_SCENE_SAMPLES` (1..16), an
isolated staged installation, repository launcher, menu observation channel and
fresh Wine prefix. The established diagnostic menu fixture selects a single
player map and disables its spell-selection handoff locally. Immutable original
manifest checks surround staging/capture/installed replay. Original drawing
continues during capture. The observer drops reentry and performs bounded disk
writes at the call boundary; no threading stress or performance suitability for
continuous capture is claimed. Failed writes consume a sample slot and partial
files are refused. The hook is process-lifetime, without hot unload.

## Native mapping and intentional policies

`compat/legacy/scene_snapshot.cpp` owns and checks the versioned wire payload.
`SnapshotResources` indexes explicit SHA-256-pinned version-4 SPR candidates
through read-only `AssetStore`, associates normalized encoded frame identities
with resource IDs/frame indices, and binds `ResourceManager` lazily. Indexing
decodes assets temporarily but retains identity/visual hashes, not decoded
frames. Replayed frames are owned native resources. Mapping refuses unknown
identities and aliases with different opaque RGB565 pixels, masks or origins;
identical aliases choose manifest order and lowest frame. A loaded frame is
checked against its indexed visual identity, including after unload/reload.
The observation IDs use an explicit `ui:` namespace; they are bitmap bindings,
not assertions of terrain/creature gameplay semantics. Asset roots must remain
read-only throughout a replay; source/input stability is checked by the runner.

Native replay supports original kinds 0 and 33 only when both original depth
sentinels are `0x8ad0`. Hidden kind -2 is skipped. Other kinds, sentinel
adjustments and unreadable frames are retained as indexed gaps. Strict replay
refuses incomplete mapping; `--supported-only` explicitly permits a partial
preview. Draw order and anchors come from the original queue, while SPR origins,
opaque masks and embedded palette RGB565 values come from native assets.
`--unshaded` is mandatory: original lighting/palette remaps, blending/effect
modes, background and HUD are not reconstructed by this adapter. Its background
is deliberately diagnostic 0x1234. Captured display records are not semantic
world snapshots, clocks or stable creature identities.

## Validation and remaining work

`tests/scene-snapshot-test.cpp` covers complete independent overlap/opaque-zero
pixels, draw order, owned snapshots, malformed admission, lazy residency,
ambiguous palettes, changed native assets and surface retirement.
`tools/test-scene-snapshot.py` retains source/binary/input hashes, runs the PE32
forwarding fixture and native CTests, and independently decodes SPR runs in
Python to compare every native RGB565 pixel. Optional original capture replay
also checks every draw's anchor and byte-exact normalized asset identity against
the captured record. These comparisons validate the stated unshaded native
policy, not the original shaded window. Original captures and aggregate reports
are registered separately so live observation never implies pixel equivalence.

Unsupported draw modes, original palette remapping, independent complete batch
output and continuous scene delivery are dependencies tracked in the replacement
plan. Animation and
simulation scheduling remain controlled by the original engine until their own
contracts are validated. Gameplay changes remain outside this workstream.
