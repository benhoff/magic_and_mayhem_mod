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

The authorized temporary90GiB reserve then admitted the bounded live attempt
`live-menus/run-0rlcdmwr` / `menu-observer/run-lnrvc8wy`. Main/Quick/Create
ready states and the original Create OK callback are observed. Original TCP
room creation displayed **Could not create game session** before any host tick
or lobby refusal; the180-second host deadline expired.
[Failure record](host-lobby-create-failure-20261012.json) preserves pre-launch
source bindings, raw hashes and the dialog capture. UI45.host-create-failure is
negative diagnostic evidence; it binds no successful execution claims and
promotes no status. Original inputs were verified before/after the attempt.

Installed Wine11.16's32-bit TCP provider contains the diagnostic `session
creation is not yet supported`; [upstream Wine's Open callback](https://github.com/wine-mirror/wine/blob/master/dlls/dpwsockx/dpwsockx_main.c)
returns DPERR_UNSUPPORTED for create. This is consistent with the dialog, not
a captured HRESULT or loaded-module proof. No provider substitution or simulated
network success was used. Host ownership stays disabled by default until a real
provider allows the frozen host round trip. The permanent storage policy is
unchanged. `acab86fa..d7f73255` reviews30 exact committed file versions with no
missing receipts; this accounting does not assert retrospective validation.

The current implementation is **enabled by default** after UI46.host-lobby-live.
[Fresh live record](host-lobby-native-live-20261012.json) observes7 ready states,
9 original callback acknowledgements on one thread,2 TCP room creations/native
host visits and exactly2 original viewport handoffs. Cancel and window-close
return through Quick/Main to original Quit; original game and desktop both
exit0. Native game/player names, map label, roster and13 rules are projected;
unsupported controls remain disabled. No peer/guest/chat/rule/map/start/gameplay
or engine replacement claim is made. A transient absent roster before initial
host readiness is rejected; accepted Cancel's known transition retains native
presentation instead of triggering unsupported-state handoff.

The supplied Win95 DirectX6 archive is pinned at
`a9daa6ec591514337ccfa3df83e363bfa29010ffccf654c0e14ff55dabf98f81`.
`directplay_dx6_launch.py` extracts four pinned files without executing the
installer. A fresh or previously profile-owned Wine11.16 WOW64 prefix is
required. Wine initialization completes before **all three** expected builtin
hashes are verified; their original bytes are preserved before scripted atomic
replacement. Existing unrelated prefixes are refused. The normal host-enabled
launcher uses `working/tests/menu-host-dx6-wine`; tests use independent prefixes.
Native-only overrides forbid builtin fallback for dplay/dplayx/dpwsockx. Actual
DPLAYX/DPWSOCKX native loads occur on the original observed engine thread.
ASCII command-interpreter-safe paths/arguments and this pinned Wine build are
current profile boundaries; other platforms/versions remain unvalidated.

An exclusive prefix session lease protects the actual game launch.
[Wine Start's wait implementation](https://github.com/wine-mirror/wine/blob/master/programs/start/start.c)
waits on the game process and obtains its exit code. A generated fixed batch
captures that result on a separate line. Only **after** that result exists does
the launcher close the idle DirectPlay helper in the owned prefix; desktop exit
is checked independently. This addresses the persistent helper observed after
normal original Quit, without treating its desktop lifetime as the game's exit.
The private prefix fixture passes2 install/reuse cases and6 mutation-free
corrupt/foreign/mismatched/concurrent refusals;
[record](directplay-prefix-guards-20261012.json). It executes no DLL code or game.

Preserved failures are independent records with their source snapshots: full
native DLL staging without prefix installation failed object initialization;
provider-only staging loaded both modules but failed room creation; installation
refused the lazily uninitialized prefix before any binary write; real host visits
then exposed the persistent helper and an extra Cancel handoff. These negative
records do not bind successful execution claims. UI46.host-lobby-live is a new
prospectively bound result, not refreshed evidence from those attempts.
`d7f73255..799688cc` reviews6 exact committed file versions with no missing
receipts. No permanent storage policy change or cleanup occurred. Other menu
source fingerprints retain their historical/current-source limitations.
