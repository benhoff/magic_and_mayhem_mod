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
