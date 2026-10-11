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
