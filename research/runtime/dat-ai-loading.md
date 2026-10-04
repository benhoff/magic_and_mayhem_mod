# DAT AI reader contracts

Reviewed 2026-10-04. Static analysis of No-CD Chaos.exe, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Addresses are preferred-image static virtual addresses, not verified live host
pointers. Confidence is high for byte extents and reader control flow; semantic
AI behavior and live integration are unverified.

| Address | Confirmed selected role |
| --- | --- |
| `0x00476756` onward | Calls experience loading and then brain loading using filename globals |
| `0x005c5f68` | Filename pointer to `AI/EXPERIEN.DAT` at `0x005da5e8` |
| `0x005c5f6c` | Filename pointer to `AI/BRAIN.DAT` at `0x005d9e44` |
| `0x00479a10` | Brain file reader; stream open, per-record key then model reader, repeats until stream failure/EOF |
| `0x00476240` | Model reader: dimension and layer count, then ordered layer records; runtime layer stride 40 bytes |
| `0x00475e60` | Layer reader: scalar, node count, node states, tail scalar and square row matrix |
| `0x00475cf0` | Node state reader: two DWORDs followed by ten DWORDs; 48 bytes total |
| `0x00477d30` | Experience file reader and surrounding bucket/processing logic; repeated record-reader calls |
| `0x00476590` | Experience record: key, vector count and words, two tail parameters |
| `0x0047ffb0` | Stream read primitive, called with explicit byte extent |

`0x00476590` stores the key at runtime object +1, vector bookkeeping at +5,
and parameters at +0x15 and +0x19. These packed object positions are not file
padding or native C++ object layout. Disassembly at `0x004765b5` reads the
count into a stack slot; `0x004765d6` retrieves it; `0x00476665..0x0047666c`
compares the payload loop index against that count. Ghidra's current pseudocode
incorrectly substitutes a constant four for this count because of imperfect
stream-call stack/signature recovery. Use the assembly and installed exact
roundtrip evidence for that field. Pseudocode is supporting evidence only.

The layer runtime node stride is 48 bytes and matrix row bookkeeping stride
is 16 bytes; the model layer stride is 40 bytes. Native code preserves stored
words and uses owned containers rather than recreating these legacy pointers
or packed object layouts. No names such as weights or neural-network topology
are treated as confirmed solely from square matrices.

The `ai/brain.dat` path used around `0x0046b077` is a separate raw-copy/logging
path through `0x00469ab0`, not the model-record reader.

`tools/export-dat-support.py --decompile` verifies the original manifest around
a hash-pinned disassembly/Ghidra read-only export. It saves selected full reader
ranges, a caller snippet, pseudocode and reference lists under a fresh
`working/decompiled/dat-support-*` directory. Decompilation can create ephemeral
functions in the read-only project; it does not persist changes to the project
or input executable. The retained JSON report links script and artifact hashes.

Confirmed storage layout and native validation are documented in
[DAT input](../formats/dat-native-loading.md). No original reader execution,
training/inference equivalence, key-to-creature assignment, save writer,
WBT-generation relationship or live replacement is established by this work.
