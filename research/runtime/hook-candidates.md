# Executable hook candidates

Static inspection, 2026-10-02. No executable changes or runtime hooks were made.
Addresses below are preferred virtual addresses, not live pointers. Both PE32
images have preferred image base `0x00400000`; calculate a live address as the
actual module base plus RVA. Runtime stability has not been verified.

## Exact builds

| Build | Working executable | SHA-256 |
| --- | --- | --- |
| Retail | `working/game-clean/Chaos.exe` | `124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214` |
| Supplied no-CD | `working/game-nocd/Chaos.exe` | `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168` |

These have different code layouts. Do not use retail addresses in the no-CD
build or assume a uniform offset translates between them. The earlier retail
route candidate `0x00501c10` is **not** the no-CD hook address.

## Route-request boundary

| Observation | Retail VA | No-CD VA |
| --- | --- | --- |
| Function reading configured node limit | `0x00501c10` | `0x00512800` |
| Node-limit global | `0x005be5c4` | `0x005e174c` |
| Configuration assignment | `0x004d644b` | `0x004e554b` |

Evidence: follow the `MaxNodesForRouteFinding` string reference through the
configuration reader, then find code references to the destination global.
In the no-CD build, the string is at `0x005e3acc`; its push instruction is at
`0x004e551d`. The route candidate has RVA `0x00112800` and starts:

```text
00512800  51                 push ecx
00512801  8b 54 24 10        mov edx,[esp+0x10]
00512805  55                 push ebp
00512806  a1 4c 17 5e 00     mov eax,[0x005e174c]
0051280b  8b e9              mov ebp,ecx
```

Confirmed static observations:

- Direct callers include `0x0050bd0e`, `0x0051cc91`, `0x0051d36a`,
  and `0x005213e6`; caller code initializes ECX with an object pointer.
- At `0x0051284d` it calls `0x0054b800`, with ECX set to `0x00690148`.
- It copies `0x83` DWORDs from that global area into object offset `+0x96b`.
- It reads object offset `+0x97b`, returns a normalized zero/nonzero result,
  and exits with `ret 0x10` at `0x00512892`.

Inference (medium confidence): this is an object route-request/result wrapper,
and `0x0054b800` is a candidate for the underlying search routine. ECX and
stack cleanup suggest a thiscall-like interface with 16 bytes of stack
arguments. Their types and semantics are not yet established. Do not label
the copied memory as a known route structure or assume this is A* yet.

The first five bytes are two complete instructions without relative control
flow, making an entry detour mechanically plausible. That does not establish
that a hook is safe: displaced instructions must execute in a trampoline,
registers/flags and the ABI must be preserved, and concurrency/reentrancy must
be understood. The global scratch area is a reason to investigate reentrancy.

Recommended first experiment: logging-only hook or debugger breakpoints at
entry and return, observing ECX, four stack arguments, return value and timing
for a simple move, blocked destination, and several simultaneous orders.
Only then infer argument types and route layout. Leave gameplay unchanged.

## Rendering

Confirmed no-CD import: `DDRAW.dll!DirectDrawCreate`, IAT slot
`0x005c5014` (RVA `0x001c5014`), reached through thunk `0x0059755a`:

```text
0059755a  ff 25 14 50 5c 00  jmp DWORD PTR [0x005c5014]
```

Direct call sites include `0x0058a9c3`, `0x0058aaa4`, and `0x0058f173`.
Confidence: high for import identification; no presentation path has been
traced live. An IAT interception or a forwarding DirectDraw proxy could wrap
the created interfaces to log display modes, surface creation, and presentation.
That is a candidate for output scaling, not proof that all rendering passes
through a particular surface method. A proxy must preserve the original API
and Wine loading behavior; no proxy has been installed.

Resolution setup also contains hard-coded choices:

| Observation | Retail VA | No-CD VA |
| --- | --- | --- |
| Width assignment | `0x004d9e55` | `0x004e8fea` |
| Height assignment | `0x004d9e63` | `0x004e8ff8` |
| Width global | `0x006331b0` | `0x00656610` |
| Height global | `0x0063487c` | `0x00657cd8` |

In no-CD code, the flag at `0x006de6d5` selects width `640/800` and height
`480/600`. Other routines independently calculate the same dimensions, e.g.
around `0x00469c31`, `0x00469c78`, `0x0046a9b2`, `0x0046a9c2`,
`0x00473e69` and `0x00473e79`. Confidence: high for these constants;
their complete consumers and buffer layouts remain unmapped. Changing two
globals alone is not a validated native-resolution solution. Output scaling
is a smaller experiment than changing internal buffers, viewport, UI and
input-coordinate calculations together.

## Configuration lifecycle

| Candidate function entry | Retail VA | No-CD VA |
| --- | --- | --- |
| Startup CFG materialization path | `0x004d9bf0` | `0x004e8d80` |
| Shutdown CFG repacking path | `0x004da8a0` | `0x004e9a30` |

No-CD startup begins `81 ec b4 04 00 00` (`sub esp,0x4b4`).
Shutdown begins `64 a1 00 00 00 00` (`mov eax,fs:0`) and establishes an
exception-handling frame. Both paths reference the encrypted CFG glob, at
`0x005e4140`, via operand addresses `0x004e8da9` and `0x004e9b2b`.
Static references plus the existing CFG experiments support these lifecycle
roles; exact internal function boundaries/roles still benefit from live tracing.
Confidence: medium-high. Hooking exception-frame setup requires particular care.
The data-only encoder already handles configuration edits, so hooks here are
lower priority than route or rendering instrumentation.

## Decompilation and safe progression

Ghidra was not found on PATH during this inspection; `objdump` is available.
Use Ghidra to import the exact no-CD PE, run auto-analysis, and inspect the
route wrapper and its callers, then `0x0054b800`. Decompiler output is inferred
C-like pseudocode, not the recovered original source or a buildable replacement.
Correcting types and calling conventions helps improve it. Always confirm
hook instruction boundaries in disassembly.

Suggested sequence:

1. Decompile and label the route wrapper, search candidate and callers.
2. Validate argument/structure interpretations with logging or breakpoints.
3. Build one opt-in, hash-checked, reversible logging-only hook.
4. Establish baseline movement tests before changing the search logic.

No runtime validation was performed in this inspection. Wine is restricted by
the current sandbox, so live validation will require a user-shell run.

## Reproduce static evidence

These commands consume working copies only and do not modify them:

```bash
sha256sum working/game-clean/Chaos.exe working/game-nocd/Chaos.exe
objdump -p working/game-nocd/Chaos.exe
objdump -d -Mintel working/game-nocd/Chaos.exe | rg -C 8 '5e174c|5c5014'
objdump -d -Mintel working/game-nocd/Chaos.exe | rg -C 4 'call.*0x512800|call.*0x59755a'
objdump -d -Mintel --start-address=0x512800 --stop-address=0x512895 working/game-nocd/Chaos.exe
```

Decompiler workflow reference:
[Ghidra beginner guide](https://ghidra.re/ghidra_docs/GhidraClass/Beginner/Introduction_to_Ghidra_Student_Guide.html).
