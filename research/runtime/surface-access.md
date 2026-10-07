# Standalone Surface2 lock/DC admission

## Scope and evidence

This experiment exercises real `IDirectDrawSurface2` objects in Wine 11.16,
normal cooperative mode, at 8×6 pixels. The 96 cases cover 16 operation sequences,
indexed8/RGB565/RGB32 and requested system-memory (`0x840`) or default offscreen
(`0x40`) caps. It executes no original game instructions. Driver observations
are confirmed only within this build and matrix; the signed state explanation
below is a model of outputs, not a recovered Wine implementation. The existing
original bitmap loader `0x486a80` is linked as a GetDC/ReleaseDC caller anchor
only, as documented in [the original sentinel findings](original-surface-sentinel.md).
This capture does not validate that caller or recover its lock scheduling.

The standalone PE32 probe initializes all 48 native words with `(index*17+3)`
truncated to the format, then runs recorded Lock/Unlock/GetDC/ReleaseDC inputs.
Every Lock receives a 108-byte descriptor prefilled with `0xabababab`; every
GetDC receives an output sentinel. Pointers are normalized to unchanged/null/new
nonnull classes. A separate owned surface supplies a valid foreign DC. Release
arguments include retained, null and foreign handles; repeated releases also
exercise stale retained handles. No invented invalid pointer is dereferenced.

Capture, offline and native execution reports are retained under
`research/runtime/` with the `surface-access`/`native-surface-access` stems.
The compressed offline corpus is `tests/fixtures/surfaces/surface-access.json.gz`.
Wine reservation prevents overlapping sessions, uses a disposable prefix and
stops only that prefix. Original manifests are checked before and after.
A sandbox socket failure and an early probe that required all terminal locks to
succeed produced no registered evidence; their logs remain under
`working/tests/surface-access/`. A later retry refused an active game session.

## Validation result

The offline and native models both match 96 cases / 1,182 transitions, 8,664
stable descriptor fields and 4,032 readable final words. The 12 terminal busy
cases are the two over-unlock schedules across six format/caps combinations;
84 unstable failed-Lock caps words are retained but excluded. Eleven offline
mutation tests and ten native context/budget guard checks pass. Historical
bitmap-raster snapshot reports remain unchanged because their sources did not
change.

## Confirmed observations

A successful full Lock prevents another Lock, returning `0x887601ae` on the
repeat. GetDC nevertheless succeeds with a Lock held. GetDC prevents repeated
GetDC (`0x8876026c`), preserving the output sentinel on failure. Lock with a
normally held DC returns busy. Unlock without a mapping returns `0x88760248`;
ReleaseDC without an active DC returns `0x8876024a`.

ReleaseDC while active rejects null and foreign DC arguments with `0x8876086c`,
preserving the active lease. Without an active DC, `0x8876024a` takes precedence
over handle validation. Both Lock→GetDC→ReleaseDC→Unlock and
Lock→GetDC→Unlock→ReleaseDC recover. Handle validation and error precedence are build scoped.

A busy Lock clears the meaningful descriptor: size remains 108, all compared
fields become zero, including the surface pointer. The trailing caps word is
not stable on this error path: retain it in evidence but exclude it from
comparison. Successful descriptors are compared in full after pointer
normalization. Default offscreen objects report driver caps distinct from the
requested caps; those values are scoped to this environment.

Excess Unlock calls while a DC is held can cause later Unlock calls to keep
succeeding and later Locks to remain busy, even after ReleaseDC. Terminal busy
cases have no readable final frame; the corpus stores `null`, never invented
pixels. Readable terminal cases preserve all initialized bytes.

## Offline model and native policy

The input-derived model uses a signed mapping balance and a separate active-DC
bit. Successful Lock and GetDC each add one; Unlock subtracts one whenever the
balance is nonzero; successful ReleaseDC subtracts one and clears the DC bit. Null/foreign release
failures retain both values.
Repeated Lock fails when already mapped, and repeated GetDC fails while its DC
bit remains set. Releasing a DC after extra Unlock calls can leave negative
balance, explaining the recorded persistent busy/extra-success results. This
model matches outputs; it does not prove the driver's internal counter type.

`renderer/surface_access.hpp` owns the same bounded single-thread admission
state. Its descriptor and DC values are opaque 32-bit tokens, never host
pointers or real HDCs. It explicitly represents negative mapping debt instead
of allowing arithmetic wraparound. Invalid dimensions/formats/caps/tokens and
balances outside ±64 refuse before the failing mutation. The instance cannot
be copied or moved. Native busy descriptors supply owned caps; this intentional
value is excluded from driver equivalence. Packed pitch and default caps are
bounded storage/environment policies.

The native replay owns initialized CPU words and compares all recorded HRESULTs,
normalized outputs, stable descriptor fields and readable terminal words. It
does not connect this admission state to GlBlitter or BitmapDcState, create a
real DC, or permit drawing through a borrowed pixel pointer. Native conservative
Lock refusal with an active but over-unlocked DC is an unvalidated policy outside
the recorded schedules; acquiring a new DC after negative balance is also
outside this matrix. Such branches need new evidence before integration.

## Remaining boundaries

Blt/BltFast/fill with Lock/DC source or destination ownership, association with
owned bitmap raster/storage, real token/alias lifetime, partial rectangle locks,
other flags and descriptor sizes, invalid argument pointers, primary/video
presentation, cross-thread races, actual loss/Restore, Windows-driver behavior,
original game callers and live/wire replacement remain pending. Driver findings,
native admission and native raster snapshots are separate milestones.

## Reproduction

```sh
# A capture writes new evidence; do not overwrite retained reports/corpora.
xvfb-run -a python3 tools/capture-surface-access.py --report working/tests/new-access.json --corpus working/tests/new-access.json.gz
python3 tools/check-surface-access.py
python3 tests/test-surface-access.py
python3 tools/check-native-surface-access.py --report working/tests/new-native-access.json
```
