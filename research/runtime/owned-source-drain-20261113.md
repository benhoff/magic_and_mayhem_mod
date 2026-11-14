# Independent native menu source draining, 2026-11-13

Two player sessions end at exactly 131,072 producer records. Their V3 source
headers declare that same cap, and both live channels use V5 with source chunks
disabled. Terminal reason1 records identify the `emit_raw` call path, at relocated
MnmScene RVA `0x21346`. The first has no completed World; the second has 2,814
balanced completed/imported/returned Worlds and no outstanding source credit.
Both then record glyph refusal20 with capture disabled and original exit124.
`emit_raw` closes the producer at the record cap; closing also closes glyph
admission, so a later text call triggers that terminal native-only refusal.
These are bounded bridge lifetime failures, not an original simulation limit.

The older synchronous V6 path already drains menu/source history between Worlds.
The public independent path now uses [V7](../formats/world-continuous-v7.md),
combining V5 owned World receipts with V6 apply-before-ACK source chunks. All
buffer sizes, thresholds, whole-World limits, guarded original return checks and
native pixel ownership remain. The original engine/AI/clock/balance are unchanged.

A single capture worker orders source chunks and Worlds. A chunk may wait beside
at most one unimported World; the worker stops acquiring further input until
that chunk has been applied and acknowledged. The native renderer consumes it
through its existing producer instance after every prior World import. Only
successful application of the matching global ordinal permits ACK/slot reuse.
No source chunk grants World completion/import/return credit. Stop joins without
acknowledging unapplied work. Cancellation errors now report actual accepted,
returned, imported, outstanding and pending-source state instead of asserting
that outstanding work exists for every cancellation.

Focused controls retain the existing V5 FIFO/two-credit/actual-return and malformed
input suite. A new queue case drains 278,528 source records before World1, checks
that ACK waits for application, rejects stale application identities and prevents
a source chunk after World1 from passing its import. Stopping a source wait joins
without ACK. Existing source-history controls apply 17 chunks/278,528 records on
software and M1 and compare the next World with independent CPU pixels. Both V6
and V7 source leases validate malformed/stale/nonce/CRC/immutable/cancelled input;
World ACK selection remains version-specific. V3/V4 channel and512V4 ACK controls
remain successful. The PE32 scene producer and actual Qt shell compile.

Exact user failure channels/DLLs/terminal records, tested source/binary identities,
commands and control results are retained in `working/owned-source-drain-20261113/`
SQLite. Initial broad test-launch configuration also retains an unrelated ARM
compile refusal in an x86-only source-region fixture; the public game target
builds successfully. An interrupted live dwell accepted original Quit/exit0 after
six chunks/98,304 records; its incomplete menu test remains incomplete. Neither
that interruption nor a bounded headless case alone claims full live acceptance.
A fresh requested dwell is recorded separately.

Asynchronous V5 and finite capture without source chunks retain their previous
bounds. Arbitrary resource churn/maps, original pixel equivalence, live battle
performance and every shutdown race remain separate. Historical proofs and hashes
are preserved with explicit pending applicability after these source changes.


The requested fresh120-second dwell passes with original exit0 and native-only
state `completed`. It drains196608records in12chunks during World generation1,
without completing a World or accumulating World credits. Main→Quick→Cancel→Main
callbacks and normal Quit complete; renderer errors, fallback and outstanding
source ownership are zero. The actual M1native owner applies all chunks. This is
a live menu/source-buffer witness, not a new live battle, original pixel-oracle
comparison or universal stability/performance claim. Evidence
`NATIVE13.owned-source-drain01` binds that scope separately from the earlier
interrupted test and ARM-only build setup refusal.
