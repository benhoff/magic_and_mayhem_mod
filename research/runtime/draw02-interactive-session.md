# DRAW-02 interactive session and caller-result acquisition

The opt-in `MnmDrawing.dll` observer extends finite opaque-site acquisition to
common DirectDraw surface API mutation routes and actual original input polling/
scan-code conversion. It forwards original calls once and records their returned
HRESULT/LastError, logical lifetimes, thread/entry/completion identities and every
admitted full primary mutation plane. Menu dispatch phases use the existing adapter
ABI. Original drawing and simulation remain active; no native drawing or input
replacement is enabled.

The pinned NoCD import thunk at0x59755a and user32 import entries at0x5c51f8 are
checked before installation. Dynamic DirectDrawCreate/Ex lookup and supported
DirectDraw/surface interface acquisition are observed. Common surface slots0..35
use the locally installed Wine SDK ABI; extended methods and original engine
branches are not inferred from this API coverage. Win32 forwarding fixtures cover
eight primary mutators under success/lost/busy/generic-error callbacks, including
LastError and x87 preservation and full readbacks. Synthetic callbacks do not
establish actual original game or driver failure equivalence.

[Wire/admission contract](../formats/drawing-api-session-v1.md). The observer
refuses reference completion on missing terminal markers, dropped records/planes,
unpaired applied actions, unknown aliases/routes, readback failures or exhausted
budgets. `observed_primary_api_mutations` remains distinct from complete physical
presentation: external window/GDI/movie/overlay routes and semantic input outcomes
need further evidence. Native source-only parity is acquired independently.

The next acquisition binds a repeatable Quick Battle interaction schedule to
observed original input calls, primary output, menu statuses and independent game
state where available. Failure comparison uses recovered original wrapper fault
schedules separately from real driver/Restore behavior; no simulated lost surface
is labeled a real live recovery.
