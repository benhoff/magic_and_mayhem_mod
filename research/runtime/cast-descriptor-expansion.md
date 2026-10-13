# Packed cast descriptor expansion534000 (NS36)

The retained No-CD export `creature-behavior-lnbhltzt/packed_cast_decode.asm`
(SHA-256 `0e5540d9b4156f6140f3e3ebbbbe5dcc30254d5916e57c91a2f8bb0d33e76240`)
is instruction evidence for the144-byte body534000..534090. Native expansion
uses owned byte input and scalar output, independent of Qt, hooks and host layout.

Input+0c becomes output word0; six unaligned little-endian unsigned WORDs
at+0d,+0f,+11,+13,+15,+17 become words1..6. Bytes+19/+1a become words7/8,
independently mappingff toffffffff. InputDWORD+1c becomes word10; byte+20
becomes word11. **Output word9 at+24 is untouched.** Callers must preserve their
initial value. Input header/padding and all other output bytes are unchanged.
EAX returns the caller output address; the native API exposes an in-place update.
These numerical mappings are high-confidence instruction findings. Unknown
word9 and other opaque-field semantics are intentionally unnamed.

The declared domain is33-byte input/12-DWORD initialized output with distinct
ownership. Aliasing, invalid extents and raw pointers are excluded. Independent
comparison will enumerate all65,536 source/target byte pairs and vary every
other field and caller word9, check guards/full output, unchanged input/body,
original32/native64 and sanitized64 outcomes. Until execution is registered,
current comparison and complete native integration remain pending.

This is cast decoding only.57b710 admission, secondary admission/projectile
updates, spell effects, resources and native World/session composition remain
separate outstanding contracts. No gameplay balance changes.

The prospective final comparison passes65,536 cases with native64, unchanged
original32 and ASan/UBSan/LSan64 agreement. All source/compiler inputs and the
original manifest remain stable. The original output-pointer return, complete
48-byte output, word9 preservation and input/output/body guards are checked.
Current decoding comparison is scoped; cast admission/session integration is pending.
