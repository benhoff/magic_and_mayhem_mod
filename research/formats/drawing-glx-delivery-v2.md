# Selected GLX delivery metadata v2

Version2uses the same MNMX1101/64-byte header and96-byte scalar records as the
selected X11 v1protocol,adding method10glXSwapBuffers. Its arguments contain
local drawable ID and original-function alias slot1..16; all other argument
words and the void result are zero. Strict v1admission refuses method10.
Original GL/image bytes and process pointers never enter the journal.

A separate64-bit ELF library wraps direct swaps,known named dlsym lookups and
both GLX proc-address lookups. Each resolved swap/provider alias retains its
exact original function. Sixteen swap aliases and sixteen aliases for each
proc-address name are bounded native policies. Repeated lookups keep the same
original owner; exhaustion refuses before the seventeenth original dispatch.
Function-pointer identity is intentionally wrapped,not a baseline loader-state
claim. Already wrapped functions are recognized to avoid duplicate observation.

Assembly tail forwarding preserves the original caller address for unwrapped
and RTLD_NEXT dlsym calls. Selected GL resolved through RTLD_NEXT remains an
explicit unobserved route. A previously loaded local Xext/X11 owner can supply
an original symbol absent from the global lookup scope via RTLD_NOLOAD. This
never loads a new graphics library; loader error-state equivalence remains
outside scope. No observer X11/GL call or recorder lock spans an original body.

Actual GLX forwarding tests separate known cleared back-buffer source from
observed front-buffer result; the latter is compared to the unchanged control,
not assumed to have reached the display. Five lookup/swap routes,original errno,
known source colours and missing-symbol outcome pass. Independent fake providers
verify sixteen exact aliases/repeated lookups and swap/proc exhaustion. These
are separate from original game/physical causality or all-route proof.

Original GLX observation01retains619API physical witnesses but zero ordinary
GLX swaps. The selected X11 calls remain457closed observations. Dynamic alternate
GL/EGL/Vulkan routes,library scope and actual observer presence in the game
process require investigation. Zero reached swaps is an open result,not a proved
backend or a complete presentation record.
