# Diagnostic source pending admission and held snapshot V2

These are native diagnostic wire formats, not recovered original file layouts.
All integers are little-endian uint32. A numeric canvas identity does not prove
separate backing storage or complete allocation ownership.

`MNMSPA01` has a 64-byte header and 64-byte rows, capacity128. It reuses
`MNMSPF01`'s source/lease/owner/tick/producer-sequence/queue/pending-count and row
slot/tag/owner/primitive/caller-RVA/object-canvas/record-kind/record-destination/
record-source/reserved fields. Header word14 is3 and word15 is1. Exactly one
current-thread tag7 source-wrapper row is required; other rows are foreign tags32/39
COPY records with known recorded endpoints different from the selected source.
Files use a monotonically increasing admission ordinal and a four-digit filename.
At most4096 files are admitted. Failed writes or exhaustion terminate the run.

Held snapshots keep magic `MNMSHB01`, 64-byte header and192-byte rows. Version1
retains its single-pending-frame rule and zero reserved words13..15. Version2 is
selected only with the pending-copy diagnostic. Row word13 is the admission
ordinal when source metadata word12 (pending frames) exceeds1; it is zero
otherwise. Words14..15 remain zero. All existing source/plane/lease/worker/error/
geometry/count/budget rules apply. The strict analyzer requires an exact
lease/source/owner/producer-sequence/queue/pending-count join for every reference,
and refuses missing, duplicate or extra admission files. No old journal is rewritten.

The additive PE32 stdcall export `SceneDrawingSourceAdmissionV1(const uint32*
point, uint32 size)` has an eight-byte argument stack. It accepts only a readable
64-byte point exactly equal to the last admitted copy's cached metadata, from the
active source lease owner/source. It returns the admission ordinal or0 and
preserves LastError. It does not sample original state or establish a new frame.

Existing source refusal records keep their layout. Code11 in `MNMSHR01` now
identifies a missing exact pending-copy admission point. It admits no source.
Independent checked page protection, source identity, paired source bytes and
worker completion remain required alongside the metadata. Physical display,
complete routes, unobserved uploads and original lifetime ownership are separate.

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
