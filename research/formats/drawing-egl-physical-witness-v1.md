# Post-EGL physical witness v1

`MNM_DRAW_EGL_PHYSICAL=1` opts into a bounded reference-only handoff after each
successful selected EGL swap. No observer EGL/GL call or recorder lock spans the
peer wait; original Boolean/errno/EGL error state is preserved. One swap worker/
process,800×600private viewport,max1024planes and5sACK wait are native diagnostic
limits. Unknown/competing workers,path/write/ACK/budget failures refuse with97.

128byte little-endian request starts MNMEGL01:version1,size128,ordinal,Unix PID,
thread ID,call ID,method11,local display ID,surface ID,function alias,original
Boolean,errno,width800,height600,post-return request time low/high,then zeros.
Six-digit request/oracle/ACK paths use the ordinal.16byte ACK is MNMEACK1 plus
ordinal and call ID. The peer retains full independently sampled RGBA planes,
exact read spans,target placement and declared bounded settling samples. ACKs
publish atomically through a temporary file. Nothing is a native renderer input.

An explicit close flag causes the next checked selected return to close the
witness interval.32byte MNMEGD01marker records version1,PID,frame count,last
witnessed call,reason1,reserved0. Further original drawing continues normally.
Admission compares every successful selected EGL call through that closed prefix,
every request/ACK/plane hash,scope,extent,read order and transient accounting.
It never infers all display changes,submitted source,lifetime generations or scanout.

Original01retains495post-EGL full planes/237600000pixels with zero transients,
separate496API witnesses and a strict closed interval. First API plane is uniform
black;first post-EGL plane is the Main image. Independent native source replay
still gives a negative API comparison,but all104native image hashes appear among
EGL witnesses. This is content correspondence,not temporal frame assignment or
complete native parity. All duplicate/missing correspondences remain reportable.
