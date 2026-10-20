# Inherited kind8 native caller admission

The resident producer whitelist omitted `0x54aab2`, the continuation after the
additive raster call in `0x54aaa0`. Recognizing class `5c73f0/54aaa0` in the
read-only World observer did not repair this separate native path: its checked
caller gate still refused that wrapper before publishing an owned rectangle.

The production gate now admits this exact continuation for producer tag26.
Its ten bytes must match the pinned RET8 and padding. The predicate is shared
with native controls; surrounding readable-memory, owner-thread, protected-canvas
and owned-record checks continue to apply. The original wrapper still supplies
shape+0x34 and executes its own return. It writes no object flag or owner gate.

The whitelist generator checks the pinned `5c73f0` virtual slot, complete
`54aaa0` wrapper and continuation bytes. It preserves this static extension
when a finite input stream has no such caller. Generated reports identify its
static origin separately from observed stream entries. An authored empty queue
tests that distinction and compiles the generated predicate.

`tools/test-world-raster-callers.py` declares the scope and source closure before
executing the native predicate. It checks all32 pinned continuations and39
admitted caller/tag pairs, with2534 signature, tag, missing-byte and unknown-PC
refusal controls. Both production and selftest observation hooks compile. The
original executable hash and manifest are verified before and after the byte
inspection. The initial K8CALL report executes no original instructions and
retains that narrower scope and its source hashes.

The K8CALL2 follow-up also executes unchanged `54aaa0` in8316 declared RGB565
cases on a37x19 canvas: four positive widths, three heights, eleven X and nine Y
positions and seven channel increments. Clipped and empty intersections return
low16 zero, matching the existing native producer return policy in that domain.
This is separate from a full register-state or pixel comparison.

Existing selected primitive contracts own the additive pixel and wrapper-state
comparisons. This result supplies native caller-policy execution, compilation
and the bounded low16 original reference. Natural live encounter, physical
object/source lifetime, larger return domains, full register/pixel equivalence
and full-session replacement remain independent boundaries.
