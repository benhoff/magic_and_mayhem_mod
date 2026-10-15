# Wine X11 display delivery observation

The bar-corrected source replay matches104original CPU source checkpoints exactly,
but its first physical witness is uniform black. This remains an open display
boundary,not an inferred driver defect. Installed Wine is11.16; local ELF census
of /usr/lib/wine/x86_64-unix/winex11.so imports XPutImage,XShmPutImage,XCopyArea,
XCopyPlane,XFillRectangle(s),XClearArea,XConfigureWindow and XMapWindow. Imported
symbols show possible routes,not reached presentation behavior. Dynamic GL,
other drawing imports and external processes remain unobserved boundaries.

The next isolated diagnostic forwards selected Unix X11 calls once and records
metadata before/after without issuing observer X11 calls. It preserves original
return/errno and holds no recorder lock across the original body. Drawable IDs
and Unix process/thread IDs are local observation identities,not game pointers
or portable native storage. Monotonic timestamps alone do not bind producer/API
causality. Native inputs never contain original image/physical output bytes.

## Selected-call evidence

Final actual Xvfb fixture04compares uninstrumented/instrumented return values,
errno and deterministic64server pixels for every wrapped method,including shared
memory images and plane copies. Five strict journal guard methods pass. Fixture01
retains a compiler indentation-warning refusal; fixture03retains prospective
source refusal before execution. Fixture02is the earlier passing development run.
A pending-only guard was adjusted to the stricter policy requiring at least one
completed call; an accidental broad test-data edit was corrected before final04.
Neither guard failure describes an original driver failure.

Original01retains588API physical witnesses and446closed selected X11 calls on
two Unix processes:11XPutImage,413XFlush,2XFillRectangle,19XSync,1XShmPutImage.
Only two image calls have800×600extent,both before the first observed primary
copy. Local Wine DLL strings include dynamic libGL/glXSwapBuffers resolution;
this is a concrete unobserved-route lead,not proof that all original drawing
presents through GL. The selected X11 journal neither tracks all other drawing
imports nor associates Unix thread IDs with original API/producer identities.

Source-only native prediction01matches106of107selected physical witnesses with
one initial468103-pixel difference;481others remain excluded. The first black
plane stays in the negative result. Subsequent source-only admission and physical
presentation remain distinct. Original manifests verify before/after; private
binary hashes and unchanged original executable sections are retained.

The next increment must observe reached GL presentation dispatch and retain its
original forwarding/refusal conditions before binding any physical causal frame.
Step2and all47journey/21operation requirements remain open. API/Unix metadata is
not a complete presented-frame catalog,and semantic selection/tooltip/pause or
120advancing update windows remain unproved.
