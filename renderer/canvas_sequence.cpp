#include "canvas_sequence.hpp"
#include "../assets/sprite_run_clipping.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mnm::render {
namespace {
std::size_t slot(const Image &image, int x, int y) {
  return std::size_t(y) * image.width + x;
}
void known(const std::vector<unsigned char> &defined, std::size_t at) {
  if (defined.at(at)!=1)
    throw std::runtime_error("Native producer reads undefined canvas pixels");
}
void frame(const assets::SpriteFrame &f) {
  if (f.width > 2048 || f.height > 2048 ||
      f.opaqueMask.size() != std::size_t(f.width) * f.height)
    throw std::invalid_argument("Invalid native producer frame");
  std::visit(
      [&](const auto &p) {
        if (p.size() != f.opaqueMask.size())
          throw std::invalid_argument("Invalid native producer frame plane");
      },
      f.pixels);
}
} // namespace
CanvasSequence::Surface &CanvasSequence::surface(std::uint32_t id) {
  auto it = surfaces_.find(id);
  if (it == surfaces_.end())
    throw std::out_of_range("Unknown native producer canvas");
  return it->second;
}
void CanvasSequence::create(std::uint32_t id, int width, int height) {
  if (!id || surfaces_.count(id) || width <= 0 || height <= 0 || width > 2048 ||
      height > 2048 || surfaces_.size() >= 64 ||
      std::size_t(width) * height > 16777216 - pixels_)
    throw std::invalid_argument("Native producer canvas admission failed");
  const auto count = std::size_t(width) * height;
  surfaces_.emplace(id,
                    Surface{{width, height, std::vector<std::uint32_t>(count)},
                            std::vector<unsigned char>(count), {}, {}});
  pixels_ += count;
  if (mirror_) {
    auto &s = surface(id);
    s.dirtyFirst.assign(height, width); s.dirtyEnd.assign(height, 0);
    mirror_->create(id, width, height);
  }
}
void CanvasSequence::discardCpu(std::uint32_t id) {
  if(!mirror_)throw std::logic_error("CPU knowledge discard requires a GPU mirror");
  auto& s=surface(id);publishWrites();for(auto& state:s.defined)if(state)state=2;
}
void CanvasSequence::requireCpu(std::uint32_t id,Surface& s,std::size_t at) {
  if(s.defined.at(at)==2 && mirror_){
    publishWrites();auto image=mirror_->readForCpu(id);
    if(image.width!=s.image.width || image.height!=s.image.height || image.pixels.size()!=s.image.pixels.size() ||
       std::any_of(image.pixels.begin(),image.pixels.end(),[](auto p){return p>65535;}))
      throw std::runtime_error("Invalid native CPU reader snapshot");
    // Only previously established GPU history can become CPU authority.
    // Reading allocation bytes must never define an unknown pixel.
    for(std::size_t i=0;i<s.defined.size();++i)if(s.defined[i]){s.image.pixels[i]=image.pixels[i];s.defined[i]=1;}
  }
  known(s.defined,at);
}
std::size_t CanvasSequence::minimap(std::uint32_t id, int stride,
    const std::function<std::size_t(MinimapPlane &)> &draw, bool tracedServices) {
  auto &s = surface(id);
  if (stride < s.image.width || stride > 4096)
    throw std::invalid_argument("Invalid captured minimap stride");
  // Two independent owned backgrounds identify every written word, including
  // writes equal to either sentinel. No destination oracle enters replay.
  MinimapPlane a{s.image.width, s.image.height, stride, {}};
  MinimapPlane b{s.image.width, s.image.height, stride, {}};
  std::vector<std::size_t> firstWrites, secondWrites;
  if (tracedServices) {
    a.words.swap(minimapWords_[0]); b.words.swap(minimapWords_[1]);
    firstWrites.swap(minimapCandidates_[0]); secondWrites.swap(minimapCandidates_[1]);
    a.writeCandidates = &firstWrites; b.writeCandidates = &secondWrites;
  }
  // Detached local buffers preserve independent storage even if a callback
  // reenters. Generic callbacks always receive freshly allocated backgrounds.
  struct Recycle {
    bool active;
    std::array<std::vector<std::uint16_t>, 2>& words;
    std::array<std::vector<std::size_t>, 2>& candidates;
    MinimapPlane& a; MinimapPlane& b;
    std::vector<std::size_t>& first; std::vector<std::size_t>& second;
    ~Recycle() {
      if (!active) return;
      // Also restore admitted writes when a callback/validation throws. Never
      // dereference a malformed receipt while unwinding a rejected operation.
      for (const auto at : first) if (at < a.words.size()) a.words[at] = 0x1357;
      for (const auto at : second) if (at < b.words.size()) b.words[at] = 0xeca8;
      a.words.swap(words[0]); b.words.swap(words[1]);
      first.clear(); second.clear();
      first.swap(candidates[0]); second.swap(candidates[1]);
    }
  } recycle{tracedServices, minimapWords_, minimapCandidates_, a, b, firstWrites, secondWrites};
  const auto count = std::size_t(stride) * s.image.height;
  a.words.resize(count, 0x1357); b.words.resize(count, 0xeca8);
  const auto result = draw(a); if (draw(b) != result)
    throw std::logic_error("Minimap result depends on diagnostic background");
  if (tracedServices) {
    // Both fresh sentinels remain distinct at every untouched word. Only the
    // union of the recovered services' committed writes can compare equal.
    // Generic callbacks keep the full scan because their writes are untraced.
    // Validate both lists before canvas mutation. Repeated final equal-word
    // assignments are idempotent; ordering/deduplication need no sort. Known
    // services usually return identical receipts for the two backgrounds.
    const auto outside = [count](std::size_t at) { return at >= count; };
    if (a.words.size() != count || b.words.size() != count ||
        std::any_of(firstWrites.begin(), firstWrites.end(), outside) ||
        std::any_of(secondWrites.begin(), secondWrites.end(), outside))
      throw std::invalid_argument("Minimap candidate outside owned storage");
    if (firstWrites != secondWrites)
      firstWrites.insert(firstWrites.end(), secondWrites.begin(), secondWrites.end());
    for (const auto from : firstWrites) {
      const auto y = from / std::size_t(stride), x = from % std::size_t(stride);
      if (x >= std::size_t(s.image.width)) continue;
      const auto to = y * s.image.width + x;
      if (a.words[from] == b.words[from]) { write(s, to, a.words[from]); s.defined[to] = 1; }
    }
  } else {
    for (int y = 0; y < s.image.height; ++y)
      for (int x = 0; x < s.image.width; ++x) {
        const auto from = std::size_t(y) * stride + x, to = std::size_t(y) * s.image.width + x;
        if (a.words[from] == b.words[from]) { write(s, to, a.words[from]); s.defined[to] = 1; }
      }
  }
  return result;
}
void CanvasSequence::terrainMap(std::uint32_t id, int rootX, int rootY, int w, int h, int centerX, int centerY, const std::vector<std::uint16_t> &colours, const std::vector<unsigned char> &hidden, std::uint32_t previous) {
  auto &s = surface(id);
  if (w <= 0 || w > 256 || w % 2 || h <= 0 || h > 256 || colours.size() != std::size_t(w) * h ||
      std::int64_t(rootX) - 2 * ((h - 1) / 2) < 0 || std::int64_t(rootX) + w > s.image.width || rootY < 0 ||
      std::int64_t(rootY) + h / 2 + (w - 1) / 2 >= s.image.height)
    throw std::invalid_argument("Minimap terrain outside owned destination");
  if (!hidden.empty() && hidden.size() != colours.size()) throw std::invalid_argument("Unclosed minimap visibility plane");
  const auto old = previous ? read(previous) : Image{};
  if (previous && (old.width < 2 * w || old.height < (h + w) / 2)) throw std::invalid_argument("Minimap previous canvas extent");
  const auto wrap = [](std::int64_t n, int size) { return int((n % size + size) % size); };
  const int startX = wrap(std::int64_t(centerX) - w / 2, w), startY = wrap(std::int64_t(centerY) - h / 2, h);
  // Resolve all projected history reads before defining destination pixels.
  for (int row = 0; row < h; ++row)
    for (int col = 0; col < w; ++col) {
      const auto cell = std::size_t((startY + row) % h) * w + (startX + col) % w;
      if (!hidden.empty() && hidden[cell]) {
        const int sourceX = w - 1 - 2 * (row / 2) + col,
                  sourceY = (row + 1) / 2 + col / 2;
        if (!previous || sourceX < 0 || sourceY < 0 || sourceX >= old.width || sourceY >= old.height)
          throw std::invalid_argument("Minimap hidden cell lacks owned projected history");
      }
    }
  for (int row = 0; row < h; ++row)
    for (int col = 0; col < w; ++col) {
      const auto at = slot(s.image, rootX - 2 * (row / 2) + col, rootY + (row + 1) / 2 + col / 2);
      const auto cell = std::size_t((startY + row) % h) * w + (startX + col) % w;
      if (!hidden.empty() && hidden.at(cell)) {
        if (!previous) throw std::invalid_argument("Minimap hidden cell lacks owned history");
        write(s, at, old.pixels.at(slot(old, w - 1 - 2 * (row / 2) + col, (row + 1) / 2 + col / 2)));
      } else write(s, at, colours.at(cell));
      s.defined[at] = 1;
    }
}
void CanvasSequence::fade(std::uint32_t id,unsigned stride,
                          std::optional<std::size_t> physicalWords,std::uint16_t mask) {
  auto &s = surface(id);
  if(!stride)stride=s.image.width;
  const auto count=physicalWords.value_or(s.image.pixels.size());
  if(stride<unsigned(s.image.width) || stride>4096 || count>std::size_t(stride)*s.image.height)
    throw std::invalid_argument("Fade span outside canvas storage");
  // Padding has no cross-WORD influence after the original channel mask. Only
  // visible pixels in the contiguous physical span require destination reads.
  for(int y=0;y<s.image.height && std::size_t(y)*stride<count;++y) {
    const auto words=std::min(std::size_t(s.image.width),count-std::size_t(y)*stride);
    const auto row=std::size_t(y)*s.image.width;
    for(std::size_t x=0;x<words;++x)requireCpu(id,s,row+x);
  }
  // Validation above is complete before any mutation. A physical prefix covers
  // one visible prefix per row; track that damage once, without per-WORD div/mod.
  for(int y=0;y<s.image.height && std::size_t(y)*stride<count;++y) {
    const auto words=std::min(std::size_t(s.image.width),count-std::size_t(y)*stride);
    const auto row=std::size_t(y)*s.image.width;
    for(std::size_t x=0;x<words;++x) {
      auto &p=s.image.pixels[row+x];p=(std::uint16_t(p)>>1)&mask;
    }
    dirty(s,0,y,int(words),1);
  }
}
void CanvasSequence::release(std::uint32_t id) {
  auto &s = surface(id);
  pixels_ -= s.image.pixels.size();
  if (mirror_) mirror_->release(id);
  surfaces_.erase(id);
}
void CanvasSequence::addRgb(std::uint32_t id, Rect r,
                            std::array<unsigned, 3> add, bool invalidateOnly) {
  auto &s = surface(id);
  if (!invalidateOnly && (add[0] > 31 || add[1] > 63 || add[2] > 31))
    throw std::invalid_argument(
        "RGB addition outside recovered unsigned channels");
  for (int y = std::max(0, r.top); y < std::min(s.image.height, r.bottom); ++y)
    for (int x = std::max(0, r.left); x < std::min(s.image.width, r.right); ++x)
      if(!invalidateOnly)requireCpu(id,s,slot(s.image,x,y));
  for (int y = std::max(0, r.top); y < std::min(s.image.height, r.bottom); ++y)
    for (int x = std::max(0, r.left); x < std::min(s.image.width, r.right);
         ++x) {
      const auto at = slot(s.image, x, y);
      if(invalidateOnly){s.defined[at]=0;continue;}
      const auto old = s.image.pixels[at];
      write(s, at, (std::min(31u, ((old >> 11) & 31) + add[0]) << 11) |
                           (std::min(63u, ((old >> 5) & 63) + add[1]) << 5) |
                           std::min(31u, (old & 31) + add[2]));
    }
}
void CanvasSequence::panel(std::uint32_t id, Rect r, unsigned mode,
                           unsigned percent, bool pressed, bool reverse) {
  auto &s = surface(id);
  if (mode < 15 || mode > 17 || percent > 1000 || r.left < 0 || r.top < 0 ||
      r.right < r.left || r.bottom < r.top ||
      (mode == 17 ? r.right > s.image.width || r.bottom > s.image.height
                  : r.right >= s.image.width || r.bottom >= s.image.height))
    throw std::invalid_argument("Panel writer outside recovered owned extent");
  if (mode == 15 && (r.right + 1 >= s.image.width ||
                     r.bottom + 1 >= s.image.height))
    throw std::invalid_argument(
        "Bevel's inclusive extra row/column escaped owned extent");
  // Validate the complete ordered footprint before the first destination read
  // can mutate storage. Overlapping corners still receive each original pass.
  auto visit = [&](const auto &pixel) {
    if (mode == 17) {
      for (int y = r.top; y < r.bottom; ++y)
        for (int x = r.left; x < r.right; ++x) pixel(x, y, 0);
    } else if (mode == 16) {
      for (int x = r.left; x < r.right; ++x) pixel(x, r.top, 0);
      for (int x = r.left + 1; x <= r.right; ++x) pixel(x, r.bottom, 0);
      for (int y = r.top + 1; y <= r.bottom; ++y) pixel(r.left, y, 0);
      for (int y = r.top; y < r.bottom; ++y) pixel(r.right, y, 0);
    } else {
      const int right = r.right + 1, bottom = r.bottom + 1,
                sign = pressed ? 1 : -1;
      const int rows[4] = {r.top, r.top + 1, bottom, bottom - 1},
                cols[4] = {r.left, r.left + 1, right, right - 1};
      for (unsigned j = 0; j < 4; ++j) {
        const int inset = j >= 2 ? 2 : 0,
                  direction = j >= 2 && reverse ? -sign : sign;
        for (int x = r.left + inset; x < right; ++x)
          pixel(x, rows[j], direction);
      }
      for (unsigned j = 0; j < 4; ++j) {
        const int inset = j < 2 ? 2 : 0,
                  direction = j >= 2 && reverse ? -sign : sign;
        for (int y = r.top + inset; y < bottom; ++y)
          pixel(cols[j], y, direction);
      }
    }
  };
  visit([&](int x, int y, int) { requireCpu(id,s,slot(s.image,x,y)); });
  auto pixel = [&](int x, int y, int sign) {
    const auto at = slot(s.image, x, y);
    const auto old = s.image.pixels[at];
    unsigned c[3] = {((old >> 11) & 31) << 3, ((old >> 5) & 63) << 2,
                     (old & 31) << 3};
    for (auto &v : c) {
      if (mode == 15)
        v = unsigned(
            std::clamp(int(v) + sign * int(v * percent / 100), 0, 255));
      else
        v = (v >> 1) + (128u);
    }
    write(s, at, ((c[0] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[2] >> 3));
  };
  visit(pixel);
}
void CanvasSequence::fill(std::uint32_t id, Rect r, std::uint16_t colour) {
  auto &s = surface(id);
  if (r.right < r.left || r.bottom < r.top)
    throw std::invalid_argument("Inverted producer fill");
  for (int y = std::max(0, r.top); y < std::min(s.image.height, r.bottom); ++y)
    for (int x = std::max(0, r.left); x < std::min(s.image.width, r.right);
         ++x) {
      const auto at = slot(s.image, x, y);
      write(s, at, colour);
      s.defined[at] = 1;
    }
}
void CanvasSequence::copy(std::uint32_t source, std::uint32_t destination,
                          Rect r, int x, int y,
                          std::optional<std::uint16_t> key) {
  const auto &original = surface(source);
  auto &dst = surface(destination);
  if (r.left < 0 || r.top < 0 || r.right < r.left || r.bottom < r.top ||
      r.right > original.image.width || r.bottom > original.image.height)
    throw std::invalid_argument("Invalid producer copy source");
  const auto left=std::max<std::int64_t>(0,-std::int64_t(x)),top=std::max<std::int64_t>(0,-std::int64_t(y));
  const auto right=std::min<std::int64_t>(r.right-r.left,std::int64_t(dst.image.width)-x),bottom=std::min<std::int64_t>(r.bottom-r.top,std::int64_t(dst.image.height)-y);
  if(left>=right||top>=bottom)return;
  const int sx=int(r.left+left),sy=int(r.top+top),dx=int(std::int64_t(x)+left),dy=int(std::int64_t(y)+top),width=int(right-left),height=int(bottom-top);
  // Validate every sampled source before any destination write. Same-ID copies
  // retain the source snapshot, matching ordered owned-storage copy semantics.
  for(int row=0;row<height;++row){const auto at=slot(original.image,sx,sy+row);const auto begin=original.defined.begin()+at;
    if(!mirror_ && !deferCopyValidity_ && std::any_of(begin,begin+width,[](auto state){return state!=1;}))throw std::runtime_error("Native producer reads undefined canvas pixels");}
  // Distinct IDs own distinct vectors. Only same-ID copies need a snapshot;
  // preflight above still completes before allocation or any destination write.
  if (mirror_) { publishWrites(); mirror_->copy(source, destination, {sx, sy, sx+width, sy+height}, dx, dy, key); }
  std::optional<Surface> snapshot;if(source==destination)snapshot.emplace(original);
  const auto& src=snapshot?*snapshot:original;
  for(int row=0;row<height;++row){const auto from=slot(src.image,sx,sy+row),to=slot(dst.image,dx,dy+row);
    if(!key){std::copy_n(src.image.pixels.begin()+from,width,dst.image.pixels.begin()+to);std::copy_n(src.defined.begin()+from,width,dst.defined.begin()+to);}
    else for(int col=0;col<width;++col){const auto word=src.image.pixels[from+col];if(src.defined[from+col]!=1){dst.defined[to+col]=src.defined[from+col]&&dst.defined[to+col]?2:0;}else if(word!=*key){dst.image.pixels[to+col]=word;dst.defined[to+col]=1;}}
  }
}
void CanvasSequence::update(std::uint32_t id, int x, int y,
                            const Image &image) {
  auto &s = surface(id);
  if (image.width < 0 || image.height < 0 ||
      image.pixels.size() != std::size_t(image.width) * image.height || x < 0 ||
      y < 0 || std::int64_t(x) + image.width > s.image.width ||
      std::int64_t(y) + image.height > s.image.height)
    throw std::invalid_argument("Invalid producer source update");
  for (auto word : image.pixels)
    if (word > 65535)
      throw std::invalid_argument("Producer source word exceeds RGB565");
  // All extents and WORDs are checked before the first write.
  dirty(s, x, y, image.width, image.height);
  for (int j = 0; j < image.height; ++j) {
    const auto from = std::size_t(j) * image.width;
    const auto to = std::size_t(y + j) * s.image.width + x;
    std::copy_n(image.pixels.begin() + from, image.width,
                s.image.pixels.begin() + to);
    std::fill_n(s.defined.begin() + to, image.width, 1);
  }
}
void CanvasSequence::glyph(std::uint32_t id, const assets::SpriteFrame &f,
                           int x, int y, Rect clip, std::array<unsigned, 3> rgb,
                           const std::array<float, 64> &coverage) {
  frame(f);
  auto &s = surface(id);
  const auto *alpha = std::get_if<std::vector<std::uint8_t>>(&f.pixels);
  if (!alpha)
    throw std::invalid_argument("Font requires indexed coverage");
  for (auto v : rgb)
    if (v > 255)
      throw std::invalid_argument("Font tint outside byte range");
  for (float c : coverage)
    if (!std::isfinite(c) || c < 0 || c > 1)
      throw std::invalid_argument("Font coverage outside unit range");
  // Reject malformed owned inputs before any destination mutation, including
  // invalid indices in rows which this particular draw would crop away.
  for (std::size_t i = 0; i < f.opaqueMask.size(); ++i)
    if (f.opaqueMask[i] && alpha->at(i) >= 64)
      throw std::invalid_argument("Font coverage index exceeds initialized table");
  const auto left = std::int64_t(x) - f.originX,
             top = std::int64_t(y) - f.originY;
  // Original581ec0 refuses horizontal clipping; vertical rows are cropped.
  if (left < clip.left || left + f.width > clip.right)
    return;
  // Coverage reads the destination even at weight zero/one. Preflight every
  // visible sample so a late undefined pixel cannot leave a partial glyph.
  for (unsigned j = 0; j < f.height; ++j)
    for (unsigned i = 0; i < f.width; ++i) {
      const auto dx = left + i, dy = top + j;
      if (dx >= 0 && dy >= 0 && dx < s.image.width && dy < s.image.height &&
          dy >= clip.top && dy < clip.bottom && f.opaqueMask[std::size_t(j) * f.width + i])
        requireCpu(id,s,slot(s.image,int(dx),int(dy)));
    }
  const unsigned target[3] = {rgb[0] >> 3, rgb[1] >> 2, rgb[2] >> 3};
  for (unsigned j = 0; j < f.height; ++j)
    for (unsigned i = 0; i < f.width; ++i) {
      const auto dx = left + i, dy = top + j;
      if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height ||
          dy < clip.top || dy >= clip.bottom)
        continue;
      const auto from = std::size_t(j) * f.width + i;
      if (!f.opaqueMask[from])
        continue;
      const auto a = alpha->at(from);
      const auto at = slot(s.image, int(dx), int(dy));
      const auto old = s.image.pixels[at];
      const double weight = coverage[a];
      const unsigned d[3] = {(old >> 11) & 31, (old >> 5) & 63, old & 31};
      unsigned channel[3];
      for (unsigned k = 0; k < 3; ++k)
        channel[k] = unsigned(d[k] * (1 - weight) + target[k] * weight);
      write(s, at, (channel[0] << 11) | (channel[1] << 5) | channel[2]);
    }
}
void CanvasSequence::raster(std::uint32_t id, const assets::SpriteFrame &f,
                            int x, int y, Rect clip, unsigned mode, int backend,
                            const std::array<std::uint16_t, 256> &colours,
                            const std::array<int, 16> &offsets,
                            unsigned period, bool invalidateOnly) {
  frame(f);
  auto &s = surface(id);
  if (mode > 5 || !period || period > 16)
    throw std::invalid_argument("Invalid producer raster mode");
  const auto left = std::int64_t(x) - f.originX,
             top = std::int64_t(y) - f.originY;
  std::vector<std::uint8_t> clippedCoverage;
  const auto* coverage=&f.opaqueMask;
  if(mode==3 && backend==7 && (left<clip.left || left+f.width>clip.right)){
    clippedCoverage.assign(f.opaqueMask.size(),0);
    for(const auto& run:assets::clipWholeOpaqueRuns(f,std::int64_t(clip.left)-left,std::int64_t(clip.right)-left))
      std::fill(clippedCoverage.begin()+std::size_t(run.row)*f.width+run.left,
                clippedCoverage.begin()+std::size_t(run.row)*f.width+run.right,1);
    coverage=&clippedCoverage;
  }
  if(backend==11){
    if(mode!=0 || !std::holds_alternative<std::vector<std::uint16_t>>(f.pixels))throw std::invalid_argument("Grayscale backend mode or source");
    if(left>=clip.left && left+f.width<=clip.right)mode=6;
  }
  if ((backend == 0 || backend == 9) &&
      (left < clip.left || left + f.width >= clip.right))
    return;
  if (mode == 5) {
    const auto shadowTop = top + f.height / 2;
    if (left < clip.left || left + f.width + f.height / 2 > clip.right ||
        shadowTop >= clip.bottom || shadowTop + f.height <= clip.top)
      return;
    const int first = int(std::max<std::int64_t>(0, clip.top - shadowTop)),
              last = int(
                  std::min<std::int64_t>(f.height, clip.bottom - shadowTop));
    auto visit = [&](const auto &pixel) {
    for (int row = 0; row < (last - first + 1) / 2; ++row)
      for (unsigned col = 0; col < f.width; ++col) {
        if (!coverage->at(std::size_t(first + row * 2) * f.width + col))
          continue;
        const auto dx = left + col + last / 2 - row,
                   dy = shadowTop + first + row;
        if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height)
          throw std::invalid_argument(
              "Projected shadow sample outside owned canvas");
        const auto at = slot(s.image, int(dx), int(dy));
        pixel(at);
      }
    };
    // Geometry and every required read are checked before CPU or mirror damage.
    visit([&](std::size_t at) { if (!invalidateOnly) requireCpu(id,s,at); });
    visit([&](std::size_t at) {
      if (invalidateOnly) s.defined[at] = 0;
      else write(s, at, (s.image.pixels[at] >> 1) & 0x7bef);
    });
    return;
  }
  if (invalidateOnly) {
    // Crop once. CPU validity changes only at visible opaque samples; no CPU
    // World pixels are synthesized and transparent history remains known.
    const auto firstX = std::max<std::int64_t>({0, -left, std::int64_t(clip.left) - left});
    const auto lastX = std::min<std::int64_t>({f.width, std::int64_t(s.image.width) - left, std::int64_t(clip.right) - left});
    const auto firstY = std::max<std::int64_t>({0, -top, std::int64_t(clip.top) - top});
    const auto lastY = std::min<std::int64_t>({f.height, std::int64_t(s.image.height) - top, std::int64_t(clip.bottom) - top});
    if (firstX >= lastX || firstY >= lastY) return;
    const auto count = std::size_t(lastX - firstX);
    for (auto row = firstY; row < lastY; ++row) {
      auto *defined = s.defined.data() + slot(s.image, int(left + firstX), int(top + row));
      const auto *mask = coverage->data() + std::size_t(row) * f.width + firstX;
      for (std::size_t i = 0; i < count; ++i)
        if(mask[i])defined[i]=0;
    }
    return;
  }
  if (mode && mode!=6) {
    for (unsigned j = 0; j < f.height; ++j)
      for (unsigned i = 0; i < f.width; ++i) {
        const auto dx = left + i, dy = top + j;
        if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height ||
            dx < clip.left || dx >= clip.right || dy < clip.top || dy >= clip.bottom ||
            !(*coverage)[std::size_t(j) * f.width + i]) continue;
        requireCpu(id,s,slot(s.image,int(dx),int(dy)));
        if (mode == 3) {
          const int shift = offsets[unsigned(dy) % period];
          const auto sample=std::uint64_t(dy)*s.image.width+dx+shift;
          if (shift < 0 || shift > 16 || sample>=s.image.pixels.size())
            throw std::invalid_argument("Producer displacement sample outside owned canvas");
          requireCpu(id,s,sample);
        }
      }
  }
  // Blends sample only their own destination. Displacement offsets are admitted
  // only in [0,16], so this left-to-right traversal samples the current pixel or
  // a later linear pixel, including the next physical row at the right edge.
  // Both preserve pre-draw sample semantics
  // without cloning the entire image/defined mask for each sprite.
  for (unsigned j = 0; j < f.height; ++j)
    for (unsigned i = 0; i < f.width; ++i) {
      const auto dx = left + i, dy = top + j;
      if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height ||
          dx < clip.left || dx >= clip.right || dy < clip.top ||
          dy >= clip.bottom)
        continue;
      const auto from = std::size_t(j) * f.width + i;
      if (!(*coverage)[from])
        continue;
      const auto at = slot(s.image, int(dx), int(dy));
      if (invalidateOnly) { s.defined[at] = 0; continue; }
      std::uint32_t colour = 0;
      if (const auto *indices =
              std::get_if<std::vector<std::uint8_t>>(&f.pixels))
        colour = colours.at(indices->at(from));
      else
        colour = std::get<std::vector<std::uint16_t>>(f.pixels).at(from);
      if(mode==6){const auto grey=((colour>>11)+((colour>>6)&31)+(colour&31))/3;colour=grey+(grey<<11)+(grey<<6);}
      if (mode && mode!=6) {
        known(s.defined, at);
        const auto d = s.image.pixels[at], m = 0x7befu;
        if (mode == 1)
          colour = ((colour >> 1) & m) + ((d >> 1) & m);
        else if (mode == 2) {
          const auto half = (d >> 1) & m;
          colour = half + ((half >> 1) & m) + ((colour >> 2) & (m >> 1) & m);
        } else if (mode == 4) {
          const auto half = (colour >> 1) & m;
          colour = half + ((half >> 1) & m) + ((d >> 2) & (m >> 1) & m);
        } else {
          const int shift = offsets.at(unsigned(dy) % period);
          const auto sample=std::uint64_t(dy)*s.image.width+dx+shift;
          if (shift < 0 || shift > 16 || sample>=s.image.pixels.size())
            throw std::invalid_argument(
                "Producer displacement sample outside owned canvas");
          requireCpu(id,s,sample);
          colour = s.image.pixels[sample];
        }
      }
      write(s, at, colour);
      s.defined[at] = 1;
    }
}
void CanvasSequence::dirty(Surface &s, int x, int y, int width, int height) {
  if (!mirror_ || !width || !height) return;
  for (int row=y; row<y+height; ++row) {
    s.dirtyFirst[row]=std::min(s.dirtyFirst[row],x);
    s.dirtyEnd[row]=std::max(s.dirtyEnd[row],x+width);
  }
}
void CanvasSequence::write(Surface &s, std::size_t at, std::uint32_t value) {
  s.image.pixels[at]=value;
  if (mirror_) dirty(s, int(at%unsigned(s.image.width)),
                      int(at/unsigned(s.image.width)),1,1);
}
void CanvasSequence::publishWrites() {
  if (!mirror_) return;
  for (auto &entry : surfaces_) {
    auto &s=entry.second;
    int left=s.image.width,right=0,top=s.image.height,bottom=0;
    for (int y=0;y<s.image.height;++y) if(s.dirtyFirst[y]<s.dirtyEnd[y]) {
      left=std::min(left,s.dirtyFirst[y]);right=std::max(right,s.dirtyEnd[y]);
      top=std::min(top,y);bottom=std::max(bottom,y+1);
    }
    if (left>=right) continue;
    auto upload=[&](int x,int y,int width,int height) {
      Image patch{width,height,{}};patch.pixels.reserve(std::size_t(width)*height);
      for(int row=y;row<y+height;++row) {
        const auto at=slot(s.image,x,row);
        patch.pixels.insert(patch.pixels.end(),s.image.pixels.begin()+at,
                             s.image.pixels.begin()+at+width);
      }
      mirror_->update(entry.first,x,y,patch);
    };
    bool full=true;
    for(int y=top;y<bottom && full;++y) {
      auto begin=s.defined.begin()+slot(s.image,left,y);
      full=std::all_of(begin,begin+right-left,[](auto state){return state==1;});
    }
    if(full) upload(left,top,right-left,bottom-top);
    else {
      Image patch{right-left,bottom-top,{}};
      patch.pixels.resize(std::size_t(patch.width)*patch.height);
      std::vector<std::uint8_t> coverage(patch.pixels.size());
      for(int y=top;y<bottom;++y)for(int x=s.dirtyFirst[y];x<s.dirtyEnd[y];++x) {
        const auto from=slot(s.image,x,y),to=std::size_t(y-top)*patch.width+x-left;
        if(s.defined[from]==1){patch.pixels[to]=s.image.pixels[from];coverage[to]=1;}
      }
      mirror_->updateMasked(entry.first,left,top,patch,coverage);
    }
    std::fill(s.dirtyFirst.begin(),s.dirtyFirst.end(),s.image.width);
    std::fill(s.dirtyEnd.begin(),s.dirtyEnd.end(),0);
  }
}
Image CanvasSequence::read(std::uint32_t id) const {
  auto it = surfaces_.find(id);
  if (it == surfaces_.end())
    throw std::out_of_range("Unknown native producer canvas");
  const auto &s = it->second;
  if (std::any_of(s.defined.begin(), s.defined.end(), [](auto state){return state!=1;}))
    throw std::runtime_error(
        "Native producer canvas completion remains undefined");
  return s.image;
}
} // namespace mnm::render
