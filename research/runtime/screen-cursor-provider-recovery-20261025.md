# Original cursor background service recovery

The two cursor-background helpers named by the screen-service frontier now have
complete local original/native comparisons. Their existing immediate and
deferred copy connections also execute through actual native surface providers.
Original screen control remains a retained supplier; no new screen-tick or
native helper hook is installed. This closes two local contracts and their
bounded provider connection,not every external service under the screen tick.

## Original behavior and portable implementation

`screen_cursor.hpp` models borrowed NoCD state. Invalidation4a4370..4a437b clears
only the whole saved-valid DWORD. It allocates and releases nothing.

Restore4a4380..4a44ae first gates cursor enable,class22,destination interface
6a49e4 and the sign bits of all four saved coordinates. It waits infinitely on
642c58 and ignores the return value. It rereads saved validity and coordinates
after that callback. Width/height subtraction wraps as DWORD arithmetic;zero
extents skip copying,while negative differences are not repaired into positive
sizes. Saved offsets are clamped at zero. Receiver641fc8 copies into6a49e0 via
58c4a0,or publishes a copied rectangle through58d5b0(type2,priority1). The mutex
handle is reread after copy callbacks and released. Restore does not itself clear
the saved-valid flag.6a49e4 is the restore destination wrapper's interface;that
same canvas is a source in the later screen presentation copy.

All55296 admitted original32,sanitized32 and independent native64 cases agree
in complete selected state and ordered callback arguments/visible state. They
include every early gate,zero/reversed/extreme extents,negative/large offsets,
nonboolean saved-valid words,four wait results and controlled wait/copy callback
mutations. Original helper bodies remain unchanged. Platform endpoints are
authored in this arithmetic/control comparison. CPU register return values,
invalid pointers,arbitrary concurrency and callee fault unwinding are excluded.

## Actual native provider connections

A separate PE32 control executes unchanged4a4370/4a4380,58d5b0 enqueue,
58d660 drain,58c4a0 copy and original graphics initialization/shutdown against
Wine DirectDraw,the native facade and guarded native mode3. Real mutex calls
are forwarded through observed IAT endpoints. No selected original instruction
is changed. Each backend runs78 owned13x9RGB565 cases:

- Early gates,invalidated/absent saved history,zero extents and clamped offsets.
- Immediate output versus deferred unchanged-before/drained-after output.
- Saturated128-record queue:the restore attempt preserves every queued word;
  existing work drains without invented capacity,discard or retry policy.
- Independent Python checks of both complete destination images,including
  unchanged pixels. All18252 destination words per backend match;the three raw
  provider streams are byte-identical.
- Eight forged/truncated/duplicate/reordered trace controls refuse. Native and guarded shutdown retire
  every device,surface,DC,clipper,palette and pixel byte before process exit;
  neither native provider loads real DirectDraw.

`CSR1.helpers06-20261025` records original/model comparison.
`CSR1.providers06-20261025` records headless real-provider integration. The
[immutable execution](screen-cursor-provider-results-20261025/execution06.json)
retains prospective contracts,2801 source fingerprints,compiler dependency
checks,original function hashes,commands and results. Sources stay stable and
original-manifest bookends pass. The private prefix is exclusively leased and
its Wine server retires. Raw binaries,provider streams,logs and the admitted
prefix remain pinned under the owned worktree's`working/tests/`.

## Remaining boundary

The portable helpers are recovered baseline models. The public native profile
continues using original screen control with native pixel/history services.
This result establishes the bounded retained-original provider connection;it
does not establish a newly installed native helper,original screen allocator or
all virtual targets. Required screen selection/control/special services,
foreground cursor resource lifetime,unknown writers,alternate ownership,
arbitrary GPU/raw readers,failure reachability and full journey/physical-driver
acceptance remain separate. Both helper integration/replacement statuses remain
none;their implementation/comparison are scoped/recorded. The provider policy
has scoped implementation/headless integration and comparison/replacement none.

Reproduce with `tools/test-screen-cursor-services.py --headless`,a fresh owned
output/report path,prospective claims for both helpers and the provider policy,
and an idle marked fixture prefix. Run through the main storage guard and a
private temporary/display namespace. The preparation and final adoption records
retain the exact source overlay used while the reviewed copy merge completed;
final accounting uses the actual integration commit as its Git base.

The first helper/provider execution remains immutable under its original IDs;
execution02 repeats the same matrix after newer master source reconciliation.
The current live copy campaign also passes:4630 complete independently checked
COPY destinations,1931CPU-keyed calls,78GPU calls and187balanced Worlds through
normal Quit. The current save/load attempt fails at the Save-menu transition
with resident producer cancellation after92balanced Worlds,before a saved-file
digest. Its stable-source failure and raw outputs remain retained. It grants no
storage acceptance or new original checkpoint comparison. Existing historic
storage passes keep their hashes;this newly observed merged-source boundary
is explicit in the register.

The final execution06 also rejects duplicated and reordered rows,so repeated good
records cannot satisfy the78-case denominator. Execution03 retains its incomplete
prospective-input refusal. Execution04/05 bodies pass,but their Xvfb wrapper exits1
after NVIDIA EGL startup crashes. Applying Mesa to that outer display as well as
the fixture yields a clean execution06 exit0. The attempt catalog and exact old/new
verifier bytes are retained. The earlier live-copy report now has one conservative
source-provenance difference:the stricter verifier tool;installed runtime source
bytes are unchanged. No earlier evidence fingerprint is refreshed.
