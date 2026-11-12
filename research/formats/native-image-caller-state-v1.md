# Native image caller-state diagnostic v1

Private fixture output is a 16-byte little-endian header followed by complete
164-byte records. Header DWORDs are `494d4e4d,31305343,1,164` (`MNMICS01`).

Each record contains 41 DWORDs:

| Indices | Meaning |
| --- | --- |
| 0 | Consecutive one-based case ordinal. |
| 1..5 | Format branch, signed X/Y anchors, clip/shape/pitch mode, FP seed. |
| 6..7 | Independently allocated original/native destination anchors. |
| 8..23 | Original state. |
| 24..39 | Native state. |
| 40 | Complete destination, unchanged source and selected FX equality bits 0..2. |

Each state consists of EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, EFLAGS, six retired
RET24 arguments, and LastError. Argument slots are sampled before PUSHFL can
overwrite a dead slot. Flags compare mask `cd5`; undefined/privileged state and
x87 instruction/data history are excluded. EDX and argument0 normalize only
inside the corresponding full 4,096-byte destination buffer. Other values remain
literal. The reader rejects shared buffers and validates the entire ordered
2×11×8×9×12 domain plus original/native destination, pitch, unused and Y arguments.

Mode8 uses negative64-byte pitch and offset2048; other modes use positive64
pitch and offset512. Widths vary in1..31, with explicit zero-width/height modes.
Twelve seeds combine three legal masked x87 precisions with four rounding modes,
four stack paddings, three active x87 values and deterministic XMM/MXCSR bytes.
Source bytes and both complete destination seeds are authored by the fixture.
Wire identity alone grants no operation or player acceptance.
