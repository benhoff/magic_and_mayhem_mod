# No-CD decompilation baseline

Keep `raw/` as the unedited Ghidra baseline. These seven files are inferred
pseudocode, not original source, buildable C, or verified hook signatures.
Do not run formatters or refactor these files. Put future annotated/readable
reconstructions outside `raw/`, with source addresses and confidence notes.

Source executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Preferred image base: `0x00400000`. Tool: Ghidra 12.1.4 PUBLIC, with its bundled
C++ decompiler built for Linux ARM64. Export date: 2026-10-02.
Scope: selected pathfinding routines, not the entire executable.

| File | Investigated role |
| --- | --- |
| `raw/00512800.c` | Route-request wrapper |
| `raw/0054b800.c` | Search |
| `raw/004ebae0.c` | Neighbor expansion |
| `raw/004ec780.c` | Heuristic |
| `raw/0040e290.c` | X coordinate wrapper |
| `raw/0040e8a0.c` | Y coordinate wrapper |
| `raw/0040eeb0.c` | Z value copy |

Original run: `working/decompiled/nocd-nn0_667n/`; seven successful exports,
zero failures, executable unchanged. The complete project and log remain local.
Each raw file's header references that run's `manifest.json`; this README
records its relevant provenance for the tracked snapshot.

`assembly/` preserves the corresponding instruction listings. Only the input
filename header has been normalized to `Chaos.exe` for portability. The
portable `manifest.json` pins the executable, tool versions and SHA-256 of all
14 artifacts. Verify with `./tools/verify-decompilation-baseline.py`; add
`--executable working/game-nocd/Chaos.exe` to check the listed bytes against
the exact working PE as well. Do not edit these baseline files.

Reproducible markup lives in `tools/ghidra/AnnotateRouteMilestone.java` and
readable, tested models in `reconstruction/pathfinding/`. Neither replaces
this baseline or modifies the game executable.

Reproduce with `./tools/decompile-game.py`. It creates a fresh ignored evidence
directory and never replaces this snapshot. See
[route research](../../route-decompilation.md) for interpretation and caveats.
