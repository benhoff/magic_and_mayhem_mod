# Owned native byte text

`mnm-text-session` binds a validated SFT resource to an application-owned pen.
It reuses recovered byte/line suppliers; it does not replace the compatibility
font producer or Qt menu outline fonts. `mnm-font-pixels` delivers immutable
plans through CPU RGB565 pixels or a bounded GPU glyph cache.

```cpp
auto font = mnm::text::FontResource::load(file); // AssetFile already resolved
mnm::text::TextSession text(font);
auto cursor = text.cursor();
cursor.x = 12;
cursor.y = 24;
text.restore(cursor);
auto plan = text.string({'H', 'e', 'l', 'l', 'o', 0});
mnm::render::FontGpuCache cache(renderer, font->sprite());
cache.draw(destination, plan, {0, 0, width, height});
```

Retain the cache across frames to reuse glyph uploads. The renderer outlives the
cache and remains on its owner thread. CPU output uses `drawFontPixels` with the
same plan. Colors are embedded RGB565 or an explicit indexed color table;
original tinted coverage and Unicode shaping remain separate.

Build and run the bounded control under the repository storage guard:

```sh
cmake -S apps/text-session -B working/build/text -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/text --target text-session-test -j2
ctest --test-dir working/build/text -R '^native-text-session$' --output-on-failure
```

See [exact component domains and limits](../../research/runtime/rendering-twentysixth-ten-20261025.md).
