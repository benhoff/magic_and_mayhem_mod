# Selected Unix X11 delivery calls v1

This opt-in native metadata protocol observes selected ELF-linked calls in the
private Wine/X11 reference environment. It is not a recovered game wire contract
or a complete presentation boundary. No Display/GC/XImage pointers or image bytes
enter it. Local XIDs,process IDs and Unix thread IDs identify only this run.

`MNM_DRAW_X11_DIRECTORY` is an absolute private capture path.
`x11-delivery.bin` starts with64bytes:MNMX1101,version1,record size96,maximum65536,
then zero reserved words. Records contain24little-endian words:phase1entry/2return,
process-local call ID,method,process ID,thread ID,monotonic nanoseconds low/high,
original result,original errno,eight argument scalars,and seven reserved zeros.
Methods1..9are XPutImage,XShmPutImage,XCopyArea,XCopyPlane,XFillRectangle,
XFillRectangles,XClearArea,XFlush,XSync. Method-specific arguments retain drawable,
rectangle geometry or flags; rectangle arrays/GC state are not transported.

Each original body forwards once,with original result/errno preserved; the
recorder holds no lock over it and issues no observer X11 calls. Atomically
resolved original functions are process-local. Journal writes use process/thread
serialization only while appending metadata. The bounded6MiB journal refuses
malformed paths,failed writes or budget exhaustion with97. Strict parsing joins
process/call IDs,checks argument/thread consistency,reserved words,per-process
identity gaps and within-call monotonic ordering. Pending calls remain explicit
in diagnostic partial mode; complete mode requires closed calls. Neither mode
proves all routes,physical changes,portable source ownership or producer causality.

The capture runner hash-checks a private copied64-bit ELF library and enables
preload only for the Wine launch. Bootstrapping and the external physical peer
are independent. Existing unpinned preload settings refuse. Parser failures
retain a failed capture report. Dynamic GL dispatch,other drawing imports,
external processes and unwrapped routes remain open.
