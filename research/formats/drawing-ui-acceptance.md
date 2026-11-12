# Original UI acceptance observations

Optional diagnostic files for the pinned NoCD executable
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
These are observation journals, not a shared-memory ABI or native render input.
No source pointers are dereferenced by a host consumer. Numeric addresses name
only the observed original-process session; they are not allocation generations.

## UI records

`MNMUIA01`, uint32 version and uint32 record size512 form a16-byte header.
Version3 is the acceptance wire. Earlier versions remain readable with explicit
unavailable fields: v1 word43 observes maximum mana, not current mana; v2
word10 observes the accepted render sequence, not the pause mode. Neither old
wire can prove the original pause byte. Records are128little-endian words;
the maximum8192-row boundary is an overflow refusal, not a complete capture.

| Words | Version3 meaning |
| --- | --- |
| 0–5 | Ordinal, kind, callback ID, GetTickCount, thread, completed World-return ordinal |
| 6–7 | Wizard maximum mana; accepted render sequence from World+0xfd62 |
| 10–11 | World pause **byte** at+0xfd5a; simulation counter at0x6c4830 |
| 12–18 | Menu screen/generation/request/status/top/depth; separate input mode at0x6903e8 |
| 20–27 | Tooltip visible/pending, font object/raw/palette, backing object, width/height |
| 28–35 | Tooltip source and destination rectangles |
| 37–44 | Checked wizard base/slot/type/owner/active/health/current mana/raw selection |
| 47–55 | Text byte count, original result/LastError, validity, health-widget object/cached value/maximum/actor, observed argument |
| 56–63 | Existing32-byte completed source-frame identity, including nonce and owner |
| 64–127 | NUL-terminated, zero-padded256-byte original tooltip text in CP1252 |

Kinds1/4 observe World return/Mini dwell. Kinds2/3 pair original tooltip text
entry/return;5/6 pair original health update entry/return. Callbacks run once.
Validity bits1/2/4/8/16/32 identify tooltip fields, checked wizard role, completed
frame, terminated text, readable health widget and verified original World
object. Unknown bits, torn records, clock/owner/generation regressions, missing
callback returns and populated reserved fields refuse terminal admission.

Ordinary campaign teardown has two additional World returns after the last
sealed frame: top6 (Defeat) then top4 (Realm). The offline assessor verifies
their exact owner, ordered return ordinals, frozen accepted render sequence
and last-frame identity, and retains each raw record hash separately. These
returns supply no World, actor-state, tooltip or pixel credit. The live reader
is used before Quit; it must not label stale terminal identity as a new frame.

The original tooltip object is0x657fe8; its visible flag is+4, pending+0x17a,
font+8, source rectangle+0x15a and destination rectangle+0x16a. The observed
English portrait formatter consumes actor health+0xe4, maximum mana+0xec and
current fixed-point mana+0xe8 shifted right8. These fields remain distinct.

## Actual key polls

`MNMUIK01` has a64-byte header: version1, record size48, count, capacity64 and
ten zero words. Twelve-word records contain ordinal, GetTickCount, thread,
VK_SPACE32 or VK_ESCAPE27, original API return, native-input ownership flag,
accepted input sequence, original return PC and four zero words. Only down/up
transitions are recorded; repeated held polls are omitted. A full64-row journal
is refused. Space must end with an observed release; Escape can cease polling
when Mini takes ownership, so its missing release poll remains explicit and
must be paired with actual inactive/released native input and normal retirement.

The optional observer is disabled unless the UI journal path is supplied. It
never changes key results, LastError, input storage, original pause state or
timeouts. Default Right-key acceptance retains its existing separate wire.

The completed campaign pilot retains no optional Space/Escape API journal.
It supplies no API-poll or recovered original key-message-handler credit.
Qt's current input forwarder sends X key events as well as publishing key state;
the acceptance assessor instead binds actual physical Space press/release to
the observed original pause-byte transitions and stable/advancing counters.
The exact original Mini ready/Cancel/resume callbacks and inactive input/media
owners supply the Mini contract. The optional API journal has fixture coverage
only unless an actual stream is retained and separately admitted.
