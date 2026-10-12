# Multiplayer lobby engine contracts

Seeded pinned No-CD40209ca7 full host6a4b18 / primary menu6a4b1c,
constructor4b8a50/final primary vtable5c689c. Guest full6c4778 / primary6c477c,
constructor4b83d0. Base constructor4b6330 sets original menu ID24.
The separate full/primary this pointers must not be conflated. Rules, player,
map, chat, action and return contracts/native integration are pending.

Confirmed static layout: host primary table5c689c, guest5c67f8, original screen24;
full-object network interfaces5c68cc/5c6828. Menu virtual methods receive primary
object+4. Callback objects are **sixteen bytes**, not the twelve-byte form type:
vtable/method/adjustor0/full-object receiver. Base type5c67f0 Cancel4b7bf0 uses
full receiver, fades primary+4, requests Quick at full+3b (primary+37), and invokes
original interface at full+8f (primary+8b), slot+44 disconnect. Unknown actions
are no-op. Callback returns0/RET4; raw private execution checks ABI and both null
and nonnull service branches with fade/disconnect stubbed.

Base primary fields: callback array+4b (five), chat history+4f, chat edit+53,
Cancel button-pointer array+57, forty headings+5b, portrait/colour buttons+5f.
Base chat callback4b7bc0 and player callbacks receive full object; host callback
type5c6938 also uses16bytes/full receiver (rule4b9c30, Start/Map4b9dd0,
remove4b9e50). Host sliders+0xa8 contain13 extended0x86-byte controls; their
setter/float/step/network contracts are not yet recovered. Original lobby tick
5595d0 retains deferred4b7b80 message processing; no engine work is replaced.

The proposed V17 **one-local-player host view/Cancel** copies original global
rules/roster/game name and map label, requires normal Create/Main/Quick depth5,
mode1/context2, exact full/primary identities, current service/full-owner links,
Create caller15 and local slot0. Peer appearance or unsupported state hands
presentation back to original controls. Native rules/chat/map/start/player
controls are disabled in this bounded view; original controls remain available.
The view is disabled by default pending live validation; the dedicated harness
explicitly opts in. Guest/network/rule/chat/map/start/gameplay remain pending.

UI45.original-lobby-fixture records7 callback/adapter cases and35 rejection
checks. Native host wire/widget checks pass. Original input verification succeeds
before/after the private fixture and each static export. The live preflight
refused6GiB estimated output with102.5306GiB free under the required100GiB
reserve; `host-lobby-storage-preflight-20261012.json` records **not launched**.
This is storage provenance, not live execution evidence. No cleanup, policy
change, live validation or integration/replacement promotion occurred.
