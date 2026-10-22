#include "../reconstruction/rendering/palette_shading.hpp"
#include "../reconstruction/rendering/raster_pixels.h"
#include "../reconstruction/rendering/world_consumer.hpp"
#include "../reconstruction/rendering/world_special.hpp"
#include <iostream>
#include <vector>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
static std::vector<U> calls;
static void output(const void *p, std::size_t n) {
  std::cout.write(static_cast<const char *>(p), n);
}
static std::uint16_t specialColour(unsigned palette, unsigned table,
                                   unsigned index) {
  return std::uint16_t(palette * 17137 + table * 1357 + index * 7919);
}
struct SpecialFixtureFrame {
  std::vector<unsigned char> bytes;
  bool words;
  SpecialFixtureFrame(unsigned identity, bool split, bool word, bool empty)
      : bytes(empty ? 40 : 72), words(word) {
    put(4, empty ? 0 : 5);
    put(8, empty ? 0 : 4);
    put(12, 1);
    put(16, 2);
    put(28, word ? UINT32_MAX : 0);
    std::array<unsigned char, 20> mask{}, indices{};
    if (!empty) {
      for (unsigned y = 0; y < 4; ++y) {
        for (unsigned x = 0; x < 5; ++x) {
          mask[y * 5 + x] = !split || (x + 2 * y) % 3;
          indices[y * 5 + x] = identity * 57 + x + y * 17;
        }
        put(40 + y * 8, U(bytes.size()));
        unsigned x = 0;
        while (x < 5) {
          unsigned start = x;
          while (x < 5 && !mask[y * 5 + x])
            ++x;
          bytes.push_back(x - start);
          if (x == 5)
            break;
          start = x;
          while (x < 5 && mask[y * 5 + x])
            ++x;
          bytes.push_back(x - start);
        }
      }
      for (unsigned y = 0; y < 4; ++y) {
        put(44 + y * 8, U(bytes.size()));
        for (unsigned x = 0; x < 5; ++x)
          if (mask[y * 5 + x]) {
            const auto value = word ? specialColour(0, 15, indices[y * 5 + x])
                                    : indices[y * 5 + x];
            bytes.push_back(value);
            if (word)
              bytes.push_back(value >> 8);
          }
      }
    }
    put(0, U(bytes.size()));
  }
  void put(unsigned at, U value) { std::memcpy(bytes.data() + at, &value, 4); }
};
static U specialReturn(U address) {
  if (address == 0x595677 || address == 0x5947b2 || address == 0x59521a)
    return 0x12340001;
  if (address == 0x597086 || address == 0x596cb8)
    return 0x12340011;
  if (address == 0x57de00)
    return 0x12340024;
  return address == 0x57f0f0   ? 0x12340080
         : address == 0x57ec90 ? 0x12340100
                               : 0x12340200;
}
#ifdef MNM_ORIGINAL_REFERENCE
static std::array<U, 4> sourceAddresses;
static U specialPointer(const void *p) {
  return U(reinterpret_cast<std::uintptr_t>(p));
}
extern "C" __attribute__((force_align_arg_pointer)) void
capture_world_special(U tag, U *registers) {
  const auto *args = registers + 10;
  const bool stack = tag == 0x595677 || tag == 0x5947b2 || tag == 0x59521a ||
                     tag == 0x597086 || tag == 0x596cb8 || tag == 0x57de00;
  const auto source = stack ? args[0] : registers[6];
  U token = 0;
  for (U i = 0; i < 4; ++i)
    if (source == sourceAddresses[i])
      token = i + 1;
  if (!token)
    throw std::runtime_error("Packed frame identity");
  const bool shortCall = tag == 0x595677 || tag == 0x597086 || tag == 0x596cb8;
  calls.insert(calls.end(), {tag, token, stack ? args[1] : registers[5],
                             stack ? args[2] : args[0],
                             shortCall ? 0u
                             : stack   ? args[3]
                                       : args[1],
                             shortCall ? 0u
                             : stack   ? args[4]
                                       : args[2]});
  registers[7] = specialReturn(tag);
}
static void *specialStub(U tag, unsigned pop) {
  auto *code = static_cast<unsigned char *>(
      mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (code == MAP_FAILED)
    throw std::runtime_error("Special trace allocation");
  const unsigned char head[]{0x9c, 0x60, 0x89, 0xe0, 0x50, 0x68};
  std::memcpy(code, head, 6);
  std::memcpy(code + 6, &tag, 4);
  code[10] = 0xe8;
  const U relative =
      specialPointer(reinterpret_cast<void *>(&capture_world_special)) -
      specialPointer(code + 15);
  std::memcpy(code + 11, &relative, 4);
  const unsigned char tail[]{0x83, 0xc4, 8, 0x61, 0x9d, 0xc2};
  std::memcpy(code + 15, tail, 6);
  const std::uint16_t bytes = pop;
  std::memcpy(code + 21, &bytes, 2);
  return code;
}
#endif
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 3)
    throw std::runtime_error("Expected original executable and trace/pixels");
  map_image(read(argv[1]));
  const std::string mode = argv[2];
  std::vector<std::pair<U, Bytes>> original;
  for (auto range : std::vector<std::pair<U, U>>{{0x57e8e0, 0x57ea60},
                                                 {0x57fe10, 0x580260},
                                                 {0x580260, 0x5806f0},
                                                 {0x581200, 0x581700},
                                                 {0x5002a0, 0x501181}}) {
    const auto *p = reinterpret_cast<const unsigned char *>(range.first);
    original.push_back({range.first, Bytes(p, p + range.second - range.first)});
  }
  if (mode == "trace")
    for (auto target : std::vector<std::pair<U, unsigned>>{{0x5947b2, 0},
                                                           {0x59521a, 0},
                                                           {0x595677, 0},
                                                           {0x597086, 0},
                                                           {0x596cb8, 0},
                                                           {0x57de00, 0},
                                                           {0x57f0f0, 12},
                                                           {0x57ec90, 12},
                                                           {0x57f5f0, 12}}) {
      auto *entry = reinterpret_cast<unsigned char *>(target.first);
      const U relative =
          specialPointer(specialStub(target.first, target.second)) -
          target.first - 5;
      entry[0] = 0xe9;
      std::memcpy(entry + 1, &relative, 4);
    }
  std::array<std::vector<U>, 2> palettes;
  for (U n = 0; n < 2; ++n) {
    palettes[n].resize(3 + 31 * 128);
    palettes[n][1] = 3;
    palettes[n][2] = 15;
    auto *words = reinterpret_cast<std::uint16_t *>(palettes[n].data() + 3);
    for (U table = 0; table < 31; ++table)
      for (U index = 0; index < 256; ++index)
        words[table * 256 + index] = specialColour(n, table, index);
  }
  palettes[0][0] = specialPointer(palettes[1].data());
#else
  if (argc != 2)
    throw std::runtime_error("Expected trace/pixels");
  const std::string mode = argv[1];
#endif
  if (mode != "trace" && mode != "pixels" && mode != "queue")
    throw std::runtime_error("Special comparison mode");
  [[maybe_unused]] const bool trace = mode == "trace";
  std::uint16_t control = 0x037f;
  __asm__ volatile("fldcw %0" ::"m"(control));
  const std::array<int, 2> positions[]{{3, 5}, {7, 8},   {28, 22},
                                       {0, 3}, {-4, -1}, {1, 2}};
  const std::array<int, 4> clips[]{{2, 3, 29, 23}, {0, 0, 40, 30}};
  const std::array<unsigned, 3> tints[]{{244, 198, 13},
                                        {10, 10, 255},
                                        {214, 95, 172},
                                        {UINT32_MAX, 256, 0x12345678}};
  U id = 0;
  for (U operation = 0; operation < 4; ++operation)
    for (bool format : {false, true})
      for (int shade : {-127, -8, 0, 127})
        for (U palette : {0u, 1u})
          for (bool empty : {false, true})
            for (bool split : {false, true})
              for (U position = 0; position < 6; ++position)
                for (U clipIndex = 0; clipIndex < 2; ++clipIndex)
                  for (U tint = 0; tint < (operation == 2 ? 4u : 1u); ++tint)
                    for (U amplitude : operation == 3
                                           ? std::vector<U>{0, 1, 3, 8}
                                           : std::vector<U>{3})
                      for (U cold = 0; cold < (operation == 3 ? 2u : 1u);
                           ++cold)
                        for (U phase = 0; phase < (operation == 3 ? 3u : 1u);
                             ++phase)
                          for (U blend = 0; blend < (operation == 0 ? 2u : 1u);
                               ++blend)
                            for (U word = 0; word < (operation == 0 ? 2u : 1u);
                                 ++word)
                              for (U provider = 0;
                                   provider < (operation == 0 ? 2u : 1u);
                                   ++provider) {
                                if (operation == 3 && !amplitude && !empty)
                                  continue;
                                std::array<SpecialFixtureFrame, 4> frames{
                                    SpecialFixtureFrame(0, split, word, empty),
                                    SpecialFixtureFrame(1, split,
                                                        word && !blend, empty),
                                    SpecialFixtureFrame(2, split, false, empty),
                                    SpecialFixtureFrame(3, split, false,
                                                        empty)};
                                std::vector<unsigned char> packed;
                                std::array<U, 4> offsets{};
                                for (U i = 0; i < 4; ++i) {
                                  offsets[i] = packed.size();
                                  packed.insert(packed.end(),
                                                frames[i].bytes.begin(),
                                                frames[i].bytes.end());
                                }
                                std::vector<std::uint16_t> pixels(48 * 38);
                                for (U i = 0; i < pixels.size(); ++i)
                                  pixels[i] = std::uint16_t(i * 2017 + 0x1234);
                                WorldWriteDisplacement displacement;
                                displacement.amplitude = amplitude;
                                displacement.phase = phase == 0   ? 0
                                                     : phase == 1 ? 1
                                                                  : 0xffffffff;
                                for (U i = 0; i < 16; ++i)
                                  displacement.table[i] =
                                      cold ? 32000
                                           : int((i * 7 + 3) %
                                                 (amplitude * 2 + 1));
                                const auto xy = positions[position];
                                const auto clip = clips[clipIndex];
                                calls.clear();
                                std::uint16_t returned = 0;
#ifdef MNM_ORIGINAL_REFERENCE
                                for (U i = 0; i < 4; ++i) {
                                  sourceAddresses[i] = specialPointer(
                                      packed.data() + offsets[i]);
                                  if (!frames[i].words) {
                                    const U value =
                                        specialPointer(palettes[0].data());
                                    std::memcpy(packed.data() + offsets[i] + 28,
                                                &value, 4);
                                  }
                                }
                                const auto before = packed;
                                global(0x658174,
                                       specialPointer(pixels.data() + 48));
                                global(0x6a2dc8, 48);
                                global(0x6def50, 36);
                                global(0x6e1f88, format);
                                global(0x6e1f68, 0);
                                global(0x6e0008, U(clip[0]));
                                global(0x6cbb6c, U(clip[1]));
                                global(0x6a49b8, U(clip[2]));
                                global(0x656618, U(clip[3]));
                                global(0x6c4e3c,
                                       provider ? 0x59521a : 0x5947b2);
                                global(0x6903dc,
                                       provider ? 0x596cb8 : 0x597086);
                                global(0x6de6e1, blend);
                                global(0x5e15f8, amplitude);
                                global(0x6c4830, displacement.phase);
                                for (U i = 0; i < 16; ++i)
                                  global(0x5f1450 + i * 4,
                                         U(displacement.table[i]));
                                if (mode == "queue") {
                                  U row[9]{};
                                  row[1] = specialPointer(packed.data());
                                  row[2] = U(xy[0]);
                                  row[3] = U(xy[1]);
                                  const std::int16_t parameters[]{
                                      std::int16_t(shade),
                                      std::int16_t(palette)};
                                  std::memcpy(row + 4, parameters, 4);
                                  row[6] = operation == 0   ? 2
                                           : operation == 1 ? 7
                                           : operation == 2 ? 12 + tint % 3
                                                            : 11;
                                  row[7] = 0x8ad08ad0;
                                  global(0x6c482c, specialPointer(clip.data()));
                                  global(0x6e1f70, 0);
                                  global(0x5e1404, id % 2);
                                  global(0x6de6d5, 0);
                                  global(0x6e1f24, 0);
                                  U envelope[]{
                                      specialPointer(row), 0, 1, 1, 0, 0};
                                  using Consume =
                                      void(__attribute__((thiscall)) *)(void *);
                                  reinterpret_cast<Consume>(0x5002a0)(envelope);
                                  returned = std::uint16_t(row[7] >> 16);
                                } else if (operation == 2) {
                                  using Tint = U(__attribute__((fastcall)) *)(
                                      void *, int, int, int, U, U, U);
                                  returned = std::uint16_t(
                                      reinterpret_cast<Tint>(0x581200)(
                                          packed.data(), xy[0], xy[1], shade,
                                          tints[tint][0], tints[tint][1],
                                          tints[tint][2]));
                                } else {
                                  const U entries[]{0x57e8e0, 0x57fe10, 0,
                                                    0x580260};
                                  using Draw = U(__attribute__((fastcall)) *)(
                                      void *, int, int, int, U);
                                  returned =
                                      std::uint16_t(reinterpret_cast<Draw>(
                                          entries[operation])(packed.data(),
                                                              xy[0], xy[1],
                                                              shade, palette));
                                }
                                for (U i = 0; i < 16; ++i)
                                  displacement.table[i] =
                                      *reinterpret_cast<std::int32_t *>(
                                          0x5f1450 + i * 4);
                                if (packed != before)
                                  throw std::runtime_error(
                                      "Original special helper modified source "
                                      "frames");
#else
                                ShadedPalette shading;
                                shading.shift = 3;
                                shading.neutral = 15;
                                shading.tables.resize(31);
                                auto colour = [&](U pal, int sh,
                                                  unsigned index) {
                                  return specialColour(
                                      pal, unsigned(shading.tableIndex(sh)),
                                      index);
                                };
                                auto drawOperation =
                                    [&](U operation, std::array<int, 2> xy,
                                        int shade, U palette,
                                        std::array<unsigned, 3> rgb)
                                    -> std::uint16_t {
                                  if (operation == 0) {
                                    std::array<WorldOpaqueSource, 4> sources{};
                                    for (U i = 0; i < 4; ++i)
                                      sources[i] = {i + 1, frames[i].words};
                                    return drawWorldOpaque(
                                        sources, blend, xy[0], xy[1], shade,
                                        palette,
                                        [&](WorldOpaqueEntry entry, U token,
                                            int x, int y, int sh, U pal) {
                                          const U target =
                                              entry == WorldOpaqueEntry::word
                                                  ? (provider ? 0x596cb8
                                                              : 0x597086)
                                              : entry ==
                                                      WorldOpaqueEntry::indexed
                                                  ? (provider ? 0x59521a
                                                              : 0x5947b2)
                                              : entry == WorldOpaqueEntry::black
                                                  ? 0x595677
                                              : entry ==
                                                      WorldOpaqueEntry::fallback
                                                  ? 0x57de00
                                              : entry ==
                                                      WorldOpaqueEntry::blend75
                                                  ? 0x57f0f0
                                              : entry ==
                                                      WorldOpaqueEntry::blend50
                                                  ? 0x57ec90
                                                  : 0x57f5f0;
                                          const bool shortCall =
                                              entry == WorldOpaqueEntry::word ||
                                              entry == WorldOpaqueEntry::black;
                                          if (trace) {
                                            calls.insert(
                                                calls.end(),
                                                {target, token, U(x), U(y),
                                                 shortCall ? 0u : U(sh),
                                                 shortCall ? 0u : pal});
                                            return specialReturn(target);
                                          }
                                          if (empty)
                                            return 0u;
                                          const int left = x - 1, top = y - 2;
                                          if (entry ==
                                                  WorldOpaqueEntry::black &&
                                              (left < clip[0] ||
                                               left + 5 >= clip[2]))
                                            return 1u;
                                          MnmPixelPlane destination{
                                              pixels.data() + 48, 48, 36, 48,
                                              U(format)};
                                          std::array<std::uint16_t, 256>
                                              effective{};
                                          for (U i = 0; i < 256; ++i)
                                            effective[i] = colour(pal, sh, i);
                                          MnmRasterPixels request{};
                                          request.frame =
                                              frames[token - 1].bytes.data();
                                          request.bytes =
                                              frames[token - 1].bytes.size();
                                          request.indexed =
                                              !frames[token - 1].words;
                                          request.palette_step = 2;
                                          request.palette = reinterpret_cast<
                                              const unsigned char *>(
                                              effective.data());
                                          request.period = 1;
                                          request.contiguous = 1;
                                          request.left = left;
                                          request.top = top;
                                          request.clip_left = clip[0];
                                          request.clip_top = clip[1];
                                          request.clip_right = clip[2];
                                          request.clip_bottom = clip[3];
                                          request.mode =
                                              entry == WorldOpaqueEntry::black
                                                  ? 7
                                              : entry ==
                                                      WorldOpaqueEntry::blend75
                                                  ? 4
                                              : entry ==
                                                      WorldOpaqueEntry::blend50
                                                  ? 1
                                              : entry ==
                                                      WorldOpaqueEntry::blend25
                                                  ? 2
                                                  : 0;
                                          if (!mnm_raster_pixels(&destination,
                                                                 &request))
                                            throw std::runtime_error(
                                                "Native opaque raster "
                                                "admission");
                                          return 0u;
                                        });
                                  } else
                                    return drawWorldSpecial(
                                        operation == 1
                                            ? WorldSpecialOperation::outline
                                        : operation == 2
                                            ? WorldSpecialOperation::tint
                                            : WorldSpecialOperation::
                                                  writeDisplacement,
                                        decodeWorldSpecial(frames[0].bytes),
                                        {pixels, 48, 36, 48, format, 48, clip},
                                        xy[0], xy[1], shade, palette, rgb,
                                        displacement, colour);
                                };
                                if (mode == "queue") {
                                  QueueRow draw{operation == 0   ? 2
                                                : operation == 1 ? 7
                                                : operation == 2
                                                    ? int(12 + tint % 3)
                                                    : 11,
                                                77,
                                                U(xy[0]),
                                                U(xy[1]),
                                                std::int16_t(shade),
                                                std::int16_t(palette),
                                                !frames[0].words,
                                                {}};
                                  std::vector<ConsumerRow> rows{
                                      {draw, empty ? 0u : 4u, 2, 0x8ad0,
                                       0x8ad0}};
                                  ConsumerState current{
                                      bool(id % 2),
                                      false,
                                      false,
                                      0,
                                      {clip[0], clip[1], clip[2], clip[3]},
                                      {clip[0], clip[1], clip[2], clip[3]},
                                      {}};
                                  consumeWorldRows(
                                      rows, current,
                                      [&](const QueueCall &call) {
                                        if (call.values[0] != 77)
                                          throw std::runtime_error(
                                              "Special queue source identity");
                                        const U kind =
                                            call.backend == QueueBackend::opaque
                                                ? 0
                                            : call.backend ==
                                                    QueueBackend::
                                                        alternateShadow
                                                ? 1
                                            : call.backend == QueueBackend::tint
                                                ? 2
                                            : call.backend ==
                                                    QueueBackend::alternateBlend
                                                ? 3
                                                : 99;
                                        if (kind == 99)
                                          throw std::runtime_error(
                                              "Unexpected special queue "
                                              "target");
                                        const auto rgb =
                                            kind == 2
                                                ? std::array<unsigned,
                                                             3>{call.values[4],
                                                                call.values[5],
                                                                call.values[6]}
                                                : tints[tint];
                                        return drawOperation(
                                            kind,
                                            {std::int32_t(call.values[1]),
                                             std::int32_t(call.values[2])},
                                            std::int32_t(call.values[3]),
                                            kind == 2 ? 0u : call.values[4],
                                            rgb);
                                      },
                                      [](const auto &) {
                                        throw std::runtime_error(
                                            "Unexpected special auxiliary "
                                            "clip");
                                      },
                                      [](const auto &) {
                                        throw std::runtime_error(
                                            "Unexpected special finalizer");
                                      });
                                  returned = rows[0].metadata;
                                } else
                                  returned =
                                      drawOperation(operation, xy, shade,
                                                    palette, tints[tint]);
#endif
                                const U header[]{
                                    id++,     operation, U(format),
                                    U(shade), palette,   U(empty),
                                    U(split), position,  clipIndex,
                                    tint,     amplitude, cold,
                                    phase,    blend,     word,
                                    provider, returned,  U(calls.size())};
                                output(header, sizeof(header));
                                output(displacement.table.data(), 64);
                                output(calls.data(), calls.size() * 4);
                                output(pixels.data(), pixels.size() * 2);
                              }
  if (id != 26112)
    throw std::runtime_error("Special helper denominator " +
                             std::to_string(id));
#ifdef MNM_ORIGINAL_REFERENCE
  for (const auto &range : original)
    if (std::memcmp(reinterpret_cast<const void *>(range.first),
                    range.second.data(), range.second.size()))
      throw std::runtime_error("Original special helper body changed");
#endif
  return 0;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
