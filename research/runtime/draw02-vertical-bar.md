# Post-World vertical bar investigation

Pinned build: No-CD40209ca7, executable SHA-256
40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168.
Read-only disassembly retained in working/tests/draw02-post-world-*-disassembly-01.txt;
original manifest verified before/after. Static confidence is provisional until
isolated original-byte tests and original-active source observation execute.

Post-World caller4fd58b invokes58f9c0with a locked pixel base and wrapper. Its
embedded image at ECX+0x1c is copied by533a00using clip6c482c and wrapper geometry.
Bar fields are width+0,height+4,x+8,y+12,current+16,maximum+20,fill rows+24.
Resize58f750 initializes width×height16-bit words to31. Regenerator58f890 emits
black borders/empty rows and descending channel colour in occupied interior rows.
Current setters derive fill rows by signed division(height-2)*current/maximum;
that state calculation and bad input behavior are outside the first bounded model.

The proposed observer retains logical parameters at completed resize/regeneration,
marks pending/unknown/reused generations explicitly,and translates known generated
source pixels into existing point requests. It never reads the embedded original
bitmap as native input. Reachability of this writer does not yet attribute the
two300-pixel strips in the negative physical comparison. Native point operations
remain separate from original buffer lifetime and complete scanout ownership.

## Bounded source and generation evidence

Isolated reference02and04execute exact relocated bytes58f890..58f9b8in a32-bit
x86 process. Both compare3444cases/8512504words with untouched surrounding
storage. Reference01was stopped by the sandbox syscall filter before comparison;
all reports remain unchanged. Initial incorrect register classifications are
corrected; earlier prospective scope bindings remain historical,not refreshed.

The opt-in observer adds generator53,draw54and resize55entry tags. Default mode
still patches only53sites. Its local256-slot cache is invalid during generation
and resize; completed parameters are tied to the retained image address/extent.
Drawing refuses unknown/pending/changed storage,unbounded shape and mismatched
locked wrapper/stride/clip. Pending and original allocation identities are local
checks,not transported pointers or proof of all possible object reuse.

Known source pixels are reconstructed from retained logical parameters as
operation23point requests. Word14is point count,word15is known generator58f890
or initial resize58f750,word16fill rows,word17format,words19/20source dimensions,
word22caller. The payload contains x,y,derived colour; no original bitmap or
original destination words are read to produce it. A current scalar fill change
without regeneration keeps the previous generated source. Resize starts a
constant31source. Current/max arithmetic,bad original shapes and unseen buffer
mutation remain outside this bounded contract.

PE32 producer fixture01passes56compatible entry sites/53default hooks,
register/flags/x87/SSE/LastError and generation-cache,clip,pending and changed
storage guards. Original capture01fails before game startup with no inherited
display; original02uses a private Xvfb display and retains594API physical
witnesses. Its46763producer records include five generated4×75bars/300derived
points each and zero failures. This establishes reached hooks within that
capture,not complete lifetimes,frames or journeys. Source-only native comparison
is recorded separately; neither original generated source nor output planes are
native inputs.

## Source replay result

Prediction01compares104selected physical witnesses:103exact matches,one initial
468103-pixel difference,and490excluded witnesses. The prior bar strips disappear.
Hash-bound offline CPU analysis01compares every selected source checkpoint against
retained native output:49920000pixels,zero differences. Physical ordinal1has an
identical original/native CPU source at checkpoint103,but a uniform black display.
Its physical result remains negative. Actual display delivery/visibility and all
source/writer lifetimes require separate observation; no cause or full-frame
completion follows solely from this separation.
