# DRAW-02 public Wine resource-copy observation

The candidate probe excludes a small offset between relevant client-source and
cached CPU buffers. This observer investigates the public transfer boundary;
it does not assume that an observed copy completes an upload or owns storage.

The installed Wine11.16PE32module hashes remain pinned by
`inspect-drawing-wine-imports.py`. Read-only copy inspection pins the ddraw IAT
RVA0x6d598, wined3d export RVA0x1df0d0and its first16bytes. The public header
provides the cdeclten-argument HRESULT ABI:
https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h
The source is an ABI lead; installed-binary pins and execution remain separate.

The opt-in hook forwards the original call exactly once with all ten arguments,
result/LastError and FP return state preserved. It records portable source and
destination resource identities, observed texture associations/generations,
subresources, destination origin, source box state, flags, thread/API parent and
resource/lifetime prefixes. It reads no resource fields or pixel bytes.

MNMWCP01/version1uses a64byte header and192byte rows, maximum131072rows.
Entry/return identity and closure are checked. Unknown resource/texture links,
null/unreadable boxes and original failures remain explicit. Calls return before
any separately unobserved asynchronous copy completion; no completion claim.

The PE32 fixture passes three controlled calls with known/null/unreadable boxes,
exact ten arguments, original success/failure, LastError and a512byte initialized
FP comparison. Four decoder methods and counter-only control guards pass.
Original transfer-route observation remains a separate bounded run.

DRAW-02 step2 public resource-copy increment: exact installed module/IAT/export/
prologue pins, three PE32cdecl10arg result/error/FP forwards and4decoder/control
methods pass. Original01normal Quit353API352EGL observes0public copy calls,
so this boundary doesnot explain split client/CPU buffers within the sample.
A source lead is the original58d540routine called during surface setup; inspect
its exact bytes/behavior before extending hooks. No alias/upload/ownership/full
47/21orsteps3–7promotion follows; current native parity is not asserted for
this isolated metadata increment.
