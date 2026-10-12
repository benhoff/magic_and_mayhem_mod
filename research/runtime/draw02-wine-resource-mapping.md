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
