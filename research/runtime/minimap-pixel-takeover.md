# Minimap pixel passes

The fixed-build native DLL splices pixel passes after original preparation:

| Pass | Splice | Completion |
| --- | --- | --- |
| Terrain | `0x553a6f` | `0x5547f3` original Unlock tail |
| Cell marker | `0x553799` | `0x553847` original RET16, after native saved-register pops |
| Creature footprint | `0x553986` | `0x553a13` original marker-list increment |
| Camera0 | `0x55280b` | `0x552a9f` original marker-list preparation |
| Camera1 | `0x552bdc` | `0x552e72` original marker-list preparation |
| Camera2 | `0x552fac` | `0x553242` original marker-list preparation |
| Camera3 | `0x55337c` | `0x553612` original marker-list preparation |

The native C kernels retain four terrain orientations and signed visibility,
cell/ring order and returned pointer,13-WORD creature footprints, camera corners,
and the previously unsupported outer diamond border. Original terrain Lock,
cache invalidation and Unlock remain. Creature lookup, fog filtering, dimension
refresh and coordinate projection remain original nonpixel preparation. Camera
origin updates, RGB colour preparation and ordered marker dispatch remain;
those dispatched cell calls reach the separate native marker pixel splice.

`NativeDrawPlaneV1` resolves current process-local owned CPU surface storage by
its exact base pointer. It returns versioned dimensions/physical WORD stride,
lock state and format, rejects retired or DC-leased storage, and retains no
pointer proof between admissions. Surface registration follows allocation,
release and current flip backing. This is not a shared host-pointer wire.
Minimap pixel bounds include allocated physical padding because the original
border/footprint routines address that storage directly. No GPU history readback
or original output capture is used by these borrowed CPU pixel passes.

Terrain inputs are copied as bounded source colours/visibility before native
writes; its history is the original wrapper's currently locked native auxiliary
surface. Marker/camera inputs are bounded prepared coordinates, colours and
private request fields. Each kernel preflights every output before committing.
Failed geometry, source spans, owned-plane resolution or owner-thread admission
forward once before native pixels change. Original signatures and all affected
page protections are checked before installing any patch; no drawing diagnostic
file I/O or call budget is introduced.

The isolated original comparison passes486 terrain and2,054 overlay native
cases, including756 outer-border cases,133,986,384 physical WORDs, source/object
effects, callback counts and full caller GPR/defined flags/DF/selected FX.
Other-thread forwards and original fog/no-marker returns are separately counted.
Camera fixtures stub their terrain call to isolate the overlay body; the terrain
body is checked independently. The actual camera entry retains that separately
spliced terrain wrapper. These results confirm selected outer-border instruction
order; combined live lifecycle/publication/World timing remains pending.

A proposed per-copy fog-region admission cache was tried on2026-10-23 and removed without a causal improvement claim. The source versions, successful fake-query controls, original-reference loader refusal and subsequent live hover-readiness failure are preserved in NT17 provenance. The original per-access admission remains.
