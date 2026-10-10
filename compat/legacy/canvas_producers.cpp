#include "canvas_producers.hpp"
#include "../../assets/jpeg.hpp"
#include "../../assets/pcx.hpp"
#include "../../renderer/dib.hpp"
#include "../../protocols/include/mnm/canvas_producers_v2.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace mnm::legacy {
namespace {
std::uint32_t word(const std::vector<std::uint8_t> &b, std::size_t p) {
  if (p > b.size() || b.size() - p < 4)
    throw std::invalid_argument("Truncated producer input");
  return b[p] | std::uint32_t(b[p + 1]) << 8 | std::uint32_t(b[p + 2]) << 16 |
         std::uint32_t(b[p + 3]) << 24;
}
void put(std::vector<std::uint8_t> &b, std::size_t p, std::uint32_t v) {
  for (unsigned k = 0; k < 4; ++k)
    b.at(p + k) = std::uint8_t(v >> (8 * k));
}
int signedWord(std::uint32_t v) {
  return int(v <= 0x7fffffff ? v : std::int64_t(v) - 0x100000000LL);
}
render::Rect rect(const CanvasProducer &c) {
  const auto &r = c.fields;
  return {signedWord(r[10]), signedWord(r[11]), signedWord(r[12]),
          signedWord(r[13])};
}
assets::SpriteFrame frame(const CanvasProducer &c, bool indexed) {
  const auto size = c.fields[19];
  const unsigned base = 24 + (indexed ? 768 : 0) + 4;
  std::vector<std::uint8_t> spr(base + size);
  put(spr, 0, 0x00525053);
  put(spr, 4, spr.size());
  put(spr, 8, 4);
  put(spr, 12, 1);
  put(spr, 16, indexed ? 1 : 0);
  std::copy_n(c.payload.begin(), size, spr.begin() + base);
  put(spr, base + 28, indexed ? 0 : 0xffffffff);
  auto decoded = assets::decodeSprite(spr);
  if (auto *error = std::get_if<assets::SpriteError>(&decoded))
    throw std::invalid_argument("Producer frame refused: " + error->detail);
  return std::move(std::get<assets::Sprite>(decoded).frames.at(0));
}
} // namespace
void appendCanvasProducers(CanvasProducerStream &result,const std::vector<std::uint8_t> &b, bool complete) {
  if (b.size() < 64 || b.size() > (!std::memcmp(b.data(), "MNMPRO02", 8) ? 512u : 128u) * 1024 * 1024 ||
      (std::memcmp(b.data(), "MNMPRO01", 8) && std::memcmp(b.data(), "MNMPRO02", 8)))
    throw std::invalid_argument("Invalid producer envelope");
  const bool ownedMinimap = !std::memcmp(b.data(), "MNMPRO02", 8);
  if (word(b, 8) != (ownedMinimap ? 2u : 1u) || word(b, 12) != 64 || word(b, 16) != 0x40209ca7 ||
      (word(b, 20) != 32768 && word(b, 20) != 65536 && (!ownedMinimap || (word(b, 20) != 131072 && word(b, 20) != 262144))) || word(b, 24) < 1 ||
      word(b, 24) > 16)
    throw std::invalid_argument("Unsupported producer envelope");
  if (result.version && result.version != word(b, 8)) throw std::invalid_argument("Producer append changed wire version");
  result.version = word(b, 8);
  for (unsigned i = 28; i < 64; i += 4)
    if (word(b, i))
      throw std::invalid_argument("Nonzero producer header reserve");
  if(result.queues && result.queues!=word(b,24))throw std::invalid_argument("Producer append changed queue envelope");
  result.queues=word(b,24);
  unsigned entered = 0, returned = 0;
  for(const auto& c:result.operations){if(c.fields[2]==11)++entered;if(c.fields[2]==12)++returned;}
  for (std::size_t at = 64; at < b.size();) {
    CanvasProducer c;
    for (unsigned k = 0; k < 24; ++k)
      c.fields[k] = word(b, at + k * 4);
    const auto &r = c.fields;
    if (result.operations.size() >= word(b, 20) ||
        r[18] > (ownedMinimap ? 512u : 128u) * 1024 * 1024 - 96 || r[0] != 96 + r[18] ||
        r[0] > b.size() - at || r[1] != result.operations.size() + 1 ||
        r[2] < 1 || r[2] > (ownedMinimap ? 24u : 23u) || r[23])
      throw std::invalid_argument("Invalid producer record");
    c.payload.assign(b.begin() + at + 96, b.begin() + at + r[0]);
    if (r[2] == 8 || r[2] == 9) {
      if (r[19] < 40 || r[19] > 1048576 || r[18] != r[19] + r[20] ||
          word(c.payload, 0) != r[19] || word(c.payload, 28))
        throw std::invalid_argument("Unclosed producer frame");
      if (r[2] == 8 && r[20] != 272)
        throw std::invalid_argument("Invalid producer font table");
      if (r[2] == 9 &&
          (r[14] > 5 || r[15] > 11 || r[17] > 1 ||
           r[20] != (r[14] == 3 ? 64u : r[17] ? 512u : 0u)))
        throw std::invalid_argument("Invalid producer raster table");
    } else if (r[2] == 7 || r[2] == 20) {
      if (c.payload.empty() || c.payload.size() > 1024 || c.payload.back() ||
          std::find(c.payload.begin(), c.payload.end() - 1, 0) !=
              c.payload.end() - 1)
        throw std::invalid_argument("Invalid JPEG source name");
    } else if (r[2] == 24) {
      if (c.payload.size() < 148 || word(c.payload, 0) > 2 || word(c.payload, 36) > 3 ||
          word(c.payload, 40) > 1 || word(c.payload, 44) > 1 || word(c.payload, 48) > 1 ||
          word(c.payload, 68) > 1024 || c.payload.size() != 148 + word(c.payload, 68) * 12 ||
          (word(c.payload, 0) != 1 && word(c.payload, 68)) ||
          (word(c.payload, 0) == 2 ? r[17] > 3 : r[17] != 0))
        throw std::invalid_argument("Invalid owned minimap overlay packet");
    } else if (r[2] == 23) {
      if (r[14] > 16384 || r[18] != r[14] * 12) throw std::invalid_argument("Invalid point requests");
    } else if (r[2] == 22) {
      if (!r[14] || r[14] > 256 || r[14] % 2 || !r[15] || r[15] > 256 || r[16] > (ownedMinimap ? 3u : 0u) || (ownedMinimap && r[19] != 0xfffffffeu) || r[17] > 1 || r[18] != r[14] * r[15] * 3)
        throw std::invalid_argument("Invalid closed minimap terrain input");
    } else if (r[2] == 19) {
      if (r[19] < 40 || r[19] > 1064 || r[20] > 2097152 || r[18] != r[19] + r[20])
        throw std::invalid_argument("Invalid closed DIB input");
    } else if (r[2] == 18) {
      if (r[19] < 128 || r[19] > 1048576 || r[20] != 768 ||
          r[18] != r[19] + 768 || r[17])
        throw std::invalid_argument("Invalid PCX producer input");
    } else if (r[2] == 14) {
      if (r[18] != 20 || r[14])
        throw std::invalid_argument("Invalid RGB addition input");
    } else if (!c.payload.empty())
      throw std::invalid_argument("Unexpected producer payload");
    if (r[2] == 11) {
      if (entered != returned || r[14] != entered + 1)
        throw std::invalid_argument("Noncontiguous producer entry");
      ++entered;
    }
    if (r[2] == 12) {
      if (entered != returned + 1 || r[14] != entered)
        throw std::invalid_argument("Noncontiguous producer return");
      ++returned;
    }
    result.operations.push_back(std::move(c));
    at += r[0];
  }
  if (complete && (entered != result.queues || returned != result.queues ||
      result.operations.empty() || result.operations.back().fields[2] != 12))
    throw std::invalid_argument("Incomplete producer queue prefix");
}
CanvasProducerStream decodeCanvasProducers(const std::vector<std::uint8_t> &b,bool complete){
 CanvasProducerStream result{0,{}};appendCanvasProducers(result,b,complete);return result;
}
void CanvasProducerReplay::apply(const CanvasProducer &c) {
  const auto &r = c.fields;
  const auto id = r[3];
  switch (r[2]) {
  case 1:
    canvases_.create(id, r[5], r[6]);
    break;
  case 2:
    canvases_.release(id);
    break;
  case 3:
  case 4:
  case 10:
  case 11:
  case 12:
    break;
  case 5:
    canvases_.fill(id, rect(c), r[14]);
    break;
  case 6:
    canvases_.copy(r[4], id, rect(c), signedWord(r[8]), signedWord(r[9]),
                   r[14] == 0xffffffff ? std::nullopt
                                       : std::optional<std::uint16_t>(r[14]));
    break;
  case 7: {
    if (r[14])
      throw std::invalid_argument("JPEG requires recovered RGB565 format");
    std::string name(c.payload.begin(), c.payload.end() - 1);
    auto decoded = assets::decodeJpeg(source_(name));
    if (auto *e = std::get_if<assets::JpegError>(&decoded))
      throw std::invalid_argument("JPEG source refused: " + e->detail);
    const auto &jpg = std::get<assets::JpegImage>(decoded);
    const auto width = std::min(jpg.width, r[5]),
               height = std::min(jpg.height, r[6]);
    render::Image pixels{int(width), int(height), {}};
    pixels.pixels.reserve(std::size_t(width) * height);
    for (unsigned y = 0; y < height; ++y)
      for (unsigned x = 0; x < width; ++x) {
        const auto p = (std::size_t(y) * jpg.width + x) * 3;
        pixels.pixels.push_back((jpg.rgb[p] >> 3) << 11 |
                                (jpg.rgb[p + 1] >> 2) << 5 |
                                (jpg.rgb[p + 2] >> 3));
      }
    canvases_.update(id, 0, 0, pixels);
    break;
  }
  case 8: {
    const auto p = r[19];
    if (word(c.payload, p) != 0 || word(c.payload, p + 4) != 0xf800 ||
        word(c.payload, p + 8) != 0x07e0 || word(c.payload, p + 12) != 0x001f)
      throw std::invalid_argument(
          "Font format/masks outside recovered RGB565 scope");
    std::array<float, 64> table;
    for (unsigned k = 0; k < 64; ++k) {
      const auto raw = word(c.payload, p + 16 + k * 4);
      std::memcpy(&table[k], &raw, 4);
    }
    canvases_.glyph(id, frame(c, true), signedWord(r[8]), signedWord(r[9]),
                    rect(c), {r[15], r[16], r[17]}, table);
    break;
  }
  case 9: {
    std::array<std::uint16_t, 256> colours{};
    std::array<int, 16> offsets{};
    if (r[20] == 512)
      for (unsigned k = 0; k < 256; ++k)
        colours[k] = c.payload.at(r[19] + k * 2) |
                     unsigned(c.payload.at(r[19] + k * 2 + 1)) << 8;
    if (r[20] == 64)
      for (unsigned k = 0; k < 16; ++k)
        offsets[k] = signedWord(word(c.payload, r[19] + k * 4));
    canvases_.raster(id, frame(c, r[17] != 0), signedWord(r[8]),
                     signedWord(r[9]), rect(c), r[14], signedWord(r[15]),
                     colours, offsets, r[20] == 64 ? r[16] : 16);
    break;
  }
  case 14: {
    if (r[14] || c.payload.size() != 20)
      throw std::invalid_argument(
          "RGB addition requires recovered RGB565 input");
    const auto width = word(c.payload, 0), height = word(c.payload, 4);
    const auto x = signedWord(r[8]), y = signedWord(r[9]);
    if (width > 2048 || height > 2048 || std::int64_t(x) + width > 2147483647 ||
        std::int64_t(y) + height > 2147483647)
      throw std::invalid_argument(
          "RGB addition extent outside admitted bounds");
    auto area = rect(c);
    area.left = std::max(area.left, x);
    area.top = std::max(area.top, y);
    area.right = std::min(area.right, int(std::int64_t(x) + width));
    area.bottom = std::min(area.bottom, int(std::int64_t(y) + height));
    canvases_.addRgb(id, area,
                     {word(c.payload, 8) & 0xffff, word(c.payload, 12) & 0xffff,
                      word(c.payload, 16) & 0xffff});
    break;
  }
  case 15:
  case 16:
  case 17:
    if (r[17])
      throw std::invalid_argument(
          "Panel format outside recovered RGB565 scope");
    canvases_.panel(id, rect(c), r[2], r[14], r[15] != 0, r[16] != 0);
    break;
  case 24: {
    const auto m = [&](unsigned i) { return word(c.payload, i * 4); };
    render::MinimapOverlayView v{signedWord(m(1)), signedWord(m(2)), signedWord(m(3)), signedWord(m(4)),
        signedWord(m(5)), signedWord(m(6)), signedWord(m(7)), signedWord(m(8)), m(9), m(10) != 0};
    canvases_.minimap(id, int(r[7]), [&](render::MinimapPlane &p) -> std::size_t {
      if (m(0) == 0) {
        std::array<std::uint16_t, 9> palette{};
        for (unsigned i = 0; i < 9; ++i) { if (m(28+i) > 65535) throw std::invalid_argument("Invalid marker palette word"); palette[i] = m(28+i); }
        const auto offset = render::drawMinimapCellMarker(p, v,
            {signedWord(m(13)), signedWord(m(14)), m(15), m(16) != 0}, palette, m(12) != 0);
        if (offset != r[21]) throw std::runtime_error("Minimap cell return offset differs");
        return offset;
      }
      if (m(0) == 1) {
        std::vector<render::MinimapCreatureMarker> markers;
        for (unsigned i = 0; i < m(17); ++i) {
          const auto at = 148 + i * 12; const auto hidden = word(c.payload, at+8);
          if (hidden > 1) throw std::invalid_argument("Invalid closed creature visibility");
          markers.push_back({signedWord(word(c.payload, at)), signedWord(word(c.payload, at+4)), hidden != 0});
        }
        const auto result = render::drawMinimapCreatureMarkers(p, v, markers, m(11) != 0);
        if (unsigned(result.gridWidth) != r[19] || unsigned(result.gridHeight) != r[20])
          throw std::runtime_error("Minimap creature dimension refresh differs");
        return result.drawn;
      }
      v.orientation = r[17]; // Selected outline entry is independent of stored terrain/marker orientation.
      render::MinimapCameraOutline outline{signedWord(m(18)), signedWord(m(19)), {}, true};
      for (unsigned i = 0; i < 4; ++i) outline.corners[i] = {signedWord(m(20+i*2)), signedWord(m(21+i*2))};
      const auto result = render::drawMinimapCameraOutline(p, v, outline);
      if (result.originX != signedWord(r[19]) || result.originY != signedWord(r[20]))
        throw std::runtime_error("Minimap camera origin differs");
      return 0;
    });
    break;
  }
  case 23:
    for (unsigned i = 0; i < r[14]; ++i)
      canvases_.update(id, signedWord(word(c.payload, i * 12)), signedWord(word(c.payload, i * 12 + 4)), {1, 1, {word(c.payload, i * 12 + 8)}});
    break;
  case 22: {
    std::vector<std::uint16_t> colours;
    std::vector<unsigned char> hidden;
    for (std::size_t i = 0; i < c.payload.size(); i += 3) {
      colours.push_back(unsigned(c.payload[i]) | unsigned(c.payload[i + 1]) << 8);
      if (c.payload[i + 2] > 1 || (!r[17] && c.payload[i + 2])) throw std::invalid_argument("Invalid minimap visibility");
      hidden.push_back(c.payload[i + 2]);
    }
    // V2 uses the validated four-orientation service and owned prior auxiliary
    // composition. V1 retains its historical orientation-zero contract.
    if (r[19] == 0xfffffffeu) {
      render::MinimapPlane history{};
      if (r[17]) { const auto image = canvases_.read(r[4]); history = {image.width, image.height, image.width, {}}; for (auto p : image.pixels) history.words.push_back(std::uint16_t(p)); }
      render::MinimapTerrain terrain{int(r[14]), int(r[15]), signedWord(r[10]), signedWord(r[11]), signedWord(r[8]), signedWord(r[9]), r[16], colours, hidden};
      canvases_.minimap(id, int(r[7]), [&](render::MinimapPlane &plane) { render::drawMinimapTerrain(plane, terrain, r[17] ? &history : nullptr); return 0; });
    } else if (r[16]) throw std::invalid_argument("Minimap invalidation result missing");
    else canvases_.terrainMap(id, signedWord(r[8]), signedWord(r[9]), int(r[14]), int(r[15]), signedWord(r[10]), signedWord(r[11]), colours, hidden, r[17] ? r[4] : 0);
    break;
  }
  case 21:
    if (r[14] || r[7] != r[5]) throw std::invalid_argument("Fade requires packed RGB565 rows");
    canvases_.fade(id);
    break;
  case 20: {
    if (r[14]) throw std::invalid_argument("BMP requires RGB565 destination");
    std::string name(c.payload.begin(), c.payload.end() - 1);
    const auto raw = source_(name);
    if (raw.size() < 54 || raw[0] != 'B' || raw[1] != 'M' || word(raw, 14) != 40)
      throw std::invalid_argument("Unsupported bitmap file header");
    const auto bits = unsigned(raw[28]) | unsigned(raw[29]) << 8;
    if (bits != 1 && bits != 4 && bits != 8 && bits != 24)
      throw std::invalid_argument("Unsupported bitmap source depth");
    const auto palette = bits == 24 ? 0 : (1u << bits) * 4;
    const auto offset = word(raw, 10);
    if (raw.size() < 54 + palette || offset < 54 + palette || offset > raw.size())
      throw std::invalid_argument("Unclosed bitmap source extent");
    render::DibInput dib;
    std::copy_n(raw.begin() + 14, 40, dib.header.begin());
    dib.palette.assign(raw.begin() + 54, raw.begin() + 54 + palette);
    dib.pixels.assign(raw.begin() + offset, raw.end());
    dib.usage = bits == 24 ? 1 : 0;
    auto image = render::decodeDibRgb(dib);
    for (auto &rgb : image.pixels)
      rgb = ((rgb >> 19) & 31) << 11 | ((rgb >> 10) & 63) << 5 | ((rgb >> 3) & 31);
    const int x = signedWord(r[8]), y = signedWord(r[9]);
    const int left = std::max(0, x), top = std::max(0, y);
    const int right = int(std::min<std::int64_t>(r[5], std::int64_t(x) + image.width));
    const int bottom = int(std::min<std::int64_t>(r[6], std::int64_t(y) + image.height));
    if (right > left && bottom > top) {
      render::Image cropped{right - left, bottom - top, {}};
      for (int row = top; row < bottom; ++row)
        for (int col = left; col < right; ++col)
          cropped.pixels.push_back(image.pixels.at(std::size_t(row - y) * image.width + col - x));
      canvases_.update(id, left, top, cropped);
    }
    break;
  }
  case 19: {
    render::DibInput dib;
    std::copy_n(c.payload.begin(), 40, dib.header.begin());
    dib.palette.assign(c.payload.begin() + 40, c.payload.begin() + r[19]);
    dib.pixels.assign(c.payload.begin() + r[19], c.payload.end());
    dib.usage = r[14];
    auto image = render::decodeDibRgb(dib);
    for (auto &rgb : image.pixels)
      rgb = ((rgb >> 19) & 31) << 11 | ((rgb >> 10) & 63) << 5 | ((rgb >> 3) & 31);
    canvases_.update(id, 0, 0, image);
    break;
  }
  case 18: {
    std::vector<std::uint8_t> raw(c.payload.begin(), c.payload.begin() + r[19]);
    if (raw.size() < 897 || raw[raw.size() - 769] != 12)
      throw std::invalid_argument("PCX producer source lacks terminal palette");
    std::copy_n(c.payload.begin() + r[19], 768, raw.end() - 768);
    auto decoded = assets::decodePcx(raw);
    if (auto *e = std::get_if<assets::PcxError>(&decoded))
      throw std::invalid_argument("PCX producer refused: " + e->detail);
    const auto &pcx = std::get<assets::PcxImage>(decoded);
    if (pcx.width != r[14] || pcx.height != r[15] ||
        pcx.bytesPerLine != pcx.width)
      throw std::invalid_argument(
          "PCX producer requires recovered unpadded source rows");
    render::Image pixels{int(pcx.width), int(pcx.height), {}};
    for (auto index : pcx.indices) {
      const auto &rgb = pcx.palette[index];
      pixels.pixels.push_back((rgb[0] >> 3) << 11 | (rgb[1] >> 2) << 5 |
                              (rgb[2] >> 3));
    }
    canvases_.update(id, 0, 0, pixels);
    break;
  }
  case 13:
    throw std::runtime_error("Original producer capture refused input, code " +
                             std::to_string(r[14]));
  default:
    throw std::invalid_argument("Unknown producer operation");
  }
}
} // namespace mnm::legacy
