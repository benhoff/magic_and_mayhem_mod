# Direct-word backend state test fixtures, version 1

These are private offline test fixtures, separate from the live diagnostic wire
contract. All integers are little-endian. They carry transient PE32 identities;
addresses are compared within one execution and are not stable across launches.

The input uses the 208-byte prefix and encoded-frame/canvas layout documented in
[word-sprite-samples-v1.md](word-sprite-samples-v1.md). Only this private probe
interprets the normally reserved DWORD at byte 76 as a seed selector `0..3`.
Live diagnostic records keep that word zero. Seeds select EFLAGS, x87 rounding
control and masked sticky/C1 status, XMM values and MXCSR. Workspace before is
independently seeded. The after-canvas is an expanded-mask oracle read only by
the comparator, after drawing. State prediction and native drawing never use it.

`state-results.bin` begins with `MNMWST01`, DWORD version `1`, and DWORD record
size `256`. The remainder is one 64-DWORD record per manifest line:

| DWORD | Meaning |
| --- | --- |
| 0 | Original/model check bitmask; all ten checks pass at `1023`. |
| 1..3 | Selected backend RVA and observed frame/canvas identities. |
| 4..5 | Seeded flags and original return flags. |
| 6 | First workspace mismatch, or `0xffffffff`. |
| 7 | Whether existing strict native admission accepts the request. |
| 8..23 / 24..39 | Original / independently predicted 16-word workspace. |
| 40..47 | Original EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI. |
| 48..49 / 50..51 | Predicted / observed signed argument X,Y. |
| 52 | Actual native workspace helper's per-word mismatch bits. |
| 53 | Admitted scalar request whose observed workspace word9 refutes old zeroing. |
| 54..55 / 56..57 | Original x87 status before/after and control before/after. |
| 58..59 | Original MXCSR before/after. |
| 60 | Production entry assembly/native-core comparison mask. |
| 61..62 | Original pre-call and return ESP. |
| 63 | Zero-dimension frame flag. |

Check bits 0..9 cover workspace, arguments, integer registers, stack/guards,
defined XOR-zero flags plus DF, x87 control/status/tag, three active x87 values,
XMM0..7, MXCSR, and canvas guard/oracle plus unchanged source respectively.
AF is undefined by XOR and excluded. Floating instruction/data pointers, opcode
and inactive x87 slots are excluded. Native entry tests use the production
assembly with a host-only admission/logging stand-in, not Win32 process hooks.

The parser and probe sources are `tests/word-backend-state-reference.cpp` and
`tests/word-backend-probe.S`. The producer and result hash accounting are in
`tools/test-word-backend-state.py`. Partial headers/records and unknown input
extents are rejected. Expected original bytes are never used as drawing input.
