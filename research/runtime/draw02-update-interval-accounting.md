# Accepted updates, World draw branches and observed call intervals

Pinned No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is opt-in reference acquisition. Original drawing, simulation, clock and
gameplay policy remain active. Native replay and semantic equivalence are separate.

The historical combined120 capture has a queue-free accepted96 with advancing
counter95→96 and active-screen samples World `006cbb78`, Results `006deec8`, then
World. The latter is the documented Quick Battle Results singleton, ID26.
V1 samples suppression0 before selected maintenance/animation/scheduler calls;
they do not establish the later comparison result or target availability. Do not
retroactively assign a precise skipped branch to that recording.

Static control flow before World production compares `[006e1f00]` at0046bb9d
and gameplay byte+100b2 at0046bba5. Either unequal comparison branches to0046bbdf,
past the scene call0046bbd3. In the default scene path004fd536 tests the acquired
target; zero branches to004fd5ad without the selected queue004fd554. The alternate
path uses004fd4ab. Late Results push0046bc14 follows the World drawing gates;
an earlier Results push0046b22e exits before accepted-update storage and belongs
to a different invocation. These are instruction-level findings; deeper flag,
target allocator and multiplayer/alternate-screen semantics remain unresolved.

`--update-routes` requires cadence and original-active API observation. Five
additional exact-byte guards precede all patches. Cadence V2 retains96-byte
records, adding events9(suppression comparison),10(transition comparison),
18(scene tail entry),20(late Results tail entry),21(default target comparison).
Words15..19 retain original flags, transition byte, pending Results byte, full
mode word and target-available boolean. Events9/10/21 name their exact site in
word13; tail calls retain the original return PC. Comparison and branch thunks
replay the original instructions and preserve flags/registers/x87/SSE/LastError.
V1 traces remain unchanged and cannot provide this new branch evidence.

Drawing `update-call-timing.bin` uses magicMNMUTIM1 and a64-byte header:
version1, row size64, build40209ca7, row limit786433, QPC frequency low/high,
then reserved zeros. All admitted calls have entry1, original return2 and
observer completion3 records. Row words0..12 are ordinal, phase, API entry,
thread, original record type/method, QPC low/high, result, LastError, first input
word, completed original journal record and physical plane ordinal. Remaining
words are zero. Phase4 closes with reason, final journal record, pending0,
plane count, QPC and drops/gaps0. Result/input and completion fields are absent
before their observation. Concurrent threads may append timestamps out of global
order; per-call phases/time and exact session identity are checked instead.

With whole-source capture, `source-update-windows-0000.bin` uses MNMDWIN1,
64-byte header/version1/row64/build40209ca7/limit512/reserved0. Each accepted
callback, under the producer recorder lock, records ordinal, phase1, cadence
record, accepted sequence, counter-before, mode, owner, closed producer prefix,
completed-or-entered queue count, failure count and pending primitive-frame count.
Remaining words are zero. The terminal phase4 retains owner, final prefix,
queue and failures. The closure records all source prefixes including queue-free
updates; a prefix is observed callback state, not atomic original allocation
ownership. Pending frames and external writers remain explicit limitations.

The QPC frequencies of drawing and cadence must match. Associate all overlapping
original call/observer intervals, retained inputs, source prefixes and selected
API/worker/EGL witnesses with half-open accepted windows. Calls that straddle a
boundary belong to every overlapping interval and must remain visible. An input
poll is observation of a consumed API result, not proof of selection, tooltip or
pause success. QPC alone does not prove causality or exclusive frame ownership.

Complete global route coverage, backing aliases/direct writers, original source
atomicity, hardware scanout and native parity remain independent open gates.
