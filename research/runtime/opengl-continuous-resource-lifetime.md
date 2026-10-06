# Continuous resource identities and final Release

2026-10-06. Native policy `NR.continuous-resource-lifetime` implements the second
continuous-producer chunk. This is a synthetic native integration increment;
original COM/driver and game equivalence, orchestration and recovery are separate.

## Contract

With `MNM_RENDER_CONTINUOUS=1`, each admitted CREATE receives a monotonically
increasing nonzero wire ID. The ID is independent of the 32-slot session table
and opaque application interface address. Retired IDs never reappear. Exhaustion
of the UINT32 ID lifetime refuses instead of wrapping; this endpoint has not been
executed in a full lifetime fixture. No additional channel or command opcode is
needed: the streaming consumer already enforces increasing IDs and owns DELETE.

The existing Release hook invokes the original call first and preserves its
return value and LastError. Only an original zero reference count supplies final
retirement evidence. The callback uses the destroyed object's address strictly
as an opaque token: it makes no observer COM call, dereference or extra reference.
Observed successful application QueryInterface relationships resolve the entire
interface component while its alias edges still exist. An admitted final Release
emits exactly one ordered DELETE, decrements live session pixels and clears its
slot. Existing tracker retirement then clears component snapshots, metadata,
locks and aliases, and removes backbuffer links. Unrelated resources remain live.

The next checkpoint at a reused address or slot emits CREATE with a fresh ID and
fresh layout/palette state. No generation is inferred from stable pointer values.
A nonfinal Release emits no DELETE; an untracked final Release emits no command.
The 32 simultaneous surfaces, 16,777,216 simultaneous pixels, 64MiB snapshot
storage and existing finite queue/ring/sequence/byte budgets remain enforced.
Lifetime creation count can exceed those simultaneous-storage capacities.

## Refusals and remaining branches

A tracked final Release with an active lock or suspended DC refuses the session;
unresolved borrowed work is not serialized as successful deletion. Alias
ambiguity, a missed/contended final Release, epoch changes, unsupported restore
or layout changes, and successful CreateSurface at a still-tracked address
without confirmed retirement retain conservative stream refusal. Alias table
saturation still invalidates provenance. No tombstone recovery, guessed identity
merge, silent resource eviction, session restart or ID reuse is introduced.

Default bounded sample/sequence sessions retain their existing final-Release
GAP policy. Palette state remains stored per surface; this change does not add a
palette resource wire identity or recover implicit destruction not observed via
an application final Release. Original Release reference-count semantics and
implicit attached-surface destruction remain unvalidated.

## Validation

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-resource-lifecycle.py working/build/render-ring
```

The independent PE32 COM fixture owns separate native pixels and padded borrowed
Lock rows, poisons those rows in original Unlock, and logically destroys the
object in original Release before the hook runs. Calls, reference counts,
HRESULT and LastError are checked. The real producer/queue/ring/decoder/GPU
consumer feeds independent complete RGBA hashes. A small optional archive checks
increasing identities, valid references, exactly one DELETE per lifetime and
simultaneous surface/pixel counts; the large pixel-churn case omits the archive.
No original assets are needed for these tests.

## Recorded result

[Final-source lifetime execution](opengl-continuous-resource-lifetime.json)
passes nine cases: four complete streams, five intentional refusals, and 63
independent complete-frame comparisons. The churn case publishes 434 commands,
creates 196 unique IDs, reaches 32 simultaneous surfaces, and retires every ID
exactly once. It repeatedly fills and reclaims the table, then recreates the
primary at alternating widths while preserving every complete frame.

The pixel-churn case publishes 100,666,308 bytes with 13 complete frames, recreating
2048x1024 resources through more than the simultaneous pixel and snapshot budgets.
Only this synthetic fixture waits for acknowledged DELETE outside callbacks
before the next large resource; production hooks retain their nonblocking queue
and existing overflow refusal. Alias lifetimes create 41 IDs with 20 actual
application QI calls and no observer references; nonfinal Release keeps UPDATE
on the existing ID, final alias Release deletes it, and reused alias addresses
start independent fresh IDs. 160 untracked releases leave the primary intact.

Every valid stream acknowledges its complete publication and ends with zero
consumer surfaces, zero ordinary native/RGBA readbacks and zero viewport uploads.
Held-lock, held-DC, contended final-release, conflicting-alias and bounded-policy
cases refuse and release consumer resources. HRESULT, LastError and original
Lock/Unlock/QI/Release/GetDC counts match the independent engine. The initial
fixture used a draw-interface GUID and ambiguous case prefixes; corrected final
fixtures use the surface GUID and exact case matching. A fast large-frame attempt
hit the unchanged queue cap before fixture acknowledgement pacing was added.
Those incomplete attempts remain under `working/tests/render-resources/`; only
the final-source execution above is registered.

[Fresh continuity regression](opengl-continuous-resource-continuity-regression.json)
passes all eight cases and 352 complete-frame comparisons on the resource-increment
sources, including bounded optional archive exhaustion and retained simultaneous
resource-capacity refusal. Earlier continuous evidence retains its old hashes.

The unchanged bounded owned-session regression passes all ten mixed, failed,
alias, capacity, restore, held-work, file-failure, byte/record-limit and indexed
cases (`working/tests/render-owned-session/run-3tl10jea/report.json`). Its first
invocation selected a renderer CMake cache rather than the required Qt-shell
cache and stopped before fixtures; the corrected invocation built the Qt shell
and command tool, then passed. This legacy report lacks source fingerprints and
is retained as a regression observation rather than new equivalence evidence.
Production and selftest PE32 builds pass. No original media was consumed.

## Subsequent complete-update increment

[Continuous mixed resource mutations](opengl-continuous-mutations.md) admits
layout replacement specifically from successful complete writable Unlock.
Descriptor-only, partial-base, Restore and missed-operation uncertainty still
refuse. This does not change the final-Release identity contract above; its
original execution fingerprints remain historical, with fresh regressions
registered under new IDs in the mutation increment.
