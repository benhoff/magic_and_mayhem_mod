# Static rendering review input pins — 2026-10-25

The unchanged web UI reader rejects an older static implementation review when
its contract identity still matches but documents linked later are absent from
its pinned input list. This metadata inconsistency also exists without the
pending native-player launcher work. It is separate from engine validation.

New immutable derived review records pin those newly linked inputs as **pending
source review**. Every prior source fingerprint, contract fingerprint, witness,
classification and recorded implementation level is retained. The prior active
record is preserved in implementation-review history. No execution evidence,
original address mapping or status dimension is promoted by this repair.

The UI therefore displays the affected review as historical with explicit
pending inputs. The previously verified software/NVIDIA rendering cohorts and
finite original-upload diagnosis keep their original fingerprints and limits.
