#include "canvas_sequence.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mnm::render {
namespace {
std::size_t slot(const Image &image, int x, int y) {
  return std::size_t(y) * image.width + x;
}
void known(const std::vector<unsigned char> &defined, std::size_t at) {
  if (!defined.at(at))
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
                            std::vector<unsigned char>(count)});
  pixels_ += count;
}
std::size_t CanvasSequence::minimap(std::uint32_t id, int stride,
    const std::function<std::size_t(MinimapPlane &)> &draw) {
  auto &s = surface(id);
  if (stride < s.image.width || stride > 4096)
    throw std::invalid_argument("Invalid captured minimap stride");
  // Two independent owned backgrounds identify every written word, including
  // writes equal to either sentinel. No destination oracle enters replay.
  MinimapPlane a{s.image.width, s.image.height, stride,
      std::vector<std::uint16_t>(std::size_t(stride) * s.image.height, 0x1357)};
  auto b = a; std::fill(b.words.begin(), b.words.end(), 0xeca8);
  const auto result = draw(a); if (draw(b) != result)
    throw std::logic_error("Minimap result depends on diagnostic background");
  for (int y = 0; y < s.image.height; ++y)
    for (int x = 0; x < s.image.width; ++x) {
      const auto from = std::size_t(y) * stride + x, to = std::size_t(y) * s.image.width + x;
      if (a.words[from] == b.words[from]) { s.image.pixels[to] = a.words[from]; s.defined[to] = 1; }
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
  for (int row = 0; row < h; ++row)
    for (int col = 0; col < w; ++col) {
      const auto at = slot(s.image, rootX - 2 * (row / 2) + col, rootY + (row + 1) / 2 + col / 2);
      const auto cell = std::size_t((startY + row) % h) * w + (startX + col) % w;
      if (!hidden.empty() && hidden.at(cell)) {
        if (!previous) throw std::invalid_argument("Minimap hidden cell lacks owned history");
        s.image.pixels[at] = old.pixels.at(slot(old, w - 1 - 2 * (row / 2) + col, (row + 1) / 2 + col / 2));
      } else s.image.pixels[at] = colours.at(cell);
      s.defined[at] = 1;
    }
}
void CanvasSequence::fade(std::uint32_t id) {
  auto &s = surface(id);
  if (s.image.width % 2) throw std::invalid_argument("Fade requires even packed rows");
  for (std::size_t i = 0; i < s.image.pixels.size(); ++i) known(s.defined, i);
  for (auto &p : s.image.pixels) p = (p >> 1) & 0x7bef;
}
void CanvasSequence::release(std::uint32_t id) {
  auto &s = surface(id);
  pixels_ -= s.image.pixels.size();
  surfaces_.erase(id);
}
void CanvasSequence::addRgb(std::uint32_t id, Rect r,
                            std::array<unsigned, 3> add) {
  auto &s = surface(id);
  if (add[0] > 31 || add[1] > 63 || add[2] > 31)
    throw std::invalid_argument(
        "RGB addition outside recovered unsigned channels");
  for (int y = std::max(0, r.top); y < std::min(s.image.height, r.bottom); ++y)
    for (int x = std::max(0, r.left); x < std::min(s.image.width, r.right);
         ++x) {
      const auto at = slot(s.image, x, y);
      known(s.defined, at);
      const auto old = s.image.pixels[at];
      s.image.pixels[at] = (std::min(31u, ((old >> 11) & 31) + add[0]) << 11) |
                           (std::min(63u, ((old >> 5) & 63) + add[1]) << 5) |
                           std::min(31u, (old & 31) + add[2]);
    }
}
void CanvasSequence::panel(std::uint32_t id, Rect r, unsigned mode,
                           unsigned percent, bool pressed, bool reverse) {
  auto &s = surface(id);
  if (mode < 15 || mode > 17 || percent > 1000 || r.left < 0 || r.top < 0 ||
      r.right < r.left || r.bottom < r.top || r.right >= s.image.width ||
      r.bottom >= s.image.height)
    throw std::invalid_argument("Panel writer outside recovered owned extent");
  auto pixel = [&](int x, int y, int sign) {
    const auto at = slot(s.image, x, y);
    known(s.defined, at);
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
    s.image.pixels[at] = ((c[0] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[2] >> 3);
  };
  if (mode == 17) {
    for (int y = r.top; y < r.bottom; ++y)
      for (int x = r.left; x < r.right; ++x)
        pixel(x, y, 0);
    return;
  }
  if (mode == 16) {
    for (int x = r.left; x < r.right; ++x)
      pixel(x, r.top, 0);
    for (int x = r.left + 1; x <= r.right; ++x)
      pixel(x, r.bottom, 0);
    for (int y = r.top + 1; y <= r.bottom; ++y)
      pixel(r.left, y, 0);
    for (int y = r.top; y < r.bottom; ++y)
      pixel(r.right, y, 0);
    return;
  }
  const int right = r.right + 1, bottom = r.bottom + 1, sign = pressed ? 1 : -1;
  if (right >= s.image.width || bottom >= s.image.height)
    throw std::invalid_argument(
        "Bevel's inclusive extra row/column escaped owned extent");
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
void CanvasSequence::fill(std::uint32_t id, Rect r, std::uint16_t colour) {
  auto &s = surface(id);
  if (r.right < r.left || r.bottom < r.top)
    throw std::invalid_argument("Inverted producer fill");
  for (int y = std::max(0, r.top); y < std::min(s.image.height, r.bottom); ++y)
    for (int x = std::max(0, r.left); x < std::min(s.image.width, r.right);
         ++x) {
      const auto at = slot(s.image, x, y);
      s.image.pixels[at] = colour;
      s.defined[at] = 1;
    }
}
void CanvasSequence::copy(std::uint32_t source, std::uint32_t destination,
                          Rect r, int x, int y,
                          std::optional<std::uint16_t> key) {
  const auto src = surface(source);
  auto &dst = surface(destination);
  if (r.left < 0 || r.top < 0 || r.right < r.left || r.bottom < r.top ||
      r.right > src.image.width || r.bottom > src.image.height)
    throw std::invalid_argument("Invalid producer copy source");
  // Validate every sampled source before any destination write. Same-ID copies
  // retain the source snapshot, matching ordered owned-storage copy semantics.
  for (int j = r.top; j < r.bottom; ++j)
    for (int i = r.left; i < r.right; ++i) {
      const auto dx = std::int64_t(x) + i - r.left,
                 dy = std::int64_t(y) + j - r.top;
      if (dx < 0 || dy < 0 || dx >= dst.image.width || dy >= dst.image.height)
        continue;
      known(src.defined, slot(src.image, i, j));
    }
  for (int j = r.top; j < r.bottom; ++j)
    for (int i = r.left; i < r.right; ++i) {
      const auto dx = std::int64_t(x) + i - r.left,
                 dy = std::int64_t(y) + j - r.top;
      if (dx < 0 || dy < 0 || dx >= dst.image.width || dy >= dst.image.height)
        continue;
      auto word = src.image.pixels[slot(src.image, i, j)];
      if (key && word == *key)
        continue;
      const auto at = slot(dst.image, int(dx), int(dy));
      dst.image.pixels[at] = word;
      dst.defined[at] = 1;
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
  for (int j = 0; j < image.height; ++j)
    for (int i = 0; i < image.width; ++i) {
      const auto at = slot(s.image, x + i, y + j);
      s.image.pixels[at] = image.pixels[slot(image, i, j)];
      s.defined[at] = 1;
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
    if (!std::isfinite(c) || c < 0 || c > 1.000001f)
      throw std::invalid_argument("Font coverage outside unit range");
  const auto left = std::int64_t(x) - f.originX,
             top = std::int64_t(y) - f.originY;
  // Original581ec0 refuses horizontal clipping; vertical rows are cropped.
  if (left < clip.left || left + f.width > clip.right)
    return;
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
      if (a >= 64)
        throw std::invalid_argument(
            "Font coverage index exceeds initialized table");
      const auto at = slot(s.image, int(dx), int(dy));
      known(s.defined, at);
      const auto old = s.image.pixels[at];
      const double weight = coverage[a];
      const unsigned d[3] = {(old >> 11) & 31, (old >> 5) & 63, old & 31};
      unsigned channel[3];
      for (unsigned k = 0; k < 3; ++k)
        channel[k] = unsigned(d[k] * (1 - weight) + target[k] * weight);
      s.image.pixels[at] = (channel[0] << 11) | (channel[1] << 5) | channel[2];
    }
}
void CanvasSequence::raster(std::uint32_t id, const assets::SpriteFrame &f,
                            int x, int y, Rect clip, unsigned mode, int backend,
                            const std::array<std::uint16_t, 256> &colours,
                            const std::array<int, 16> &offsets,
                            unsigned period) {
  frame(f);
  auto &s = surface(id);
  if (mode > 5 || !period || period > 16)
    throw std::invalid_argument("Invalid producer raster mode");
  const auto left = std::int64_t(x) - f.originX,
             top = std::int64_t(y) - f.originY;
  if ((backend == 0 || backend == 9) &&
      (left < clip.left || left + f.width >= clip.right))
    return;
  if (mode == 5) {
    const auto shadowTop = top + f.height / 2;
    if (left < clip.left || left + f.width > clip.right ||
        shadowTop >= clip.bottom || shadowTop + f.height <= clip.top)
      return;
    const int first = int(std::max<std::int64_t>(0, clip.top - shadowTop)),
              last = int(
                  std::min<std::int64_t>(f.height, clip.bottom - shadowTop));
    for (int row = 0; row < (last - first + 1) / 2; ++row)
      for (unsigned col = 0; col < f.width; ++col) {
        if (!f.opaqueMask.at(std::size_t(first + row * 2) * f.width + col))
          continue;
        const auto dx = left + col + last / 2 - row,
                   dy = shadowTop + first + row;
        if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height)
          throw std::invalid_argument(
              "Projected shadow sample outside owned canvas");
        const auto at = slot(s.image, int(dx), int(dy));
        known(s.defined, at);
        s.image.pixels[at] = (s.image.pixels[at] >> 1) & 0x7bef;
      }
    return;
  }
  const auto old = mode ? s.image : Image{};
  const auto defined = mode ? s.defined : std::vector<unsigned char>{};
  for (unsigned j = 0; j < f.height; ++j)
    for (unsigned i = 0; i < f.width; ++i) {
      const auto dx = left + i, dy = top + j;
      if (dx < 0 || dy < 0 || dx >= s.image.width || dy >= s.image.height ||
          dx < clip.left || dx >= clip.right || dy < clip.top ||
          dy >= clip.bottom)
        continue;
      const auto from = std::size_t(j) * f.width + i;
      if (!f.opaqueMask[from])
        continue;
      const auto at = slot(s.image, int(dx), int(dy));
      std::uint32_t colour = 0;
      if (const auto *indices =
              std::get_if<std::vector<std::uint8_t>>(&f.pixels))
        colour = colours.at(indices->at(from));
      else
        colour = std::get<std::vector<std::uint16_t>>(f.pixels).at(from);
      if (mode) {
        known(defined, at);
        const auto d = old.pixels[at], m = 0x7befu;
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
          if (shift < 0 || shift > 16 || dx + shift >= s.image.width)
            throw std::invalid_argument(
                "Producer displacement sample outside owned canvas");
          const auto sample = slot(old, int(dx) + shift, int(dy));
          known(defined, sample);
          colour = old.pixels[sample];
        }
      }
      s.image.pixels[at] = colour;
      s.defined[at] = 1;
    }
}
Image CanvasSequence::read(std::uint32_t id) const {
  auto it = surfaces_.find(id);
  if (it == surfaces_.end())
    throw std::out_of_range("Unknown native producer canvas");
  const auto &s = it->second;
  if (std::find(s.defined.begin(), s.defined.end(), 0) != s.defined.end())
    throw std::runtime_error(
        "Native producer canvas completion remains undefined");
  return s.image;
}
} // namespace mnm::render
