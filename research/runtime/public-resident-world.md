# Public resident World integration

Native policy work in progress,2026-10-11. The public command stream must import
an independently completed V4 World before post-World CPU HUD updates and copies
reach the primary. This is separate from recovered original renderer equivalence.

The proposed MNMCMD04 opcode19 carries destination ID,u64 generation and nonce,
never pixels or host pointers. It accepts only800x600RGB565 live destinations and
strict consecutive generations with immutable nonce. Missing provider,stale
reference,unsupported format or extent terminates the consumer. A shared GPU
lease import inverts the exact RGB565 bit replication on the GPU; diagnostics
may read expected native bytes independently. Runtime staging,CPU baseline
quarantine,HUD ordering,menu transitions and live performance remain pending.

The first isolated fixture executes all65,536 RGB565 values through sharing
contexts and compares native words independently. It also verifies ordered
800x600 World/HUD/copy composition, wrong format/extent/identity/version and
missing-provider refusal, complete validity after invalidation and resource
cleanup. Report:`public-resident-import-fixture-20261011.json`. Public producer,
CPU baseline separation and live gameplay remain pending. The existing command
consumer fixture also passed. This software-GL result asserts no TITAN FPS.

## Public adapter and newly reached branches

The opt-in public shell path (`--live-menus --native-commands --resident-world`)
now pairs one owned World queue with its shared GPU lease. An80-byte acknowledgement
contains no World pixels. The scene adapter restores the original page guard,
then binds the current-thread full800x600RGB565 Lock to an ordered opcode19.
The native owner verifies the actual original closing records and imports before
releasing the slot. Missing identities, bad acknowledgements and transport loss
stop rendering and input. This path does not recover from stale CPU World words.

The original simulation and CPU UI drawing continue. Post-World opcode20 carries
only changed UI WORDs with exact binary coverage, so a dirty rectangle cannot
upload unchanged stale World words. Destination-dependent CPU UI operations and
writes equal to the old CPU baseline remain explicit equivalence gaps. DirectDraw
is disabled by the existing owned graphics staging; the public test checks that
its module is absent. This does not remove the legacy graphics interface vocabulary.

Public execution exposed additional pinned branches:

* Raw WORD596cb8 returns at500845 in campaign traversal and57e8fb in the
  reviewed wrapper. The original zeroes EAX at59707d; only the scoped AX0
  branch is admitted. Descriptor/backend mismatches retain refusal.
* Additive545c10 retains original54ab00/546540 wrappers, gates and object
  side effects. Only leaf bodies are suppressed. Caller bytes at54ab16 and
  54657c are pinned; selected low16 AX is0. Ordered native rectangles use the
  existing low16-wrap/unsigned-saturation GPU implementation.
* The observed protected write at547edb reached colour leaf547d80 through
  wrapper54ab60, returning at54ab72. Its native V3 operation26 owns tight
  RGB565 pixels, modes0..3 and bounded1..64 extents. Optional original source
  planes are read and normalized privately; pointers never cross the channel.
  Canonical31/63/31 channels are mandatory. V1/V2 refuse operation26.
* Rotating the public camera reached minimap49/45 paths outside the original
  orientation-zero capture scope. Public V3 prefixes now select the existing
  owned four-orientation minimap overlay operation24 and terrain sentinel.
  Historical unmarked V3 terrain retains orientation-zero admission.
* Background secondary bitmap/copy work overlaps the guarded World. Its owned
  bytes are deferred under the emit lock until the next prefix, with64record/
  4MiB bounds. Global bind, World destination and World copy source remain
  prohibited. Closing source bytes stay immutable. Frame slots use atomic
  claims and oracle identities are assigned only on actual publication.

The additive hook initially omitted RET8 on its suppressed path. A synthetic
fixture reproduces the bad stack cleanup with exit96, then passes against the
corrected production trampoline. Both additive and colour entries are now
compared against a callee-pop control, including8saved register words and512B
x87/SSE state. This fixture proves the trampoline ABI, not primitive admission
or independent original pixel equivalence.

## Validation boundaries and current failure

`tools/test-public-resident-fixtures.py` declares source/input hashes and scoped
claims before execution. It checks exhaustive GPU import, masked HUD/copy,
retained public request replay, mixed ordered sprites/additive/four-colour-mode
rectangles, malformed payloads, deferred ownership/refusal limits and leaf ABI.
The native reference comparison is independent of original destination pixels;
it does not constitute a new original comparison.

The retained Titan campaign negative `public-resident-campaign-stall-20261011.json`
reaches71completed Worlds (the periodically published report stops at64), with
zero World/native/RGBA delivery readbacks. The inspected native image shows the
World, portrait and HUD. Physical camera/rotation/selection stress then stalls
before Mini readiness; the bounded smoke deadline fails. Early native publication
windows reach53..62FPS and painted windows35..41FPS, then fall below19 and to0.
These transient figures are not a sustained performance pass. Public casting,
combat, pause/resume, quit and sustained19FPS remain pending. Historical kind8
and minimap original comparisons retain their recorded hashes; this wiring does
not promote them to current public replacement or pixel equivalence.

## Default offscreen storage and batch profile milestone

The first rejected descriptor is108bytes, flags7,510x180 and offscreen caps0x800
with no explicit memory placement or format. Native policy selects owned
system-memory RGB565 (returned caps0x840). Explicit0x1007 requests still require
exact565 masks; unsupported caps and RGB555 remain refused. The first rejection
is retained as156local bytes without changing HRESULT or GetLastError.
`native-default-offscreen-pass-20261011.json` passes64unchanged Wine cases,
37,440pixel words and2,048caller words, separately from4native default-placement
cases/367,200words,10refusals and bounded512surface cleanup.

Resident World now discards CPU validity once at entry rather than replaying
each sprite's CPU validity work. All prefixes, native draws and shared-frame
publication run under one bounded GL context batch. Unknown CPU reads still
refuse and native GPU history stays resident. Fresh owned headless fixtures are
`public-resident-batch-fixtures-20261011.json`:1,440,000reference words, mixed
primitives, strict admission, rotated minimaps and leaf ABI; no original or
public-gameplay equivalence is inferred.

Titan profiling after batching measures24.41ms native work,11.02ms producer
work,3.03ms prepare and6.38ms submit per World, versus35.48ms before batching.
The aggregate11.22FPS remains a failure. The run reaches195verified Worlds
then refuses a concurrent primary-screen40x40 cursor save copy. The captured
source isID1, destinationID5, while the immutable World entry isID2; this is
secondary cursor traffic, not a World source read. Capacity exhaustion is the
remaining admission reason (record or byte budget); exact counters were absent
in that historical diagnostic. Additional bound counters and a tested512record
limit are pending fresh execution; the4MiB budget and World-source/destination
prohibitions remain. The bounded smoke
progress report records active/frame/stress state. The legacy fallback flag
denotes terminal suspension on this resident route; it does not present the
original viewport. See `public-resident-batch-background-copy-failure-20261011.json`.
Performance, casting/combat, Mini/Cancel, semantic quit and current original
World/UI pixel equivalence remain pending. Historical hashes are preserved.

The follow-up512record/4MiB queue preserves every World-source/destination
refusal and passes nine malformed/bounds cases plus a512record primary cursor
copy burst. Fresh full owned fixtures are retained in
`public-resident-cursor-queue-fixtures-20261011.json`. The Titan follow-up passes
1100resident Worlds without a deferred refusal but stops at smoke Escape stage4
(Mini does not appear) and misses19FPS; negative report
`public-resident-cursor-queue-live-failure-20261011.json`. No status promotion.

## Guarded owner permission-query epoch

`NR.world-source-region-epoch` is an intentional public-resident policy. The
original World owner caches at most64committed readable region answers after
installing the NOACCESS guard. Both guard boundaries discard the cache; other
threads and nonresident routes still query freshly. Headers,caller bytes,palette
and frame bytes are copied/checked per request; byte identities are never
trusted by pointer. Unknown,guarded,uncommitted and overflowing region answers
do not become proofs. This differs from `readable_extent.h`'s original fresh
synchronous admission policy: external permission changes during the guarded
World epoch are excluded. Original permission/pixel equivalence is pending.

The actual-query counter remains the existing wire field; the shell reports its
per-World average. Fresh fixtures in
`public-resident-region-epoch-fixtures-20261011.json` prove10000queries collapse
to2,64region capacity/eviction,epoch permission revalidation and the existing
owned reference/ABI/strict-defer corpus. The prelaunch undeclared-test dependency
refusal is preserved separately; subsequent declarations name both policies.

The default tutorial negative reaches~26actual queries/World and~18FPS with
zero delivery readbacks, a large improvement over thousands of physical queries
and~12FPS. This still fails19FPS and does not reachMini. The earlier inspected
native image shows a SummonZombie tutorial popup; native physical Escape
generation was forwarded but the tutorial route is not an ordinary campaign
pause/resume proof. The physical Escape helper now holds250ms instead of80ms
to span slower engine polls; this changes test stimulus,not input semantics.
Ordinary difficulty1 casting/combat,spell/portrait,Mini/quit and sustained19FPS
proof is running separately.60FPS and full original equivalence remain pending.

The ordinary difficulty1 Titan run reaches225verified Worlds at19.7667aggregate
FPS before a projected-shadow bounds refusal. Actual memory queries average29.82
per World, with zero World/native/RGBA delivery readbacks. This is negative
evidence (`public-resident-region-epoch-shadow-failure-20261011.json`), not a
sustained19FPS or completed casting/combat/portrait/quit proof. The next bounded
fix investigates the right-edge shadow against actual opaque samples.

## Projected shadow transparent-padding admission

The ordinary campaign gen226 request1115 has68x38source geometry,origin(-18,-23),
anchor(700,179),clip(0,0,800,312). Its nominal projected right limit is805, but
decoded opaque samples reach only786. The old rectangular preflight therefore
refused safe coverage. Native preflight now checks actual projected samples only
when the bounding rectangle exceeds storage; covered overflow still refuses
before destination mutation. GPU cropping removes only transparent padding at
the owned canvas boundary. The original unprojected horizontal refusal and
vertical/alternating-row rules remain. No pixel readback or destination seed is
introduced. `GlBlitter::dimensions` reads owned metadata without resolving or
fencing a presentation. Direct ResourceCache draws also check covered storage.

The scene regression uses this geometry with analytically placed opaque samples,
checks exact darkened pixels, odd vertical cropping, whole horizontal refusal,
actual covered overflow preserving the previous frame, and undefined-surface
metadata without readback. This is native admission evidence; independent
original execution equivalence and completed public gameplay remain separate.

Two fixture setup refusals are preserved under the shadow fixture path/recipe
reports: incorrect CMake executable location, then a standalone SPR declared
as an effect without its required ANI. Neither is gameplay or original
equivalence evidence. Corrected execution uses standalone UI resource recipes
and runs both atlas and separate-surface shadow backends.

The first complete shadow execution passed both backends and the upload/reference
corpus, but executable hash reporting raised an internal type error afterward.
Its report is retained separately as `public-resident-shadow-fixture-report-error-20261011.json`;
this incomplete result is not promoted to a successful fixture report.

Fresh complete `public-resident-shadow-fixtures-20261011.json` passes both
shadow backends, atomic genuine overflow refusal, cropped rows/whole horizontal
refusal, owned metadata without readback and existing1.44Mword/import/defer/ABI
fixtures. Sources/inputs are unchanged and all executed native fixture binaries
are hashed. This is native integration evidence, not original pixel equivalence
or a public gameplay/performance pass.

The campaign prelaunch compiler check formerly scanned all built test targets;
this correctly refused undeclared test inputs but incorrectly treated them as
shell dependencies. It now follows the shell's linked objects/local archives
and requires their exact compiler receipts.287actual repository source/header
dependencies are found on this build. Synthetic linked-target fixtures prove
unrelated/generated inputs are excluded and missing linked object/archive
receipts refuse. New durable result `public-resident-linked-provenance-fixtures-20261011.json`
is source bound; the earlier negative is preserved separately. New validation
metadata does not promote engine gameplay or original equivalence.
