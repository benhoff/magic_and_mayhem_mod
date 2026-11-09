# Font lifecycle journal version 1

`font-lifetime-v1.bin` is an optional passive NoCD observation, not a shared
object layout. All words are little-endian DWORDs. The 64-byte header has magic
`MNMFLT01`, version 1, header size64, build40209ca7, byte bound16777216,
record bound65536, then nine reserved zero words.

Each record begins with 32 DWORDs: byte extent, ordinal, kind, owner thread,
tick. Kinds1/2 are function entry/normal return,3 is glyph-source use,4 terminal.
Word5 is the hook tag:0..7 alternating heading/body/tooltip/yellow setup and
destructor,8 raw load,9 byte draw,10 installation coordinator. Words6/7/8
are font ID, resource generation, role; word9 is the original return PC.

Entry word10 is the byte value for byte calls; word11 is flags; word13 font ID.
Load entry word14 is conversion count and its sole256-byte payload is a bounded
NUL-terminated path. Return word10 identifies the matching entry; word11 flags,
word12 the load result (zero for other tags). Load return words13..16 contain
new resource token, load count, success and prior-generation replacement.
Return words17/18 are font tracking and line height. Other fields are zero.

Glyph words10/13..16 are its active byte-call entry, byte, font-relative frame
offset, frame length and normalized FNV (palette pointer bytes28..31 zeroed).
Words19..22 are X/Y, current queue and producer sequence before the request.
Word23=1 explicitly records an unparented glyph; it earns no font binding.
Words24..31 are reserved zero. No journal value is a dereferenceable pointer.

The terminal record is128bytes: words5..10 contain loss, prior-record count,
prior-byte count, allocated token count, outstanding calls and current live
fonts. Remaining words are zero. Readers require exact terminal accounting;
zero live fonts counts current font objects, not every overwritten allocation.
