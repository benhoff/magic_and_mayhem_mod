# Diagnostic pending-copy source acquisition

This opt-in step-2 policy preserves the original renderer and adds no original
mutex waits. The default pending-frame refusal is unchanged. Different canvas
IDs do not prove disjoint storage. `--source-pending-copy` requires the existing
read-only source lease, exact worker-command completion, paired held snapshots,
producer capture and wrapper storage transitions. This is diagnostic capture,
with an explicit permission/timing effect; it is not unobtrusive gameplay.

A selected source may have exactly one current-thread nonprimitive tag7 frame.
An additional frame must be a foreign-thread nonprimitive, nonreserved tags32/39
(`58c360` in the pinned hook table), with a readable 96-byte owned COPY record,
known live source/destination IDs and source-wrapper association. Neither endpoint
may be the captured wrapper/ID. Every other pending tag, missing/unknown record,
primitive, duplicate selected submission or ambiguous/lifetime state refuses.

Source lease acquisition checks the predicate while owning recorder metadata,
then checks allocation/protection and applies read-only protection. Byte export
admits the extra frames only while the same owner/source/object/pixels lease is
active and the whole lease extent still has committed read-only pages. Checked
lease release revalidates identity, dimensions, interfaces and protection before
restoring exact original permissions. Aliased writes to protected pages retain
the existing fault record and terminate the diagnostic run, even if IDs differ.
Unknown backend aliases, complete original lifetimes and all routes remain open.

Every admitted multi-frame byte export retains a bounded `MNMSPA01` file, using
the refusal row layout with header reason3/diagnostic flag1. The strict decoder
checks the exact policy shape and retains `disjoint_backing_proved=false` and
`source_ownership_proved=false`. A metadata file alone is insufficient: accepted
capture also requires the independent paired page lease, held source snapshots,
exact worker outcomes and no fault/rejection. Missing/failed metadata writes and
a 4096 admission budget exhaustion terminate rather than silently truncate.

Held snapshot V2 is selected only by the explicit pending-copy diagnostic. V1
keeps its original single-frame rule. V2 row word13 binds a multi-frame snapshot
to the exact preceding admission ordinal, obtained through an additive stdcall
`SceneDrawingSourceAdmissionV1(point64, size64)` export. The query reads only the
cached admission point and preserves LastError; it does not poll original state.
The analyzer independently checks every referenced admission's lease/source/
thread/producer sequence/queue/pending count and refuses missing, duplicate or
extra files. Missing exact points retain rejection code11. Accepted ordinary V2
snapshots keep a zero admission word.

Validation is pending. The owned PE32 fixture exercises disabled/unleased
refusals, admission, 16 malformed/pending states, lost protection, changed source
identity, exact restore and an actual foreign-thread write through an aliased
allocation. Fresh original-active execution is separate from these owned pages.
No global acquisition, 47/21 acceptance, native replay/parity, live replacement
or scanout claim follows from this increment.

Admission journal version1 remains strict for tag32 only. Version2 adds the
observed foreign tag39 (`58ca90`) COPY shape, with the same protected-page and
known-endpoint requirements. Other COPY tags remain refusals. Original attempt01
retains six admitted tag32 snapshots, then an exact foreign tag39 refusal at
source lease130/queue27. It is a negative incomplete capture, not a pass or a
lifetime/overlap proof. Its raw data and hashes are preserved.

Owned fixture suites can explicitly use `--shared-fixture-prefix` with a newly
cloned private suite prefix. A root/role marker, exclusive file lock and bounded
idle-server check reject game prefixes and concurrent use. Fixture processes run
serially, retain separate binaries/journals and terminate their own Wine server.
Historical prefix copies are preserved; the default still creates a private copy.

Corrected-contract owned suites02 pass all16 pending guards and8 held cases,
including real foreign-thread aliased write refusal and exact point/export joins.
The current original120-update retry is blocked by the unchanged100GiB floor;
original execution, complete routes and frame/caller/lifetime closure remain
pending. No DRAW-02 or step2 completion is asserted.
