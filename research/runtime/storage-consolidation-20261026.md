# Storage consolidation, 2026-10-26

This is host storage work. No original game artifacts were modified or executed;
no engine comparison, integration or replacement claim changes.

Confirmed: duplicated review history, full SQLite snapshots and expanded archive
catalogs were recurring costs. New review caches index imported receipt identities
and preserve their own bodies/drafts. Configured immutable report writers share
one pending store. Chunked snapshots retain exact raw SQLite bytes across two
filesystems. Legacy catalog bytes remain in two verified compressed histories.
The controls and final maintenance receipt are SQLite-backed reports under
storage-consolidation-results-20261026/.

Retention operation records are pinned under
working/retention/storage-consolidation-20261026/, with independent cold receipts
and payloads under /var/local/magic-and-mayhem-archive/. Each operation records
its actual source identity and restore result. Historical evidence hashes and
failure records remain unchanged. Filesystem free-space changes include other
writers; reclaimed allocated bytes and archive costs are reported separately.

The current pass holds recent, active, pinned, modified and hardlinked inputs.
The fresh environment review found only a small unpinned remainder after previous
cleanup. Broad old environment pins still protect substantial storage. Pending
stores with hardlinks need a full alias/consumer review; old timestamps alone do
not justify replacing them. Current-source controls do not retroactively claim
that earlier producer code ran unchanged.

See [storage policy and recovery](../../docs/storage-consolidation.md).
