# Common search capture and sequential continuation replay

Build: no-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This milestone changes tracing and offline replay; no live replacement or game
executable patch is installed. End-to-end real-game sequence agreement is pending.

## Entry and return evidence

The pinned `research/runtime/decompiled/nocd/assembly/route_search.asm` shows:

- `0x0054b800`: entry starts `81 ec d4 00 00 00`, with ECX context and entry
  stack DWORDs: return address, creature, unknown argument, X, Y, Z, budget pointer.
- `0x0054be45`: sole return instruction `c2 18 00`, after register restoration
  and adding `0xd4` back to ESP. At this breakpoint ESP equals entry ESP and
  points to the same return address. The return pops six argument DWORDs.
- Initialization byte is at actual receiver `+0x20c`, not a fixed global.
  Any nonzero value resets the search; zero resumes preserved containers.

Confidence: high for ABI and stop pairing from assembly plus scripted debugger
fixtures; the new common-function controller has not yet captured a real map.
Loaded addresses are always module base plus RVA; dynamic context/object/map
pointers come from each event. Expected instruction bytes are checked before
installing the two temporary debugger breakpoints.

The two known preferred contexts are wrapper `0x00690148` and direct caller
`0x005131b3`'s context `0x006c4bd0`. Common-function tracing records return address,
thread, stack and context for any caller entering this routine, including callers
not listed ahead of time. It does not claim coverage of different algorithms or
functions that never call `0x0054b800`. Caller gameplay roles remain unconfirmed.

## Capture

```bash
./tools/trace-route-experiment.py --search-sequence --calls 20 --seconds 300
```

The existing staging, executable hash check, debugger selection and cleanup
remain in the main controller. `tools/trace-search-sequence.py` installs common
entry/return breakpoints instead of wrapper sites. World capture is automatic
in this mode; the old wrapper mode remains available. Every entry writes its own
`world-NNNN.bin/.json`, including refreshed map/terrain/creature inputs for
continuations. The context prefix before and after the call is also recorded.
These raw prefixes retain heap pointers solely as evidence; they are not restored.
The capture deadline is checked on each memory chunk, and existing cleanup
removes both debugger breakpoints before detaching.

Completed calls are flushed to `search-calls.jsonl` with entry sequence and
completion ordinals. Entry/return pairing uses thread plus entry ESP, with an
additional return-address check. Different contexts may interleave; overlapping
calls to the same context are refused. Returns belonging to calls already
running when tracing began are skipped. Initial zero-flag calls are recorded
but do not establish replay state. Incomplete pending calls leave their world
files for diagnosis, without fabricating completed records. Counts and timings
are debugger observations, not performance benchmarks.

## Replay

```bash
./tools/replay-route-world.py --sequence working/experiments/route-trace/run-REPLACE
```

The Python launcher validates all companion hashes and snapshot/call identities,
then sorts completed calls by entry ordinal. A temporary index feeds one host
process so state persists across calls. Each actual context token owns a separate
`RouteContextPrefix`, `SearchState` and previous world in `RouteSequenceReplay`.
The new C++ API is `replay_route_sequence_call` in `route_world.*`.

A nonzero observed byte starts a fresh state. A zero byte continues only if a
preceding call for that context was replayed successfully and left its model
flag zero. Queue entries, duplicate/tie order, node records, predecessors,
movement payloads and closest-node fields are retained. Each call uses its own
captured budget and refreshed world, including the creature's current route
bytes. Mutable terrain/cell/creature fields may change between calls.

Host guards refuse unobserved initialization, flag disagreement, prior failure,
changed map token/topology or changed request identity without reset. Request
identity includes creature token, start XYZ, target XYZ and unknown argument;
map identity includes cell base, dimensions, row/layer tables and plane stride.
These are conservative replay boundaries, not newly discovered engine rules.
A fresh call clears the blocked state. Starting capture mid-search remains
inconclusive until a reset is observed; no heap-tree decoder has been implemented.

For each replayed call, compare all 524 route bytes, remaining budget and output
flag. Common captures additionally compare best heuristic (`+0x211`), best node
(`+0x215`) and best priority (`+0x265`) from the recorded context. Queue/node counts
in reports are model diagnostics, not comparisons to native container contents.
A mismatch taints later continuations of that context; their observable comparisons
remain recorded but cannot be reported as validated matches until a fresh call.
Other contexts remain independent. No original output is used to seed model state.

`sequence-replay.json` stores per-call outcomes, caller addresses and aggregate
matched/mismatched/inconclusive counts. Exit 0 means every completed call matched;
1 means at least one mismatch; 2 means inconclusive calls without mismatches.
Malformed evidence or a replay exception fails rather than reporting a match.
A single continuation snapshot cannot be replayed through the single-file launcher.

## Verification

- Full-world C++ tests cover one-unit budget exhaustion, continuation to completion,
  interleaved contexts, opaque route-byte refresh, orphan continuation, reset,
  flag disagreement, moved map base and changed target without reset.
- Python world tests run the actual replay binary on a six-call synthetic sequence,
  verify context ordering, malformed call/snapshot pairs and mismatch propagation
  confined to one context. Synthetic expected output is explicitly fixture data,
  not claimed native search evidence.
- Scripted debugger tests cover common sites, a relocated module, interleaved
  returns, an initial unmatched return, an initial continuation, same-context
  overlap, changed return address and rejection of instruction-byte mismatch
  before breakpoints are installed.
- CTest includes the new common-collector tooling suite. Tests never launch or
  attach to a game. ASan/UBSan and i386 replay checks are also reproducible.

Remaining live work: load a map, issue long movement orders so some searches
exhaust their original budget, and compare a sequence through both contexts.
Use separate idle, move and attack captures to establish caller roles. Arbitrary
mid-search import still requires decoding engine priority/node trees and heap
ownership; the sequential approach deliberately does not restore raw pointers.
