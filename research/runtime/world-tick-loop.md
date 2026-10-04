# Original world update loop: static recovery

Reviewed 2026-10-04. The strongest recovered world-update entry is
**`0x0046afc0`**, reached through the active gameplay screen's virtual update
method. This is original PE32 engine code, not a native host implementation.
The message loop, world update, creature maintenance, budgeted creature work,
and timer callbacks are distinct functions.

For pool identity, creation, cleanup versus release, slot reuse and teardown
dependencies, see [entity lifetimes](entity-lifetimes.md).

No game was launched, hook installed, or binary changed for this investigation.
Confidence is high for the instructions, table entries and static call edges
below; live activation, thread IDs, cadence, pause semantics and complete
gameplay equivalence remain unverified. This advances discovery, not replacement.

## Build and reproduction

All addresses below apply only to supplied No-CD `working/game-nocd/Chaos.exe`:

- SHA-256: `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
- Architecture: PE32 i386; preferred image base `0x00400000`.
- Discovery: import/IAT resolution, whole-executable Intel disassembly, caller
  searches, file-backed vtable inspection, and selected Ghidra pseudocode.
- Ghidra 12.1.4 reused the existing analyzed project in read-only/no-analysis
  mode. Inferred pseudocode helped navigation; assembly establishes the findings.
  Some routines were missing from its function inventory, so absence of a
decompiler reference is not absence of code.

Reproduce the assembly and table evidence:

```bash
./tools/original-manifest.sh verify
python3 tools/export-world-tick.py
./tools/original-manifest.sh verify
```

The exporter checks the executable hash, seven instruction anchors and the
gameplay update table slot before producing a fresh
`working/decompiled/world-tick-*` directory. It records named disassembly
ranges, direct call/tail-jump reference contexts, table values, artifact hashes
and tool version, and checks the executable again afterward. No original
artifact is an analysis input; original-manifest checks additionally verify
repository input integrity. Direct-reference scanning is a discovery aid:
objdump can decode inline data as instructions and indirect edges require
separate review. There is no executable patching or runtime observation.

Recorded export: `working/decompiled/world-tick-7eensd6i`. Validation checked
all 21 artifact hashes across 20 assembly ranges and the direct-reference
report, the update vtable slot, both direct/tail callers, search-budget evidence,
Python syntax and unknown-build rejection. Original-manifest verification
passed before and after the investigation (2,927 files).

Do not reuse the clean executable's timer addresses from
[threading evidence](threading.md) in this build. Clean `0x00521f40` is a timer
wrapper; No-CD `0x00521f40` is inside creature-related code. No-CD timer
wrappers instead start at `0x00534ad0` and `0x00534b60`.
Preferred VAs are not observed host pointers. Future hooks must verify the
loaded image/build and translate addresses relative to its actual image base;
discover pool pointers and active screen identity each run.

## Dispatch chain

```text
0x004e9a08 calls 0x00557370: outer Windows message loop
    drain messages with PeekMessageA/GetMessageA/TranslateMessage/DispatchMessageA
    if active flag [0x006c5038] != 0: call 0x004e3e40 (pacing)
    obtain active screen pointer from [0x006f34e0]
    if screen exists and active flag remains nonzero:
        call screen.vtable[0x10 / 4] at 0x00557421

gameplay vtable 0x005c5dd8, slot +0x10 -> 0x0046afc0
    state/exit gates and command acceptance
    advance counters
    per-creature maintenance: 0x00509e50
    budgeted/staggered creature work: 0x0052a780
    secondary creature pass: 0x0052a9f0
    other world/effect/interface/audio operations
```

`0x00557370–0x00557450` repeats until its exit condition or WM_QUIT path.
`0x00557403` calls pacing, `0x00557408` loads the current screen pointer,
and `0x00557421` dispatches its `+0x10` slot. Other screens can implement
the same slot differently; this is not an unconditional world tick.

The gameplay constructor installs table `0x005c5dd8` at `0x0046a10b` and
stores screen discriminator `2` at object `+4`. The six inspected table entries
are `0x0046a460`, `0x0046ab30`, `0x0046ae90`, `0x0046aef0`,
`0x0046afc0`, and `0x0046c070`. Screen push/pop at `0x00557040` and
`0x00557130` manipulate `0x006f34e0` and dispatch lifecycle slots.
Those provide concrete leads for Qt screen integration; the discriminator's
full enum and pause behavior have not been recovered.

Two additional static routes invoke the world update with ECX=`0x006cbb78`:

- `0x004757df`: **tail jump**, after checks in wrapper `0x004757c0`.
- `0x0057689a`: call in an alternate screen path, conditional on its `+0x250`
  field being `2`. Its complete parent function/mode remains unresolved.

Do not assume the world update runs only when the gameplay screen itself is
the top screen. Also, Ghidra classified the tail jump as a call reference;
the actual opcode at `0x004757df` is `E9`.

## World state and order within `0x0046afc0`

The entry receives its object in ECX, preserves it in ESI, and returns with
plain `ret`; no stack argument is visible. Treat this as an observed register
contract, not a complete C++ class or proven decompiler calling convention.
`0x006cbb78` is a statically referenced gameplay object address; its complete
layout and all ownership relationships remain unknown.

| Evidence site | Confirmed operation | Interpretation / confidence |
| --- | --- | --- |
| `0x0046b444` | Calls `0x005383c0` on `0x006a2898` with current object `+0xfd62` and `1`; zero return exits before counters/pool updates | Command/event gate; high for control flow, provisional for complete role |
| `0x0046b45f` | Increments object DWORD `+0xfd62`; forwards value to `0x00487b10` and `0x00485b00` | Session/update sequence candidate; high for operation, provisional name |
| `0x0046b488–0x0046b496` | Increments DWORD `0x006c4830` when object `+0xfd5a == 0` | World tick counter candidate; high static confidence |
| `0x0046b72e–0x0046b748` | Walks pool pointer `[0x006def58]`, count `[0x006df180]`, stride `0xe4b`; skips records with DWORD `+4 == 0`; calls `0x00509e50` | Creature maintenance pass; high within known creature layout |
| `0x0046b79f`, `0x0046b7a9` | Calls `0x0052a780`, then `0x0052a9f0`, both on pool manager `0x006def58` | Ordered additional creature passes; high |
| `0x0046b7bb` / `0x0046b7d4` | Conditional `0x005404a0` calls on `0x006eaf08`; alternate branch loops ten times | Additional subsystem work; exact semantics not recovered |
| `0x0046b805` / `0x0046b828` | Walks another pool with stride `0x22e`, calls `0x004970e0` for records with nonzero `+4`; normal bound `[0x006898dc]`, alternate bound 125 | Secondary object updates; exact entity type provisional |
| `0x0046b851`, `0x0046b85b` | Calls `0x004f2e20`, then `0x004f3900`, on map context `0x006c5490` | World/map-related updates; exact contracts unreviewed |
| `0x0046b87f` | Calls `0x0046f5d0`, which processes queued map-cell work with explicit limits | Further bounded world work; full contract unreviewed |
| `0x0046b899` | Conditional call `0x00571300` on `0x00658178` | Connects to the known audio manager path; audio reconstruction remains separate |

The counter is consumed by `0x00509e50` for deadline comparisons and by other
creature-related routines for modulo checks. Initialization also resets it
at `0x00470a68`. These strengthen the tick interpretation; they do not prove
the meaning of every clock or that all update invocations advance it.
The alternate `+0xfd5a` branch skips the main creature passes while retaining
other work. Naming that branch "paused" would be premature.

`0x00509e50` contains countdown/deadline handling, conditional creature state
updates and positional audio calls (including `0x00571460`). It is one part of
creature updating, not the entire AI/movement/combat state machine.

## Budgeted creature work

`0x0052a780` is a particularly useful migration boundary:

- Writes manager `+0x230 = 0x35` (53), or zero if global `0x006e1f44` is nonzero.
- Uses manager `+0x258` as a continuing creature index, resolves records with
  stride `0xe4b`, and checks record `+4` and `+0xe4` before `0x00519ae0`.
- The first loop consults the budget field, selected work's return, another
  manager flag, and index wrapping. Its complete debit/resume contract still
  needs recovery; 53 must not be interpreted as "53 creatures per tick".
- A further pool pass selects records by index modulo 20 matching manager
  `+0x25c`, and calls `0x00522ad0`, `0x00518420`, `0x005244f0`.
- Another conditional group uses index modulo 90 matching manager `+0x264`
  before `0x00523470`.
- At the end, those two manager phase counters advance and wrap at 20 and 90.

These are staggered work phases, not measured milliseconds or frequencies.
There is already scheduling behavior to preserve before proposing worker
threads. A concrete connection to pathfinding is now established: manager
`0x006def58 + 0x230` aliases global **`0x006df188`**, the shared expansion
budget used by the [cached search caller](pathfinding-search-model.md).
`0x00513162` reads that budget, selects the minimum of it and the separate
`0x006df18c` budget, and calls search at `0x005131b3` using context
`0x006c4bd0`. After return, `0x005131c7–0x005131d3` subtracts consumed
expansions from both globals. Thus 53 is an expansion budget shared across
that path, not a creature count. The full scheduling debit/resume policy and
complete call chain from selected work to every route request remain unresolved.

## Pacing and timers are separate

`0x004e3e40` uses No-CD `GetTickCount` IAT slot `0x005c5164`, compares elapsed
time against an interval, and busy-waits at `0x004e3f89–0x004e3f93` on one path.
It stores the next timing baseline at `0x006e8100`. It also updates
`0x006e1f00`, which suppresses selected later work in the world update; the
meaning of the entire adaptive pacing policy remains provisional.

Gameplay entry `0x0046a460` initializes interval `0x006dbe95`:

- If `0x0068991c != 0`, stores 50 ms and companion value 20.
- Otherwise stores integer `1000 / [0x006de6fd]` and copies that divisor into
  companion field `0x006dbe99`.
- Pacing can further adjust its selected interval using other mode/state values.

Thus a nominal 50 ms branch is statically present, but a universal 20 Hz or
50 Hz world tick is **not** established. Actual settings, branches, stalls,
event/network readiness and execution cadence require live observation.

The No-CD timer wrapper `0x00534ad0` calls `timeSetEvent` via IAT
`0x005c52e0`, with delay from receiver `+0`, resolution `+4`, supplied callback,
user data zero and flags `1`. `0x00534b20` is a separate flags-zero variant;
`0x00534b60` calls `timeKillEvent` via `0x005c52b8`.

At `0x0055772a–0x0055773c`, initialization supplies delay 20, resolution 5,
callback `0x004eaaf0`. That callback simply calls `0x004a4910` and returns
`ret 0x14`; its worker reads cursor coordinates, uses mutexes and performs
cursor-region/surface work. It is not the recovered creature/world-update entry.

Other inspected starts at `0x0046a8b1`, `0x0046b6b6`, `0x0047268c` supply
1000 ms, resolution 5, callback `0x00469a60`. That callback writes small
global flags/counter-derived values and returns `ret 0x14`; it does not call
the creature pool update. A further timer registration at `0x0057821f`
uses callback `0x005753e0`; that callback's full role was not reviewed.

The static chain places world work on the thread executing the message loop
when that dispatch path is active. It does not prove a single-threaded process,
exclude other world callers, or establish which Wine thread owns live gameplay.

## Smallest next live observation

Observe without changing scheduling or calling game methods from Qt:

1. Log bounded entry/exit records at message-loop dispatch, `0x0046afc0`,
   `0x00509e50` and `0x0052a780`, with thread ID, monotonic timestamp and ECX.
   Distinguish update invocations from counter-advancing invocations.
2. Capture active screen/vtable identity and pre/post values of world counter,
   object `+0xfd62`, mode `+0xfd5a`, scheduler index/budget/phases, and pacing
   interval. Validate pointers and bounds before reading pool records.
3. Exercise idle gameplay, one moving creature, multiple orders, in-game menu,
   focus loss, map transition and exit. Record actual map and settings.
4. Compare timer-callback thread IDs with the world-update thread. Confirm
   whether alternate screens continue world work and what actually pauses it.
5. Use a separate low-overhead run for cadence/profiling; debugger stops and
   diagnostic logging are not reliable frame-time evidence.

Do not promote these addresses to replacement hooks until entry bytes/ABI,
live call order, lifetimes and failure behavior are validated. A Qt session
command drain point is now a concrete research target, not an approved ABI.
