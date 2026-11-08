#pragma once
#include <cstdint>

namespace mnm::reconstruction::rendering {
struct DcImageResult {
  std::uint32_t eax, ecx, firstArgument;
};
// NoCD 58d1a0/58d240/58d280/58d370. Named COM/image suppliers execute their
// own pixel and platform contracts; this model recovers only caller ordering,
// normal-return ownership and meaningful EAX. It deliberately retains ignored
// GetDC/image/ReleaseDC errors and the two unguarded null-surface paths.
template <class Memory>
DcImageResult dcImageWrapper(Memory &m, std::uint32_t entry, std::uint32_t self,
                             std::uint32_t source, std::uint32_t x,
                             std::uint32_t y,
                             std::uint32_t entrySeh = 0xffffffffu) {
  using U = std::uint32_t;
  const bool load = entry == 0x58d1a0 || entry == 0x58d280;
  const bool positioned = entry == 0x58d280 || entry == 0x58d370;
  // In the load variants GetDC reuses the caller's path argument slot. In
  // draw variants its initial contents are the saved incoming ECX receiver.
  U dc = m.scratch(4);
  m.set32(dc, load ? source : self);
  U image = load ? m.scratch(12) : source;
  if (load) {
    m.call(0x486950, image, 0, {});
    if (!m.call(0x486ae0, image, 0, {source})) {
      m.call(0x486960, image, 0, {});
      return {0, entrySeh, source};
    }
  }
  U surface = m.get32(self + 8), result = 0;
  if (positioned || surface) {
    m.com(surface, 0x44, {surface, dc});
    m.call(0x486a80, image, 0, {dc, positioned ? x : 0u, positioned ? y : 0u});
    // The image supplier may change the wrapper surface and the DC value.
    surface = m.get32(self + 8);
    result = m.com(surface, 0x68, {surface, m.get32(dc)});
  }
  if (load) {
    m.call(0x486960, image, 0, {});
    return {1, entrySeh, m.get32(dc)};
  }
  // The saved incoming ECX is also the GetDC output slot. POP ECX restores
  // the resulting HDC (including a supplier rewrite), not necessarily self.
  return {result, m.get32(dc), source};
}
} // namespace mnm::reconstruction::rendering
