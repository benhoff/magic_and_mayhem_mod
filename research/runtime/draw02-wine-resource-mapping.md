# Selected Wine public resource mapping

A separate metadata observer pins the installed PE32Wine11.16modules and public
DirectDraw imports before changing only loaded IAT entries. Observe texture to
resource association and map/unmap with exact cdecl ABI and original result/error/
FP preservation. Export only opaque localIDs and pitches,not host pointers.
No private object layout or GPU reads. Fixtures precede original capture; CPU
source/mapbuffer/upload/wholelifetime ownership remains separate.

ABI lead: [Wine11.16public header](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h). Map takes resource,subresource,descriptor,optionalbox,flags; unmap takes
resource/subresource. Descriptor has row pitch,slice pitch,data pointer. Getter
returns a resource for a texture. These source declarations alone are not proof
of installed machine ABI or runtime activation.

Installed module hashes are the existing pinnedddraw/wined3d pair. Selected
DirectDraw IAT/export RVAs are getter6d6dc/219320,map6d608/204490,unmap6d618/204bc0.
The read-only inspector checks names,hashes,PE32bounds andnon-forwarded exports;
the runtime checks loaded symbol/slot identity before changing only those slots.
Public map has five cdecl arguments,unmap two,getter one;actual fixtures validate
forwarding separately from the upstream declaration lead.

The128-byte journal usesMNMWRM01and64-byteheader. Opaque resource/data IDs use a
separate2048entry table,so existing submission-ID budgets remainunchanged. Map
intervals record pitches/data ID andoriginal result;failedunmap retains an active
interval,successfulunmap closes it,unknownunmap is explicit. Open intervals at
stop remaincounted rather than falselyclaimedclosed. No buffer bytes are read.

Fixture01passes six once-only forwards,results/LastError,pitches,failed map with
nulloutput,failedunmap/successfulclose andunknownunmap. Three decoder methods
pass openinterval retention/identity/reserve/truncation guards. Fixture02adds an
original-return512byteFPsnapshot comparison before live use.

Finalfixture03passes seven original forwards including original-return512byteFP
state,LastError,failedmapnulloutput and unreadableoptionalbox forwarding. Box
status2records unknown readablemetadata without dereferencing or replacing the
original API failure. The descriptor data pointer becomes an opaque localID;
no map buffer bytes are read. Original capture is a separate observation.

Original01with sourcelease diagnostics exits during early input/shutdown,with an
active sourcelease and incomplete terminals; no cause is assigned. Metadata-only
original02closes5093resourcecalls/1257mapintervals,0open/unknown,but its hostwait
misses the Quit acknowledgement before process exit. Both failed capture reports
and exact artifact/manifest hashes are retained,not refreshed into success.
The revised Quit observer records whether ACK was seen and separately requires
actual exitcode0after the exact originalMainQuit request; it doesnot invent ACK.
Fresh original03validates that path before resourceactivation closure is claimed.

Final-source original04 passes with5229calls:2647getters,1291maps and1291unmaps,
0open/unknownintervals. All1291maps have known observed texture generations;
1186use rowpitch1600/slicepitch960000.365API364EGLwitnesses andshortwhole16source
boundary remain separate. OriginalQuitexit0is verified,ACKnotobservedexplicit.
No descriptor data bytes or private object/GPUmemory are read. Exact CPUbuffer/
mapbuffer/upload relation and whole lifetime/reuse policy remain open.
The earlier combined-sourcelease shutdown coupling is retained as unresolved;
metadata-only success doesnot resolve it or promote all source ownership.


## Step 2 resource recorder retirement checkpoint (2026-10-13 UTC)

Failure provenance/storage retry committed asb7feb37cwith staged/normal hook
checks. Reviewedb670a41a..b7feb37c:14exact receipts,0gaps. Combined original32
resource capture reached its source interval but exited99after observer journal
retirement. The primary acquisition error is retained, while its complete
900request sidecar does not promote the failed acquisition.

The owned PE32 postclose regression reproducesexit99on the first getter after
seven recorded callbacks and resource journal close. Code inspection confirms
the resource-enabled flag formerly remained set after close, permitting later
callbacks to write the closed handle. The fix atomically disables recording
before retirement and rechecks under the private journal lock at both phases.
All original callbacks continue forwarding once. Track resource callbacks
through outside-lock map queries so ordinary session stop waits for them; a
forced midcall teardown remains incomplete. Map-point emission also rechecks
its flag under lock after an outside-lock export. No lock spans original calls,
source bytes or query exports; no broad IAT/module/timer quiescence is asserted.

Owned resource fixture passes normal/midcall/pending-stop cases:seven recorded
callbacks,three postclose forwards,result/LastError/512byteFP preserved,active
callback defers stop,midcall trace refused. Owned map-point fixture passes its
12forward cases and a forced query-close race:closed point journal stays160bytes,
original results/error/FP preserved and missing query trace remains incomplete.
Three resource and six map-point portable guards pass. Baselineexit99negative
and earlier source hashes remain immutable.

Fresh combined original32after the fix passes32queues/170160records/1338oracles/
27139826bytes,913paired requests,985closed storage calls,910lock-return epochs
and119local address IDs.6973public resource calls contain1727maps and matching
unmaps,0open/unknown intervals. All1727map points remain unknown:15live unknown,
895live transition-pending unknown(flags131),817stopped. Public copy-boundary
observer records0calls; absence supplies no transfer proof.584physical planes/
1643consumed inputs and actual gameexit0 pass;2927original files verify before/
after. Static joins account all913COPY/APIrequests,1177excluded APIentries and
43adjacent return reversals without changing replay ordering.

Reassessment: retirement is scoped callback observation safety, not complete
hook teardown or source ownership. Most live map queries occur before the
wrapper Lock publishes its returned cache address. The current query journal
does not retain an input address ID for those pending unknown points; add a
versioned pointer-free identity before comparing exact query/Lock-return
intervals. Do not infer current backing aliases from later pointer equality.
Earlier120checkpoint/physical failures remain failed. Step2and all47/21remain
open; steps3–7stayordered/pending.
