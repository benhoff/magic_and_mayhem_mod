# Native pixel producers outside resident World batches

The mapped pixel families now have independent native kernels and guarded
production routes. Original simulation, resource preparation, clipping setup,
wrapper calls and surface lifecycle remain where stated; native code computes
and stores admitted pixels. No captured original output supplies a runtime oracle or source cache. Blend
history comes from the stated current borrowed CPU compatibility storage.
Unknown writers and fallback paths prevent a claim of closed native history.

| Family | Native route |
| --- | --- |
| Glyphs | RGB565/555 RET16 kernel; unlimited capture0 production calls |
| Panels | Bevel, halo and whitewash after original Lock/preparation |
| Images/effects | PCX surface/index/WORD, RGB add, brighten, row copy and fade |
| JPEG | Source-only native RGB cache; production original decoder-call splice |
| BMP/DIB | Canonical indexed/24-bit source painted into current native DC leases |
| Minimap | Four-view terrain, cell/ring, creature, camera corners and outer border |
| Colour rectangles | Constant or triple-WORD sources, four modes, both formats |
| Rasters |17 generic/fast/slow entries,23 interior splices, original loop workspace |
| Surface copies | Native DirectDraw services plus ten native owned-DC BitBlt CALL splices |

Launch the scoped producer routes with:

```sh
MNM_OTHER_PIXEL_PRODUCERS=1 ./tools/run-qt-shell.sh --live-menus
```

The public shell’s glyph takeover defaults apply independently. Native menu
rendering and owned native drawing are required; the launcher verifies pinned
source code, imports, owned DLL/cache/source hashes and feature policy. Each
individual `MNM_PANEL_TAKEOVER`, `MNM_PIXEL_TAKEOVER`, `MNM_JPEG_TAKEOVER`,
`MNM_MINIMAP_TAKEOVER`, `MNM_COLOUR_RECT_TAKEOVER`, `MNM_RASTER_TAKEOVER` and
`MNM_DC_COPY_TAKEOVER` flag accepts0/1; an explicit individual0 overrides the
group. This group selects these scoped writers. It does not declare complete
original-renderer retirement or eliminate native admission refusals.

The source installation must contain the pinned original No-CD executable;
existing modified working executables are rejected. Supply an isolated owned
source installation through `MNM_MENU_GAME_SOURCE` when needed. Original media
is read-only. Production routes have no drawing file captures or finite call
limits; diagnostic modes remain separate.

The original comparison fixtures cover pixel arithmetic, clipping, physical
padding/guards and specified caller/workspace state. They do not prove complete
presented journeys, every simulation-generated source, cross-thread mutation,
arbitrary aliases or driver state. CPU consumers use admitted current borrowed
compatibility storage; these routes do not read back GPU World history. Combined
resident frame ordering, active battle/effect/menu transitions and full live
replacement remain independent integration milestones. Unknown/unregistered
writers remain visible in the coverage inventory.

See [image/effect](image-effect-pixel-takeover.md), [JPEG](native-jpeg-pixels.md),
[glyph](glyph-production-takeover.md), [minimap](minimap-pixel-takeover.md),
[colour rectangles](colour-rectangle-takeover.md), [raster](outside-world-raster-pixels.md)
and [DC copy](native-dc-copy.md) contracts and their immutable evidence records.
