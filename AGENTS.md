# Magic & Mayhem Reverse Engineering

## Goal

Build a documented, reproducible modding toolkit for Magic & Mayhem (1998),
with an initial focus on three gameplay systems:

1. Commander orders for summoned creatures.
2. Per-creature veterancy.
3. A more active mana tempo/economy.

## Rules

- Never modify files in `original/`.
- Treat `original/` as read-only input, including the scripts stored there.
- Run `./tools/original-manifest.sh verify` before and after experiments that
  consume original artifacts.
- `working/` may be modified and may contain generated or installed game files.
- Every binary modification must be performed by a script.
- Before patching a binary, verify the expected original bytes and/or executable
  hash.
- Record discovered file structures under `research/formats/`.
- Record runtime structures, functions, addresses, signatures, and hooks under
  `research/runtime/`.
- Prefer data/config modification over executable patching where possible.
- Prefer isolated runtime hooks over broad executable rewrites.
- Keep experiments small, reversible, and independently testable.
- Separate confirmed findings from hypotheses.
- For reverse-engineering findings, record the evidence and confidence level.
- Do not assume pointer addresses or offsets remain stable across launches unless
  verified.
- Avoid introducing gameplay balance changes while still validating engine
  behavior.

## Repository layout

- `original/`: immutable source media and supplied binaries.
- `working/`: disposable generated installations and experiment outputs.
- `tools/`: reproducible preparation, inspection, launch, and patch scripts.
- `research/formats/`: documented on-disk structures and configuration formats.
- `research/runtime/`: documented runtime structures, functions, and hooks.
- `patches/`: patch specifications and release metadata, not patched binaries.
- `tests/`: automated checks and repeatable manual test protocols.

## Game launch

Once the working installation and launcher have been created, use:

```bash
./tools/run-game.sh
```

