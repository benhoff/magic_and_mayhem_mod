# Owned initial-history diagnostics and COPY refusal metadata

These are native adapter policies, not recovered original allocation guarantees.

`owned-initial-history-0000.bin` uses little-endian DWORDs: a16-byte header with
magic `MNMINIT1`, version1 and row count1..512; each16-byte row contains canvas ID,
width,height and policy marker0x4e495a31. IDs are nonzero and unique; dimensions
are1..2048. Rows witness freshly owned RGB565 zero allocations. The matching V3
source operation is an existing FILL with zero value, full extent and word22
marker0x4e495a31; it is not an observed original FILL call. Only fully written
rows enter the published count, and the file is written at producer shutdown.

Public resident native-owned COPY observations may use word23 marker0x4e435231
only with exact word21 HRESULT0x88760096 (native rectangle refusal). The native
facade rejects that operation before pixel writes. The marked record retains the
failed call and replays without mutating pixels or advancing canvas dirty state.
Unmarked COPY records retain their ordinary bounds/history checks. Within the marked form, unknown
markers, other HRESULTs and malformed marker/status pairs refuse; V1/V2 refuse
this extension. Earlier consumers reject the formerly reserved nonzero word23
before replay, so mixed versions fail closed. Unmarked failure classes remain
outside this scoped extension. It does not assert general original-driver failure semantics.

A retained display-transition witness has source81 at640x480, requested source
rectangle(371,556)..(411,596), return0x88760096 and caller0x4a4c26. Earlier replay
ignored this result and attempted the invalid copy. Native policy accounting
keeps this observation separate from original-driver equivalence.
