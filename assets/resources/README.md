# Native visual resource ownership

`mnm-resources` binds stable semantic IDs to explicit visual recipes and owns
decoded input. `mnm-resource-renderer` separately owns their OpenGL uploads.
Both are reusable services without widget, legacy-pointer or build-specific
reconstruction dependencies. This is the resource-ownership milestone; it does
not connect the live original simulation to native scene composition.

## Bind and draw

```cpp
// Inspect the AssetStore::create result for an assets::Error first.
assets::ResourceManager resources(std::move(store));
const assets::ResourceId redcap{assets::ResourceKind::creature, "10"};
resources.bind(redcap, {assets::ResourceImageFormat::sprite,
    "Creatures/RedCap.spr", "Creatures/redcap.ani", {}, {}});

render::GlBlitter renderer;
render::ResourceCache uploads(renderer, resources);
uploads.draw(redcap, frameIndex, destination, anchorX, anchorY);
uploads.retire(redcap); // Release both GPU and decoded ownership; keep binding.
```

IDs contain a kind and a caller-assigned canonical lowercase ASCII name. Their
text forms, such as `creature:10`, `terrain:celtic/forest`, `effect:36` and
`ui:menu/title`, are independent of insertion order, file spelling and memory
addresses. They are native policy, not original numeric/pointer identities or an
implemented wire protocol. A caller must maintain the semantic mapping.

Binding is lazy and immutable. Binding the identical recipe again is harmless;
changing a recipe under an existing ID fails. Create a new manager/session to
change bindings. Creature/effect recipes require paired SPR/ANI; terrain recipes
require SPR/TTD; UI recipes accept SPR (optionally ANI), BMP, PCX or JPEG. The
image format is explicit, not inferred from its filename. An optional sequence
binds an ANI sequence index; it neither selects a named action nor advances it.

A checkpoint can instead supply value-owned `animationBytes`, mutually exclusive
with the animation path. Binding copies those bytes and preserves them across
decoded unload/reload. They use the same bounded ANI decoder and paired bitmap
checks. Immutable encoded ANI vector capacities have a separate 8 MiB aggregate
default `recipeBytes` budget; each input also obeys the ANI loader's input limit.
Repeated identical bindings do not charge twice. Encoded recipes stay resident
until manager destruction, independently of the decoded resource budget.

Loading resolves every path through the read-only AssetStore, closes all files,
and publishes one complete owned resource. It checks every ANI bitmap record
against the paired SPR and any selected sequence against the ANI table. Missing,
malformed, mismatched and over-budget resources throw exceptions with the input
path where applicable. TTD records remain opaque; catalog-to-SPR rendering field
interpretation belongs to reconstruction/scene composition.

## Budgets and lifetime

Default manager budgets are 1,024 immutable bindings, 64 resident decoded
resources and 128 MiB of resident object/vector storage. Byte accounting charges
vector **capacities**, palettes, masks, auxiliary planes, ANI records/offsets and
TTD records. It excludes allocator/map/string bookkeeping and temporary decoder
input/scratch; those remain bounded by the existing per-format loader limits.
TTD retains its loader's 65,536-record bound. Failed loading leaves residency,
successful-load counters and revisions unchanged. There is no implicit CPU
eviction: `unload(id)`/`unloadAll()` explicitly release it. Returned references
expire on those operations or manager destruction.

Each successful load receives a manager-local monotonically increasing revision.
Retirement preserves the ID/recipe and invalidates old decoded references. The
upload cache recognizes a new revision and removes the ID's old uploads on its
next draw. `retire(id)` immediately releases both layers; `release(id)`/`clear()`
release GPU ownership alone. External file changes are observed only after
explicit decoded retirement; hot reloading/file watching is outside scope.

The default GPU cache retains at most 24 frame/colour-table variants and 8 Mi
colour-plus-coverage texels. Two nonempty surfaces are charged per upload; empty
frames occupy a cache slot without textures. LRU eviction respects both its own
limits and the renderer's shared 64-surface/16 Mi-texel limits, including surfaces
owned by other callers. Impossible admissions fail before cache eviction. Later
allocation failure may leave previous LRU evictions in effect but publishes no
partial upload. Hits draw without uploading or reading pixels back. Palette
override keys copy all 256 RGB565 words, so identical tables share a variant and
mutating a table at the same address cannot reuse stale colours.

SPR retains signed origins, embedded palettes and transparent coverage. BMP/JPEG
use an explicitly unshaded RGB565 conversion; PCX uses its indexed palette. All
bitmap pixels are opaque, including black. Bitmap origins are zero. Drawing can
use existing destination clipping. Original lighting, palette animation, effects,
font/cursor policies and named animation actions remain separate contracts.

Use the manager on its constructing thread; it checks access. Create/use/destroy
the renderer/cache on the Qt GUI thread, with the manager and renderer outliving
the cache. Neither service exposes texture pointers as resource identities.

## Build and validation

```sh
cmake -S renderer/resources -B working/build/resources -DCMAKE_BUILD_TYPE=Debug
cmake --build working/build/resources --target \
  resource-manager-test resource-cache-test mnm-resource-preview -j4
ctest --test-dir working/build/resources -R '^native-resource-' --output-on-failure

# Fixture checks + relevant loader/sprite/renderer regressions:
python3 tools/test-native-resources.py
# Add installed Redcap recipe smoke, with original-manifest guards:
python3 tools/test-native-resources.py --root working/game-clean
```

The manager also builds independently with `cmake -S assets/resources -B
working/build/resource-manager`. Its fixture tests need no display or game.
GPU checks require Xvfb/OpenGL. The runner retains new logs, source/binary hashes
and a report under `working/tests/native-resources/run-*/`; a concurrent source
change refuses a current-source success claim.

Preview an explicit installed recipe without launching the game:

```sh
./tools/original-manifest.sh verify
xvfb-run -a env QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  working/build/resources/mnm-resource-preview \
  --root working/game-clean --kind creature --id 10 \
  --image Creatures/RedCap.spr --animation Creatures/redcap.ani \
  --frame 0 --output working/resource-redcap.png
./tools/original-manifest.sh verify
```

The preview draws twice to verify upload reuse, then retires decoded/GPU data.
It requires a new output outside the asset root. It displays a selected bitmap;
ANI scheduling, automatic installed recipe population and live replacement are
not supplied. See [scope/evidence](../../research/runtime/native-resource-manager.md).
