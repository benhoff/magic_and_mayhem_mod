# Installed reload lighting

The original Adept campaign is saved under a fresh name in the private copied
installation. The host retains exact save bytes and their SHA-256, selects the
matching original Load row, and only then arms the lighting observer. Initial
campaign loading remains outside the captured interval.

An opt-in checked trampoline at00501e00 retains the first eight original bytes
(`64 a1 00 00 00 00 6a ff`) and executes the unchanged loader. A small return
witness records before/after light counts, return value, refresh flag, an opaque
handle-equality mask and the session nonce. It contains no game pointers. Equal
addresses do not imply unchanged ownership; the separate isolated loader tests
establish free/allocation order. The witness must match the first post-load
lighting input at caller00472201. Original loading itself is not replaced.

The normal pointer-free lighting bridge captures all source inputs, buffers and
cycle state after loading. Original-active observation precedes source-matched
opt-in replacement. Every captured result is compared with independent unchanged
PE32 replay and native64 plus ASan/UBSan replay. A continuing original scene and
strict wire refusals are separate checks. Original orchestration, serialization,
window/input and rendering remain their own contracts; this finite saved-game
journey does not assert arbitrary-save or whole-world replacement equivalence.
