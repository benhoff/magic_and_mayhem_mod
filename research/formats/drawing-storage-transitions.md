# Canvas storage transition diagnostics

Native observer wire contract; not an original game file format. Addresses stay
inside the process. Recorded identities represent numerical observations, not
allocation ownership. See `../runtime/draw02-canvas-storage-generations.md` for
execution scope, evidence and unresolved boundaries.

`canvas-storage-transitions-0000.bin` starts with64bytes: magicMNMCST02,
version1/2/3, row bytes96, build40209ca7, maximum131072rows (terminal included),
maximum512address identities, then zero words. All fields are little-endian u32.

Lifecycle rows contain24words: ordinal, phase1entry/2return, invocation, hook tag
0create/1release/2Lock, thread, caller RVA or0unknown, producer prefix, queue,
failures, cached canvas generation, cached address identity, width, height,
WORD stride, flags1readable-wrapper/2known-cache, interface+4address identity,
interface+8address identity, wrapper width, height, byte pitch, observed Lock
return epoch, returned Lock address identity, relocated caller flag, reserved0.
Create/release return registers are unspecified and omitted. Every observed
Lock return advances the epoch, including null returns. Version1 requires word22
zero; version2/3allow1only for the exact relocated Lock caller RVA18d329.

Version3query rows: ordinal, phase3, query ordinal, route1map/2client, epoch,
cached address identity, status0unknown-point/1matching-epoch/2no-epoch/
3changed-address, reserved0, then sixteen words of version2portable storage
metadata. Its word15equals the query ordinal. The query and epoch observation
share the producer recorder critical section. Missing/mismatched epochs are
explicit; the recorded point may remain a known cache association without
proving storage ownership.

Terminal rows: ordinal, phase4, calls, returns, open calls, producer prefix,
queue, failures, observed address count, observed Lock-return epochs,
version3query count (older versions0), then zero words. Captures require no open
calls. Budgets/write failures cannot silently truncate into completed evidence.

Version2portable points retain the64byte version1scalar layout, replace the
version field with2and use word15for query ordinal. Zero ordinal is permitted
when inactive/stopped; a live unstopped export requires an active journal.
Legacy version1exports keep zero reserved word15. The observer chooses version2
only with explicit storage-transition capture, preserving legacy operation.

A numerical address can recur across epochs or logical canvas generations.
The journal does not infer physical allocation reuse versus shared backing,
real COM refcounts/aliases, all calls/writers, uploads or complete lifetime.
