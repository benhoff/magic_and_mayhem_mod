# Selected kind8 sprite wrappers

Pinned No-CD constructors assign vtables5c74e4and5c74fc. Their slot+0xc methods
are5492b0and549730; resource initialization namesSPRITES\\FLYS.SPR and
SPRITES\\CURE.SPR. These static facts do not establish natural queue reachability.

Both methods require a present bank, player0..7, a nonzero owner/player slot
and a nonempty bank. Flies reads owner through object+0x1c; Cure uses+0x10.
Flies consumes one random word. Cure first consumes a word and proceeds only
when (word&0x7fffffff)%100>50, then consumes a second word. Selection is the
masked word modulo positive frame count. The selected frame uses three-argument
word copy when its+0x1c field is exactly0xffffffff; otherwise it calls generic
copy with zero offsets and clipped copy only for lowAX==1. Both wrappers clear
AX and preserve the object. Original RNG implementation and raster pixels are
separate contracts. Native tokens contain no original object or frame pointer.

The72000case original/native comparison passes, including13248random calls,
3936word calls,3552generic calls and1184clipped fallbacks;64512cases perform
no raster call. Eight native controls refuse wrong class/method pairs, unknown
method values and oversized reached banks while retaining absent-bank no-ops.
Both original manifest bookends and source stability checks pass. Positive native banks are limited to4096frames as an
explicit native policy. Negative original counts, malformed pointers/arrays,
constructors/writers/lifetime, RNG implementation, pixel semantics, original
highEAX preservation, other table modes and live class/caller admission remain
outside this wrapper model. Native runtime refusal is unchanged at this step.

Ghidra's retained discovery inventory has no function body for either virtual
entry. Explicit recovered-range exceptions retain that gap:5492b0..549375 and
549730..549815(exclusive ends). The driver pins each bank-load entry, full-range
SHA-256, finalRET8 and following padding; actual unchanged method execution
supports the exceptions. External RNG/raster callouts remain separately bound
synthetic callbacks. The inventory is not presented as newly complete.
