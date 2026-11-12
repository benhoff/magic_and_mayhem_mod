# Passive palette custody v1

Opt-in `MNM_PALETTE_OBSERVE=1` augments the closed producer with a separate
`palette-observation-0001.bin`. It observes all ten guarded original builder
entries/normal returns and the selected `5979b0` free wrapper. It does not
invoke builders, change their arguments, write palette memory or admit callers.
The fixed selected executable is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Entry signatures are checked before installing any palette hook. Assembly
preserves registers, flags, x87/SSE state and original calling conventions;
observers preserve LastError. Exceptional unwinds remain a missing normal return.

Little-endian DWORDs: a 64-byte `MNMPAL01` header contains version1, size64,
build `40209ca7`, 128MiB byte bound, 4096 allocation and 65536 selection slots,
then zero reserves. Every record starts with 32 DWORDs. Words0–6 contain total
bytes, ordinal, kind, owning engine thread, tick, World queue and preceding
producer sequence. Word30 is payload CRC32C; word31 is zero. Kinds are builder
entry1/return2, tracked-free entry3/return4, selected-state change5 and terminal6.
Only one thread may produce observations. Loss is permanent and disqualifies
complete admission; observation failures do not change original call arguments.

Builder entry: word9 count, word10 RGB555 flag, word13 mode0–9, word14 original
caller. Payload is 768 source RGB bytes and 80 global bytes from `5f18d0`.
Return repeats these fields, adds word8 allocation ID, word11 node-byte count
and word15 original EAX. Successful payload owns next/shift/neutral and all
produced table bytes, followed by 768 post-call source bytes. Counts are bounded
powers of two2–256; other original domains explicitly cause observation loss.
WORD modes contain count−1 tables; DWORD modes8/9 contain count/2. Four bytes
of unused allocation padding are excluded. IDs increase per successful builder
return, including when the allocator reuses an address. Private pointer keys
never cross the wire. Mode-specific profile file/API inputs still require
separate binding; a table snapshot alone does not prove their recovery.

Tracked-free records name allocation ID in word8, tag10/caller in words13/14;
return also preserves EAX without interpreting the free wrapper's undefined
return value. Freed IDs become inadmissible for subsequent selections.

Selection records: words8/9 selected/root allocation IDs, word10 chain ordinal,
word11 table index, word12 effective WORD-table CRC32C, word13 backend,
word14 caller, word15 pointer-normalized frame CRC32C; words16–23 reproduce
producer words8–15 (XY, clip rectangle, operation, backend), word24 signed shade,
word25 frame bytes, word26 destination canvas, word27 chain length and words28/29
shift/neutral. Payload owns all allocation IDs traversed through the ordinal.
All selections are checked, but unchanged frame-address/selected-ID/ordinal/
table/CRC choices are deduplicated. A later selection row therefore reports an
observed change, not every raster call or a cross-launch frame identity. Join
to retained complete requests/output before claiming a visible effect.

Terminal word7 loss, word8 allocation count, word9 preceding bytes, word10 all
checked bindings, word11 still-live original allocations and word12 outstanding
calls. No outstanding call/loss is admitted. Still-live allocations are explicit;
normal process exit is separate from an observed original free. This format
does not independently establish actual cycling, original profile semantics,
allocation completeness outside its admitted domain, native output or retirement.
