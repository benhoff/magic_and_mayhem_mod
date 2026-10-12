# Diagnostic cursor gate lease accounting

Native caller-serialized model only, with64bounded cells and no OS calls.
Each token reserves a nonzero current-thread numeric owner and two distinct
nonzero mutex identities. Tokens advance monotonically within a run, never
reusing released values. Capacity exhaustion and impending counter wrap refuse.
A lookup requires the exact token and owner. Return additionally checks both
identities, then clears the cell. Wrong owner/identity, null/stale token and
invalid identities refuse without changing another cell.

The prospective fixture fills all64cells, rejects overflow, rejects foreign
owners/swapped identities, drains all tokens, proves double-return refusal,
reuses empty storage with a new token, rejects stale token, invalid inputs and
refuses wrapping after the final nonzero token. Caller locking is not supplied;
model use must be serialized. The paired lease helper reserves outer then inner lifetime cells; a failed
second reserve returns the first and clears its numeric token. A return checks
both identities, counters and zero outstanding acquired mutex levels before
draining the two reservations and token. Retired cells still permit returns,
so active leases can drain, but cannot admit new leases.

The additional fixture proves partial-reservation unwind, wrong owner/stale
token refusal, retained outstanding levels, close deferral during active lease,
retirement drainage, reused identity and invalid original pair refusal.

No actual duplicate handles,
mutex levels, versioned DLL exports, source protection or game integration exists.

`draw02-cursor-gate-leases-fixture-01.json` passes both current-source owned
fixtures. The acquired-level argument is trusted internal state: a future owning
adapter must store actual gate levels in its token; untrusted requesters cannot
assert zero. No OS/live synchronization is implemented by this accounting model.
