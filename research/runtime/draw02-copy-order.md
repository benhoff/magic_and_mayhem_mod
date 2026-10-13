# Bounded copy-order mismatch

Evidence: `draw02-source-physical-close-copy-order-01.json`, original120
source-close capture and its completed failed native comparison. Confidence:
high for recorded geometry, checkpoint hashes, unique local associations and
worker ordering; provisional for a general cause or correct ordering repair.

Of141eligible source/physical contexts,140match. API3768/EGL430at producer
prefix208533differs in1600pixels, rectangle440310..480350, in both native
source and display. Original CPU checkpoint1334(sequence208514)and1335
(sequence208520)have the same hash as the held source. Native1334matches;
native1335changes after background-copy replay.

The local producer return stream lists background save1→5at208516before
full copy2→1at208517. Unique local API/rectangle/source-destination matching
links those to API3757(thread424)and3756(thread384), respectively. Original
full-copy commands1037/1038finish before background-save command1039enters:
last full worker return record3115, first background worker entry3116. Thus
actual driver order is full→save while producer return order is save→full.

This establishes a missing ordering boundary in a return-only input model.
It does not alone prove every CPU/GPU view, alias or the full cause of the
40x40difference. Next observe explicit request/entry/return/API/worker identity
for copy operations and retain all missing/ambiguous links before a general
source-only replay correction. Do not swap records by hypothesis, discard the
failed context, use original raster as input or refresh older evidence hashes.


## Explicit request observation

`draw02-copy-order-fixture-02.json` passes five owned PE32 cases, including
actual two-thread reversed returns. `draw02-copy-order-original32-02.json`
retains913paired request IDs with actual gameexit0.
`draw02-copy-order-producer-links-01.json` checks every descriptor and emitted
COPYsequence against the unchanged producer stream:913matches,0unmatched,
45adjacent entry pairs with reversed return order. Four decoder and eight
source-cursor frontend guards pass. These observations establish entry and
return identities only; API request/worker binding, driver order, CPU/GPU view
ownership and the safe source-only replay repair remain pending.


## Strict API and worker associations

`draw02-copy-api-worker-bindings-01.json` associates all913requests with913
successful APIcalls and1130completed worker commands. Require same owner,
geometry, opaque/keyed flags, logical source/destination/extent and all three
API point prefixes within the explicit request interval. Unique completed
submission/packet/worker identities remain separate from these source metadata
checks. All1075additional stopped-source APIentries are retained outside the
association. Six guards cover missing phases, descriptor/owner/key/interval
changes, ambiguous/duplicate associations and absent/changed/shared commands.

Forty-two of45adjacent return reversals execute in entry order at the worker;
three execute in return order (requests692/693,837/838and841/842). Thus neither
producer entry nor producer return order supplies a general replay ordering
contract. This is static analysis of the frozen original capture, with no new
game execution, renderer correction or semantic/physical ownership promotion.
