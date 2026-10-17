# Exact primitive / API / worker identity

Implemented and exercised in an isolated checkout, 2026-10-17.

The source-lifetime trace identified pending cursor wrappers, but its time overlap
join deliberately did not assign an API to one wrapper. This additive opt-in
query snapshots the innermost active wrapper on the calling thread under the
scene recorder mutex. It exports local call/parent IDs, canonical request fields,
canvas identity/extent and QPC, without addresses or pixel bytes.

The drawing observer invokes it outside its journal mutex at surface API entry,
original return and observer completion. The decoder checks exact lifecycle,
request, thread, body interval and the complete observed surface-call denominator.
Existing explicit API-to-Wine submission and command-to-worker IDs extend this
chain without choosing a parent by time overlap. Unbound routes remain unbound.

This supplies identity evidence only. It neither leases cursor GPU resources nor
proves their CPU/GPU aliases, unknown writers, input semantics, global presentation
or native replacement. The complete first-120 gate and all 47/21 requirements
remain in force. No prior acquisition is relabeled with these new observations.


The first identity acquisition (original120-01) retains 335052 primitive calls and
5523 surface API calls. Of 2707 explicit consumer chains, 2194 bind while the
primitive recorder is active; the other 513 explicitly report that it has stopped.
All 36 pending cursor COPY wrappers have exact API and worker chains. None is
assigned by temporal proximity. The capture ends with normal original-child
Quit/exit0 and unchanged 2927-file original manifests; launcher exit0 remains
unwitnessed. Diagnostic journaling changes timing.

An independent join checks 408 original EGL regions, all matching at the declared
RGB tolerance of one. First-120 accounting retains 120 World queues, 1984 inputs
and 311 primary/source-worker-EGL witnesses. There are 20 cross-prefix sets and
16 late original-API acknowledgements in this acquisition; historical counts
remain in their original reports. The unchanged full interval gate rejects all
four ownership requirements. The six-case actual PE32 recorder fixture and
parser controls pass. These findings locate the missing source ownership proof;
they do not supply immutable COPY backing or all alias/writer coverage.


A second acquisition after the menu startup-clock change is the current-source
[original capture](native-completion-identity-original120-20261017.json). Its exact
counts and raw journal identities are retained in that report and the independent
[ownership](native-completion-ownership-20261017.json),
[EGL](native-completion-egl-20261017.json) and
[first120 accounting](native-completion-accounting-20261017.json) results.
The first acquisition remains retained with its original menu-source fingerprints;
those two hashes became historical after the startup fix. No old hashes were
refreshed. The current full [admission](native-completion-interval-gate-20261017.json)
still rejects all four ownership gates.


The renewed capture records394451primitive calls,5486surface APIs and2712consumer
chains. All35pending cursor wrappers (13opaque,22keyed) have explicit consumers.
All556unbound chains report the recorder-stopped state at every sampled phase;
they are outside the active primitive scope. The407selected EGL regions match
within the declared tolerance. First120accounting retains120World queues,
1974inputs and306primary/source-worker-EGL witnesses. All17cross-prefix API
calls returned before their completion-associated update began; source-byte
ownership remains unproved. These counts describe the renewed run only.
