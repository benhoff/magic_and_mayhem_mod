# Logging-only x86 neighbor bridge

## Scope and confidence

Implemented 2026-10-02 for the pinned no-CD working executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The first live validation unit is one standard creature expansion at
`0x004ebae0`: descriptor mode DWORD `+0` is zero, link DWORD `+0x3a` is
zero, and the input budget is positive. Other calls forward unchanged.
The alternate six-link branch and other search entry points remain outside
this capture scope; their offline reconstruction is separate.

**Confirmed in a synthetic Wine PE32 harness:** the added import loads the
bridge, the hook forwards to the supplied expansion, preserves its return
value, carry flag, nonvolatile registers, callee stack cleanup, x87 stack,
SSE/MXCSR state and Windows last-error state, retains the
existing vector prefix, and records an expansion that matches the C++ model
across all 36-byte candidate records and budget. Unsupported descriptor mode
and exhausted-budget calls still execute the supplied function. Evidence:
`working/tests/neighbor-shadow/` (each run has a manifest, Wine log, input
pair and comparison report). Reports explicitly label synthetic origin.
Latest full harness: `working/tests/neighbor-shadow/run-oxtru6p_/`.
Native i386 and sanitizer replay both match that capture; all 18 reconstruction
CTest suites pass. Confidence is high for that tested bridge contract.

**Unvalidated:** loading this bridge in an actual map and matching an original
engine expansion. A synthetic expansion does not establish game equivalence
or prove crash-free gameplay. No game was launched during implementation.

## How it works

`tools/build-shadow-bridge.py` uses Clang, llvm-dlltool and lld-link to build a
freestanding PE32 i386 DLL without a CRT or Windows SDK. The DLL imports
Kernel32 functions only. Build manifests record source and DLL hashes; PE
link timestamps are fixed to zero.

`tools/prepare-shadow-experiment.py` checks the source executable hash, copies
`working/game-nocd` into a fresh experiment, and adds a `.mnmcap` data section
containing a replacement import-descriptor table and one `MnmShadow.dll`
import for `ShadowAnchor`. Existing code sections and old IATs are retained.
The patch is scripted; neither `original/` nor the source working executable
is modified. Staging and launch manifests pin the patched EXE and DLL hashes.

At process attach, the DLL verifies FXSAVE support and the six instruction
bytes `81 ec 8c 00 00 00` at image-relative `0xeaae0`. A trampoline executes
that displaced `sub esp,0x8c` and jumps back. Unsupported hosts pass through;
DLL loading does not fail merely because installation was skipped. Installation
happens before normal game execution, rather than while another game thread
may be executing that instruction. This is a process-lifetime import: dynamic
unloading is unsupported.

Entry assembly saves flags, all general registers, and a 16-byte-aligned
FXSAVE image. The C logger executes with DF clear; assembly restores the
saved state before forwarding. The pushad frame exposes ECX at index 6,
original return at 9, and the four stack arguments at 10..13 (output vector,
descriptor, prior payload, budget pointer). These indices are specific to this
hook, not a general debugger frame format.

For a sampled call, the logger freezes the descriptor, payload, world and
existing candidates, and substitutes a return trampoline on that call's stack.
The original function still generates candidates and consumes the budget.
Its `ret 16` returns to the logger, which records the full resulting vector
and budget and resumes at the saved caller address. Registers, flags,
floating state and last-error state are preserved again around return logging.
No reconstruction runs in the game process. Comparison happens afterward in
`neighbor-replay`, so this milestone measures original behavior without
replacing the search algorithm.

A single atomic busy flag skips overlapping/reentrant captures. Default limit
is one eligible sample; environment limit is capped at 100. Dimensions, pointer
spans, candidate counts and allocation sizes are bounded; unreadable or
unsupported data produces no completed comparison claim. Logging uses
CREATE_NEW and launch refuses a reused evidence directory. Disk I/O and
allocation occur on the calling thread and can delay the game. This is not
benchmark instrumentation. VirtualQuery checks are best effort: they do not
freeze other threads or guarantee that a world cannot change concurrently.

## Capture and compare

Stage only (does not launch the game):

```bash
./tools/prepare-shadow-experiment.py
```

Use the printed directory as `RUN`. Launch when ready to load a map and issue
movement orders:

```bash
./tools/run-neighbor-shadow.py "$RUN" --samples 1
```

This goes through `tools/run-game.sh --runner tools/shadow-game-runner.py`;
the adapter launches the separately hash-checked staged executable. Generic
launcher metadata describes its usual baseline; the experiment manifest
identifies the actual staged executable. Exit the game before comparing:

```bash
./tools/compare-neighbor-shadow.py "$RUN"
```

A completed sample contains `game/shadow/world-0001.bin` and
`expansion-0001.bin`. The report checks every candidate byte, ordering, the
existing prefix and budget; it records input hashes and replay executable
hash. A mismatch exits nonzero. Missing/incomplete captures are refused,
not counted as matches. The first replay accepts canonical row/layer tables
and plane stride only, rejecting layouts whose inverse coordinate map could
differ from the engine's division-based coordinate calculation.

To verify the bridge without a game session:

```bash
./tools/test-shadow-bridge.py
```

That builds a supplied synthetic expansion, applies the added-import patch,
runs it in `working/tests/shadow-wine`, and compares its single capture. It
requires working Wine and wineserver IPC. Offline staging/parser checks run
in CTest as `neighbor-shadow-tooling` and via `tests/test-neighbor-shadow.py`.

## Primary interface references

The loader and import layout follow Microsoft's
[PE format reference](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format).
Process-attach initialization is kept to the Kernel32 operations described in
[DllMain guidance](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain).
These references establish platform interfaces, not game-specific semantics.

Prepared but unlaunched game capture: `working/experiments/neighbor-shadow/run-34mzkbq2/`.
Its EXE / DLL hashes were verified after the final builds.
