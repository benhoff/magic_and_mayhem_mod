# Bounded diagnostic cursor-source mutex gate

This is a proposed native diagnostic policy, not a recovered original contract.
It is isolated from the live capture path. The original timer uses an outer10ms
wait followed by an unchecked inner infinite wait; this policy instead attempts
both waits with timeout0, in outer/inner order, and refuses contention immediately.

Callers must pin two distinct live mutex handles and provide a zero-initialized
per-thread gate. No close/use protection, handle identity lookup or original
lifetime integration is implemented. Handles cannot be closed concurrently;
future integration must establish this separately and must preserve original
caller registers, result, LastError and floating-point state.

Successful acquisition owns one additional recursion level per mutex. Release
runs inner then outer and removes each acquired bit only on successful release.
Failed release sets failure6 and retains its bit; reacquisition while any bit
remains refuses with failure5. The caller must handle this explicit fault.
Null/equal handles or callbacks refuse with failure1, abandonment with failure2,
timeout with failure3, other wait failure with failure4. An abandoned wait grants
ownership: the gate rejects the potentially inconsistent state and releases that
newly acquired level, retaining failed releases. It never repairs original state.

Microsoft documents ownership on WAIT_ABANDONED and immediate zero-timeout checks
in [WaitForSingleObject](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject).
Recursive ownership needs one release per acquired level; failure when the caller
is not owner is documented in [ReleaseMutex](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-releasemutex).
These API rules support the native policy, not an inference about actual game
ownership, aliases, source storage or complete original locking behavior.

The prospective PE32 fixture tests real Wine kernel mutexes with synchronized
threads: uncontended pair, caller's retained recursive levels, competing outer
and inner owners, failed inner acquisition unwinding outer, and abandoned outer
and inner. Independent competitor probes check both mutexes. A64cell scripted
matrix crosses each outer/inner raw result0/80/102/FFFFFFFF with each release
success mask, checks timeout0 and reverse release order, retains failed-release
levels and forbids reacquisition until explicit successful release.

`draw02-cursor-source-gate-fixture-01.json` passes all64scripted cells and
seven groups of real Wine kernel mutex scenarios in an isolated PE32 process.
Confidence is high for that bounded fixture, provisional for any game application.
Passing owned fixture behavior does not establish safe live source capture, concurrent handle lifetime,
complete references, pixel parity, game semantics or original suppression.

## Duplicate-handle lifetime fixture increment

Before applying to original capture, an isolated PE32 fixture additionally tests
DuplicateHandle SAME_ACCESS2 with no inheritance and no CLOSE_SOURCE flag. An
invalid source must refuse without changing either real mutex's ownership. Two
private duplicates must have different handle values and share the original
mutex objects: recursion acquired/released through duplicates must preserve the
caller's earlier levels acquired through originals.

A synchronized worker owns the inner duplicate before both original handles
close. The main thread must still observe inner contention, unwind its outer
acquisition, and obtain both duplicates only after the worker releases. Competitor
probes test both mutexes independently. All private handles are closed afterward.
This is owned-fixture lifetime evidence, not concurrent original constructor/
close/use coordination or complete source ownership. Duplicate object references
intentionally extend lifetime and remain a native diagnostic policy.

Microsoft's [DuplicateHandle documentation](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-duplicatehandle)
identifies the same underlying object and documents reference-count retention
until both handles close. No original call is patched or game executed here.

`draw02-cursor-source-gate-fixture-02.json` passes all64scripted cells and nine
real scenario groups with current source/contract hashes. Fixture01 remains
historical after the extended test/runner/scope; no old hashes were refreshed.
