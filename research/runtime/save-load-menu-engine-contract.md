# Save/Load menu integration

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred image VAs for that build; allocated controls are checked
each time, never assumed stable between launches.

The next live-menu increments, in expected everyday use order, are Save/Load,
campaign Realm navigation, campaign Spellbox/Grimoire/Character, remaining
result/report screens, then multiplayer setup/discovery/lobby. This is a work
order, not measured usage telemetry or a completion claim. Main, Quick Battle,
single-player setup/Map, pre-battle Spellbox, fresh Region Entry, campaign Mini
Cancel/Quit and selected Preferences already have their separately recorded
integration evidence. Each added screen needs its own original control,
ownership, return and live validation evidence.

## Recovered controls

The read-only exporter retains instruction ranges, vtable candidates and byte
anchors. The private PE32 fixture executes original bytes with save/load I/O,
fade and modal-resource dependencies stubbed. Confidence is high for executed
branches; screen admission, file durability, complete load failures and live
returns remain pending. See [serializer boundaries](persistence-progression.md).

| Screen | Receiver / vtable | Contract |
| --- | --- | --- |
| Load (11) | `0x006de4e8` / `0x005c64c4` | List `+0x5b`, edit `+0x4f`, caller `+0x60`, context `+0x65`; ordinary shared tick at vtable `+0x10` |
| Save (13) | `0x006de7c8` / `0x005c64fc` | List `+0x57`, edit `+0x4f`, overwrite admission DWORD `+0x5f`; ordinary shared tick at vtable `+0x10` |

Load callback `0x004aa800` takes one stack index with ECX receiver: 0 disables
the list and sets bytes `+0x5f/+0x64`; 1 calls fade, sets return DWORD `+0x43`,
and clears `0x00657d37` only for caller 3. Other indices do nothing. Deferred
update `0x004aa5d0` consumes `+0x64` once, passes the edit text to the loader,
then sets original success/return flags. The fixture exercises the successful
branch and no-pending branch; failure modal behavior is only statically inspected.

Save callback `0x004ab750`, local index 1, fades and sets `+0x43`. Its index 0
path calls `0x004ab870` (disk-space check and deferred request); index 2 deletes
through original `DeleteFileA` and list maintenance. Save update `0x004abd00`
skips leading ASCII spaces, calls writer `0x004ea5a0` with the edit text in ECX
and overwrite admission in EDX, then clears nonzero admission. Result 1 fades
and returns; result 2 creates the original overwrite modal and sets admission 1.
The original confirmation and remaining error/delete branches must be retained
and separately tested before live activation. Result 1 is not durable-write proof.

Original list rows are `0x101` bytes: 256-byte text plus a selection byte.
Count is `+0x49`, row pointer `+0x45`, selected index `+0x5b`. Selection setter
`0x004d1060` can toggle an already-selected row; a semantic selection first calls
it with -1, then the wanted index. Getter `0x004d1df0` returns the selected row.
Edit setter `0x004cd0e0` copies the complete NUL-terminated string to the buffer
at `+0x45` without a length guard. Any adapter must validate the destination and
bound the full name before calling it. Native row IDs must refer to the current
original snapshot rather than Qt-side filesystem enumeration or display order.

Mini callback `0x004b23f0` uses campaign indices 0 Load and 1 Save. Load records
caller 17, and both set the pending screen and Mini return flag. Main Load uses
its original callback index 1 and caller 3. Their initialized stack/return
contracts still need bounded live observation; these static stores do not admit
loaded campaigns or arbitrary parents.

## Reproduction

```sh
python3 tools/working-retention.py run --reserve-gib 1 -- python3 tools/test-save-load-menu-contract.py
python3 tools/working-retention.py run --reserve-gib 0.1 -- python3 tools/export-save-load-menus.py
```

Both tools verify the original manifest before and after input consumption and
reject unsupported executable hashes. The fixture creates a private executable
and Wine prefix under `working/`; it never runs the game, creates saves, changes
installed executable bytes or promotes live replacement. Historical hashes and
menu evidence remain unchanged.
