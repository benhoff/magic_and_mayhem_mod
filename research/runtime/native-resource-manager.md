# Native semantic resources and bounded uploads

2026-10-06. This milestone introduces explicit native resource ownership between
the existing read-only asset loaders and OpenGL sprite renderer. It is a native
policy, with no new original function/address mapping or binary modifications.

`AS.resource-manager` covers canonical kind/name identities, immutable lazy
recipes, owned complete decoding, paired ANI bitmap/sequence admission, explicit
CPU retirement, fresh load revisions and bounded residency. `NR.resource-cache`
covers value-keyed frame uploads, LRU and shared renderer budgets, revision-aware
GPU retirement, native bitmap RGB565/indexed policy, and the offline recipe
preview. The [public contract](../../assets/resources/README.md) defines default
budgets, charged storage and reference/thread lifetimes.

## Evidence and confidence

Synthetic manager checks use independently encoded SPR/ANI/TTD/BMP/PCX plus a
locally encoded JPEG. They exercise category recipes, stable identity across
insertion order, immutable binding conflicts, owned data after input removal,
complete-load failures/retries, paired frame/sequence rejection, binding/count/
exact byte budgets and cross-thread refusal. They do not newly establish decoder
equivalence or JPEG codec fidelity.

The GPU fixture checks complete expected RGB565 images, signed SPR origins,
transparent holes versus opaque zero, resident-hit upload/readback counters,
LRU order including empty frames, clipped draws, mutated/equal colour tables,
new-revision pixels after decoded unload/file replacement, BMP/PCX/JPEG input,
exact/local/external handle budgets and zero terminal resources. Independent CLI
BMP checks cover PNG dimensions, idempotent resident drawing, recipe admission,
existing-output refusal, asset-root output refusal and unchanged input bytes.

The reproducible runner additionally runs relevant path/file, SPR/ANI/TTD,
BMP/PCX/JPEG and sprite/OpenGL primitive regressions. Optional installed Redcap
loads bind `creature:10` to caller-selected normal-memory ANI/SPR paths, draw
frame zero, verify a resident hit, emit PNG and retire resources. It verifies
originals before/after and pins unchanged installed inputs. This smoke is native
integration; it does not execute an original resource loader or compare original
gameplay pixels.

The [retained runner report](native-resource-manager-20261006.json) records all
14 passing checks, the installed PNG/input hashes and successful immutable
manifest checks. It is registered as `AS.resources.native-20261006`. Confidence
is high within these explicit native policy tests; historical loader, scene and
original comparison results retain their own source fingerprints and scope.

## Remaining boundaries

- No automatic complete creature/effect/terrain/UI catalog population. The caller
  assigns semantic IDs and explicit paths/sequence recipes; existing recovered
  CFG/effect catalogs remain separate inputs for later binding adapters.
- No complete original terrain-definition-to-SPR admission, action names,
  attachment/effect printer execution, original lighting or palette animation.
- No simulation scene snapshot, wire resource protocol, Qt-shell integration,
  original asset-loading suppression, GPU context-loss recovery or live replacement.
- Resident decoded bytes are bounded; total process RSS, allocator overhead and
  decoder scratch are not the resident-byte metric. Actual OOM/device failures,
  revision exhaustion and non-Linux hosts are not induced by these tests.

Offline resource ownership, native synthetic integration and installed smoke
remain separate from original comparison and live replacement in the register
and coverage ledger. No gameplay balance behavior changes.
