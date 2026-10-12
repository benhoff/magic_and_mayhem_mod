# Diagnostic cursor mutex retirement model

This native policy is a caller-serialized pure state model. It performs no OS
calls, synchronization, pointer reads or original patches. The owning module
keeps its private original/duplicate handles; another module must use versioned
scalar metadata and opaque numeric tokens, never this host layout or handles.

Publication requires empty storage, distinct nonnull original/duplicate values
and a nonzero identity. A reserve checks the current original and identity,
rejects retired/closing cells and guards counter overflow. Each accepted reserve
adds one lease; a matching return removes one. Retirement checks identity and
blocks further reserves. It keeps the duplicate while leases remain.

After retirement and all returns, close-begin claims exactly one close attempt.
The caller drops its synchronization before the OS Close call. Close-end checks
the claimed duplicate/identity; failure retains its duplicate and retired state
for explicit retry, while success clears the original/duplicate/identity. New
publication cannot reuse the storage before that confirmed success. This model
neither decides what a failed original Close means nor releases mutex recursion;
those are separate adapter/OS/gate responsibilities.

The prospective fixture covers lease counts0..32, wrong identities/handles,
retired admission refusal, close deferral, duplicate close claims, failed close
retention/retry, reused original value with new identity, invalid publication and
counter overflow and independent outer/inner retirement. Token identity uniqueness and caller serialization remain
caller obligations, with concurrency and live original lifecycle unimplemented.
No source alias, complete reference, native parity or safe live synchronization
is inferred from these state transitions.

A host-thread fixture also crosses8workers×1000reserve/return attempts with
retirement under an explicit caller mutex; one guaranteed pre-retirement lease
proves close deferral and drains before final close. This tests the supplied
caller serialization, not unsynchronized model safety or original game ordering.

`draw02-cursor-mutex-lifetime-fixture-01.json` passes the declared transition
matrix and eight host threads with current source/contract hashes. Confidence
is high for this serialized model; actual original/OS integration remains open.
