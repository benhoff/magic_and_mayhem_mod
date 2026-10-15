# Pinned Wine DirectDraw submission boundary

Step 2 requires a source-to-submission boundary before assigning physical frames.
The installed Wine 11.16 PE32 `ddraw.dll` imports
`wined3d_device_context_blt` and `wined3d_swapchain_present`. Pin both module hashes
and validate named imports/exports before any loaded-memory interception.

The pinned upstream Wine 11.16 [header](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/include/wine/wined3d.h)
and [export specification](https://raw.githubusercontent.com/wine-mirror/wine/wine-11.16/dlls/wined3d/wined3d.spec)
declare cdecl signatures with ten and six arguments respectively. These are
external ABI leads, not a recovered layout of the original game or proof of the
installed modules' internal execution. Runtime forward/result/error fixtures
and actual original observation are still required.

An observer may record opaque process-local texture/swapchain identities,
rectangles, flags and enclosing API/source-point metadata. It must forward once,
release recorder locks over the original body and preserve return/error state.
It must not read private Wine object layouts or texture bytes. Async worker/EGL
assignment, storage ownership, generation reuse and alternate routes stay open.

## Step 2 pinned Wine submission checkpoint (2026-10-12 UTC)

`draw02-wine-import-inventory-01.json` pins installed Wine11.16ddraw/wined3d
modules and the two selected named import/export RVAs. Four pin/PE guards and
four strict journal guards pass. The actual PE32 fixture02passes9cdecl callbacks,
9cross-thread reentries,exact arguments/single forwarding/result/LastError/x87,
nested parent42and one unbound call. The initial CRT-free link failure is retained.

Original01reaches the hooks but hits the unchanged1024EGLwitness ceiling before
Results. It retains4169pairedcontextblt rows without a terminal,so does not admit.
Original02uses the separately declared shorter diagnostic schedule and closes
all32World returns/173224source records,1676Winecontextblt calls on2threads,
370APIphysical and369post-EGL witnesses. Original sections,source pins and
manifest checks pass. The swapchain-present import is not reached in this cohort.

Offline associations01binds1675submissions toBltFast and1toBlt parent entries;
739rows haveAPIphysical ordinal and428have liveAPI-source points. These are
exact enclosing call associations,not texture-storage ownership. Opaque Wine
object identities lack release/reuse generations; asynchronousEGLassignment,
allroutes and full semantic journeys stay open. Steps3–7 remain ordered/pending
and all47/21acceptance requirements remain open. The reviewed4c4762a..ff36674
range has26matching exact file-version receipts and0gaps. Storage remains about
890GiBworking/436GiBfree;no cleanup or historical hash refresh.
