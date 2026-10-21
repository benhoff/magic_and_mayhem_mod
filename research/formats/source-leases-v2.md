# Explicit external source lease journals

Profile2 (MNM_DRAW_WINE_SOURCE_OWNERSHIP=2) preserves the record sizes and pinned
NoCD build tag of the existing diagnostic journals. All fields are little-endian
DWORDs. Process-local callback objects are described in backing_lease_v2.h;
no pointers from those objects enter the journals. Profile1 retains its existing
wire versions and private-only ownership admission.

* MNMSHF03, version3, 64-byte header/256-byte records: MNMSHF02 fields0..30
  remain. Record DWORD31 is storage mode: 0 private, 1 provider-v1 binding
  transfer, 2 provider-v2 original-binding read lease. All later words are zero.
  Mode2 requires kind3..6, exact guarantees7, stable provider/object/generation/
  token and completed producer fence. Phases1..7 retain order; phase6 verifies
  the unchanged original binding in mode2. It does not claim a restoration.
* MNMPOW03, version3, 64-byte header/128-byte records: owner kinds3..6 mean
  shared/file/imported/GPU provider read leases. Phase10 binds this owner to a
  submission before acquisition. Words0..7 are ordinal,phase,thread,owner,
  submission,providerID,objectGeneration,zero. Words8..15 are objectLow,
  objectHigh,token,requiredFenceLow,requiredFenceHigh,completedFenceLow,
  completedFenceHigh,guarantees7. Words16..31 are zero. Phase2 acquires, phase3
  follows exact worker/context/fence completion, phase4 retires metadata after
  handoff verification. These owners never use VirtualProtect/VirtualFree as a
  claim of ownership over external allocation aliases. Terminal records require
  all external owners retired.
* MNMWMS05, version5, 64-byte header/256-byte records: MNMWMS04 bounded headers
  and performance-counter fields39..42 remain. Word43=1 means direct provider
  readback; words44..63 are zero. It requires success, READ/map/unmap/error words
  zero, exactly two getter/descriptor calls, valid row/slice pitch, ordinary
  bounded RGB565 geometry, a sequential accepted snapshot attempt, and an owned
  comparison payload with CRC. CPU-mappable descriptor access is unnecessary
  only for this mode. Other attempts retain normal public READ map semantics.
* MNMSOW02, version2, 64-byte header/64-byte records: header word7=2 selects
  external lease policy, remaining reserved words zero. Record word6 counts
  direct provider handoffs; all later words zero. Ready counts are zero. Closed
  COPY count equals total handoffs, and direct count equals the independently
  decoded handoff count. All ten strict capabilities remain required.

Join the provider identity, object/generation/token/fence and storage mode across
physical owners, handoffs and snapshots; join the independent reservation and
GPU completion journals to the same submission/command/worker/thread. Physical
and reservation record ordinals are different domains. Do not infer private
value ownership from a valid producer lease. Do not infer global host/device
alias ownership or hardware acceptance from these finite records.
