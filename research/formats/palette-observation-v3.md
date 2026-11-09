# Passive palette custody v3: original in-place World refresh

V3 uses `MNMPAL03`, version3. V1/V2 remain distinct immutable formats; their
strict selection-vs-builder content check still refuses an unobserved mutation.
The actual v2 default800 capture exposed World entry's already recovered
57daa0 →584430/589280 rewrite of second/fourth WORD/DWORD blue palette nodes.
V2 captured every initial builder correctly but did not observe these writes.

Two additional guarded six-byte original prologues are intercepted. Both are
fastcall (ECX count, EDX existing node), seven stack DWORDs and RET28. The stack
arguments are RGB source plus three binary64 tint coefficients. The original
routine runs unchanged once. Entry/return register, flags, LastError, x87 and
SSE preservation use the same production assembly and explicit RET28 controls.
Only already tracked live mode1/mode9 allocations with matching counts2..256
are admitted; unknown, mismatched or retired refresh targets permanently fail.

Kind7 is refresh entry, kind8 its normal return. Header word7 is the existing
node's revision (return increments on nonzero EAX); words8/9/10/11 are allocation
ID/count/format/node bytes, word13 is hook tag12 or13, word14 caller and word15
return EAX. Words16–19 retain actual pre-observer floating context. Kind7 payload
is768 RGB bytes,80 lighting-global bytes,24 coefficient argument bytes, then
the complete before-node. Kind8 payload is the complete after-node plus768 source
RGB bytes. Next-node pointer bytes are zeroed in snapshots: chains are separately
represented by allocation IDs. The source parser checks the full before-node
against its current owned version, records the new version, and refuses changed
headers, failed-call mutations and any unobserved intervening write.

Kind5 word7 now names the selected revision; payload consists of allocation ID /
revision DWORD pairs for every selected chain link. A revision change emits a
new selection even when its CRC is unchanged. Geometry, callsite, source sequence,
frame/table CRC and all existing bounds/owner/retirement rules are retained.
No private pointer or assumed across-launch address crosses this wire.

A refresh proves actual runtime production only when its directly captured RGB,
globals, coefficients and floating context independently reproduce the complete
after-node and caller result. Initial construction is independently reproduced
as well; before/after original tables are comparison outputs, never native model
inputs. Native output additionally requires the exact selected new revision,
owned source packet/order, complete original/native comparison and visible
palette index sample. Initial unpresented constructor bytes are not a prior
native frame. This finite World-entry refresh proof makes no arbitrary periodic
animation, exceptional floating-point or full Windows platform claim.

Header word8 is either0 (compact changed-state logging) or4: explicit complete
selection events for local queue tags1..4, with compact logging elsewhere. The latter
requires `MNM_PALETTE_FULL_PREFIX=4`; other values refuse setup. It bounds direct
per-draw selection/delivery/order joins after correlation with the
complete owned source packets without expanding every unchanged later draw. The terminal still counts
all checked selections; no complete later per-draw journal is inferred.

The actual native continuous path resets its local queue and source sequence
for each owned World packet (`continuous_reset`), so tag1 recurs for successive
global generations. Zero marks outside-World/menu/retirement context; it is not
a backwards World generation. V3's low-tag full logging consequently records
every selected World draw in these captures. Correlate its complete ordered
selection list with the complete ordered owned source packets to establish the
real global generation. Never label header word5 itself a global generation,
sort records, or accept a missing/duplicate request. The strict reader retains
positive local-tag regression refusals and permits outside-World zero records.
The original strict-reader failure and old format document source are retained.
