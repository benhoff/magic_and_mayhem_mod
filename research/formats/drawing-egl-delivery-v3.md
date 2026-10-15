# Selected EGL delivery metadata v3

Version3retains the64-byte MNMX1101header and96-byte scalar record layout,
adding method11eglSwapBuffers. Arguments are process-local display ID,surface ID,
and original-function alias slot1..16; remaining arguments are zero. Original
EGLBoolean result0/1and errno are retained. The observer issues no EGL/GL query
and never consumes EGL error state. No opaque handle/function/image pointers or
original pixel bytes enter the journal. Earlier versions refuse method11.

A separate native ELF observer supports direct,named symbol and EGL proc-address
swap lookup. Sixteen exact swap/provider functions are retained; repeated names
reuse the original owner and excessive aliases refuse. Opaque display/surface
values map to at most256local diagnostic IDs by kind/equality,without holding
a recorder mutex across a driver body. These IDs deliberately do not claim
allocation generations,complete retirement or DSO/handle lifetime ownership.
Alternate swaps and RTLD_NEXT-selected functions remain unobserved boundaries.

The real Xvfb EGL fixture compares four swap lookup routes to an unchanged
control,known cleared source buffer,original Boolean/errno/EGL error and missing
symbol outcome. A separate original-active capture is required to show reached
calls. Source prefixes,actual physical changes and coherent presented journeys
remain independent of a successful swap return. The first original black display
is retained until its actual delivery/visibility behavior is independently bound.
