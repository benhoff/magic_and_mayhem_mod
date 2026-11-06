# Camera source-generation/output closeout — 2026-11-06

This closes a finite prerequisite in the existing default-profile 800×600 Quick
gameplay capture: camera orientation and minimap center are now bound to the
exact owned source generation and its independently compared native output.
It adds no game fixture, native implementation, capture or original-game run.
The acceptance inventory stays **38/156 passed, 118 pending**. Both final
decisions and the full camera requirement rows remain pending.

## Evidence and confidence

`OC06.source-camera-output-closed01` records a new analysis of the immutable
GA05 acquisition/comparison already admitted by WX06. The current MR06/WX06
parents retain their existing source-applicability boundaries. All earlier
behavior, evidence and scenario definitions stay unchanged; the old gameplay
report keeps its historical shared-test fingerprint.

The original producer packets already contain the minimap's orientation and
center at the actual terrain/camera/cell/creature calls. These are owned source
arguments, unlike the periodic HUD/semantic observations. The new analysis:

1. Verifies retained bytes and decodes every source generation and prefix chunk.
2. Requires terrain and all three overlays to agree on orientation, center and
   destination, with the captured camera/cell/creature ordering.
3. Checks the original selector's four cases against its three camera bodies:
   views 0, 1, 2, 3 select bodies 0, 2, 0, 1. A fourth body is not required.
4. Binds generation numbers to native import surface incarnations and command
   intervals before deletion, then joins every actual World swap occurrence to
   the matching retained original comparison and exact decoded viewport digest.
5. Requires three distinct settled source generations for each selected action,
   with nonempty creature markers and captured indexed, clipped, shadow and
   displacement raster routes. Source input identity and final output equality
   are checked together; raster counts alone are insufficient.

The six action windows show views **0→3→2→1→0**, then an Up pan with view 0
unchanged and source map center **[37,65]→[35,63]**. The complete retained history
has 191 source generations and 1,020 World swap occurrences, covering
1,027,751,340 already compared viewport pixels with zero differences. This
pixel count selects actual World swaps; the older acquisition's larger total
also includes transition output. Repeated swaps are retained as occurrences;
repeated redraws cannot substitute for three advancing source generations.

71 focused checks pass: 20 controls over actual retained source/action/output
data, 12 existing gameplay checks, 30 UI checks and 9 client checks. The controls
reject wrong source orientation/center/body/order, missing scene/marker routes,
redraw-only progress, missing/duplicate swaps, wrong lease/source/pixels, unowned
GPU output and dependent history. The initial transition-row control mistake
and its corrected execution remain separate immutable records.

Confidence is high within that finite source/action/output join. The full raw
producer history and accepted comparison remain the inputs; the compact derived
report is not a replacement capture. New reports, controls, development failures
and accounting are preserved through SQLite and the verified local object pair.

## Boundaries and next work

This does not bind every periodic camera/HUD/semantic scalar to a source frame.
It does not recover camera-world coordinates from the minimap center, prove
picking/HUD alignment, whole minimap semantics, actor animation/combat or full
simulation pause. The initial prefix without minimap context supplies no camera
witness. No individual intermediate raster output or original ABI result is
newly compared by this analysis.

The full camera rows still need the software and 640×480 gameplay domains,
viewport limits and picking/HUD alignment. Reuse this source-generation join
when extending the existing gameplay driver; another fixture or sampler is not
needed for orientation/minimap-center identity. Preserve the known software640
pre-Save admission failure, historical capture failures and both final decisions.

## Reproduce

Use new immutable report names and the local verified evidence stores:

```sh
python3 -B tools/close-rendering-source-camera.py \
  --report research/runtime/rendering-source-camera-results-20261106/NEW.json
MNM_SOURCE_CAMERA_RESULT=research/runtime/rendering-source-camera-results-20261106/NEW.json \
  python3 -B tests/test-rendering-source-camera-closeout.py -f
```

The source reader uses one bounded temporary `worlds.bin` interchange file under
ignored `working/`, removing it after analysis. It does not restore a whole
execution tree or stage another installation, prefix or build.
