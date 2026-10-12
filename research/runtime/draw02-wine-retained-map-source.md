# DRAW-02 last observed map address at source submission

Step2 remains in progress. Original client-memory capture finds a single public map for several protected source texture generations before the recorder knows the canvas. A later address comparison may expose the association. Retaining an observed address after successful unmap does not establish that the driver allocation remains valid or exclusively owned.

The next experiment retains only local descriptor addresses and their resource/texture generation/subresource/map interval. At exact source submission it queries the live canvas cache, revalidates that no map epoch/open state changed and emits portable scalars. Unknown, ambiguous, stopped and changed states remain explicit. No pointer or map bytes enter a wire record. Fixtures precede original execution.

The MNMWQS01/version1 journal has a64-byte header and192-byte rows. Each
source query records exact submission/thread/API/texture generation/subresource,
last resource map call/interval/dataID/pitches/open state, query status, before/
after resource and lifetime prefixes, ticks, candidate count and a64-byte cache
point. The decoder independently validates all three original journals and
reconstructs map/lifetime state at each prefix. Changed map, changed lifetime,
ambiguous, unbound and stopped points are explicit. No map bytes are read.

The PE32 fixture passes28original forwards and10queries:5matches/5unknown,
including3closed-map matches. It covers failed remap retention, generation reuse,
real observer unmap/lifetime transitions during the outside-lock point callback
and ambiguous resource matches, with initialized FP/LastError checks. Four
decoder methods validate epoch/extent/thread/prefix/candidate/reserve/closure.

Original01 is a retained negative:161partial queries and64address matches
include canvases7/2/8/9/10/12/13. At API1688,phase2 source getter status2
refuses two pending producer frames after its single source command completed.
The current getter admits exactly one selected source-copy frame. The additional
frame's identity/overlap is not recorded, so this is not a write or lifetime
failure diagnosis. Earlier failures are not retroactively assigned this cause.

Metadata-only original02 completes1675queries:340known cache points,1027
unbound and308stopped. All340known points follow an observed unmap and cover
fullframe canvases2/7/8/9/10/13 plusUI12/14/15/22/53/54. It closes all
resource intervals,336API335EGLwitnesses and normal Quit exit0. Source byte/
readonly/worker-lease/held diagnostics are disabled in this separate run.
Address equality at this point cannot prove retained driver allocation ownership,
upload completion or whole source lifetime. Native parity was not rerun.

Next diagnose the exact additional pending producer frame at the protected byte
getter refusal before considering any admission change.
