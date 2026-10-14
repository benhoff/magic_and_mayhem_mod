# Multiplayer setup menus (No-CD40209ca7)

Confirmed static discovery: Quick callback `4a83a0` actions0/1 request
Create `6de750` (ID15/finalvtable `5c660c`) and Join `6a5018`
(ID16/finalvtable `5c659c`). Constructors `4b0a90`/`4aec40` corroborate
those identities. Read-only exports are retained under
`working/decompiled/multiplayer-controls-awcjx_iw`, `-n_0m4m89` and `-x5n3bs5g`.
The manifests pin original bytes and verify all2927 immutable inputs before/after.
Confidence: high within the stated form scope; networking is not reconstructed.

Both entries set mode `68991c=1` and context `689920=2`, initialize controls,
use global username `6de715` (fallback `5c6160`) and initially select radio0.
Create entry `4b0c50` generates the original randomized game name. Join entry
`4aedd0` disables TCP radio1 when byte `6e2031` is nonzero. Transport IDs0/1/2
are IPX/SPX, TCP/IP and null modem, corroborated by the installed layout order
and provider selection in `4af9b0`/`4b1a30`. Automatic launch flag `6e1f0c`
can bypass normal forms; V15 excludes that caller.

| Packed field | Join | Create |
| --- | --- | --- |
| +4b | 2 TextButton pointers | 2 TextButton pointers |
| +53 | 3 RadioButton pointers | 2 EditBox pointers (game/user) |
| +57 | 2 callback pointers | 3 RadioButton pointers |
| +5b | username EditBox | 2 callback pointers |
| +5f/+63/+67/+6b | radio list/selected/count/capacity | same |
| +6f | deferred submit byte | deferred submit byte |

TextButton finalvtable is `5c6c6c`, RadioButton `5c6d40`, EditBox `5c6ca0`.
Callback objects have vtable/method/receiver DWORDs. Join type `5c65cc` uses
`4af910` for buttons and no-op `4af9a0` for Edit. Create type `5c663c` uses
`4b1840` for buttons and no-op `4b1920` for Edit. TextButton action IDs are
0OK/1Cancel. Edit `+45` is text; `+60` is max length. User limit is15;
Create game limit is256. Constructor `4cccc0` allocates256 bytes; setter
`4cd0e0` copies strlen+1 without checking capacity. Native game names are
therefore bounded to63 printable ASCII bytes, user names to15, with original
spaces preserved. These native transaction limits are policy, not equivalence
to all original text-input behavior. Existing standalone previews retain their
64-character/TCP default policy.

Join OK `4af910` focuses an empty username via `4caf00`, clears `+6f` and
returns0. A nonempty name calls fade `557510` and sets `+6f=1`; it does not
set returning `+43`. Cancel fades and sets `+43=1`. Create OK `4b1840` first
focuses an empty game name, then an empty username. Otherwise it fades and
sets **both** `+43=1` and `+6f=1`. Cancel sets returning only. Unknown action
IDs do nothing; both callbacks are thiscall, return0, RET4.
Original radio setter `4ce730(group,radio,playSound)` resets selection and
uses list capacity; V15 validates all three pointers/count/capacity before
calling it with playSound0. Original setters and callback are one transaction.
All guards run before mutation, including identity, owner stack, callback roles,
text pointers and availability, modal/deferred/fade/pending/return flags.

Original tick remains `5595d0`. Deferred Join `4afb00` clears `+6f`, calls
network helper `4af9b0` and on success requests session selection `6de838`;
failure reenables controls. Deferred Create `4b1c70` clears `+6f`, calls
`4b1a30` and requests lobby `6a4b1c` on success. Helpers select the original
provider, initialize the original network objects and show original error
messages. V15 invokes neither network helper directly nor substitutes a
session list. Submission hands presentation to the original viewport. A fresh,
same-thread, acknowledged ready form/Main/Quick return can restore Qt ownership.

V15 adds pointer-free snapshots and an atomic80-byte host name lane while
retaining every V14 lane. Normal admission requires Main/Quick/form stack depth4,
exact fixed objects/vtables, mode1/context2 and automatic launch flag0. Cancel,
Escape and window close unwind through original Quick and Main Quit. Native
form drafts survive idle snapshot refreshes; original text is reapplied on a
fresh admission. No original networking, renderer, audio or menu lifecycle work
is bypassed.

Validation is recorded separately below after execution. Live submission,
network error recovery, session enumeration/join, lobby rules/chat/start, late
campaign Character/Portmanteau/Realm/Grimoire callers and multiplayer gameplay
remain pending. Static destination links do not validate their network behavior.

Validation2026-10-12: `UI43.original-form-fixture` retains20 private original
callback/setter cases and50 transactional guard rejections (callback stack and
callee-saved registers checked). `UI43.forms-live` retains seven native ready
states/nine original acknowledgements in `working/tests/live-menus/run-xwe7tidy`,
experiment `working/experiments/menu-observer/run-2ts5zn_j`: three Create/Join
admissions, original defaults/masks, native draft retained across eight engine
refreshes, Cancel/Escape/window-close unwind and original normal Quit.14 related
Qt wire/lifecycle/widget checks pass. Original source verification passes before
and after each consuming experiment. Submit/network/session/lobby remain pending.
The two callback byte ranges are explicit inventory exceptions backed by the
private execution anchors; the static deferred Join slot remains an inventory
mapping gap. No binary inventory or old evidence hashes were refreshed.
