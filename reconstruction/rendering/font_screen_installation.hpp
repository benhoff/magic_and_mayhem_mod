#pragma once
#include <cstdint>
#include <initializer_list>

namespace mnm::reconstruction::rendering {
// NoCD 5575b0, normal returns from named suppliers. The original coordinator
// reloads resources; it does not retire the previous font allocations.
// Platform, image/font loading, fatal-error and widget bodies remain explicit
// suppliers.
template <class Memory> void installScreenFonts(Memory &m, std::uint32_t self) {
  using U = std::uint32_t;
  const U path = m.scratch(256), rect = m.scratch(16);
  auto call = [&](U entry, U receiver, std::initializer_list<U> args = {},
                  U second = 0) {
    return m.call(entry, receiver, second, args);
  };
  auto load = [&](U entry, U object, U name) {
    if (!call(entry, object, {name, 2}))
      call(0x49e4e0, name);
  };
  // Resolution is reread after the first load/error supplier, rather than
  // cached for the complete installation.
  call(0x59c1fd, 0, {path, 0x5e0690, m.get32(0x6de6d5) ? 800u : 640u});
  load(0x4a5ae0, 0x6b0400, path);
  m.set32(0x6b0630, 2);
  call(0x59c1fd, 0, {path, 0x5e0674, m.get32(0x6de6d5) ? 800u : 640u});
  load(0x4a5ae0, 0x656680, path);
  m.set32(0x6568b0, 2);
  load(0x4a5ae0, 0x6df1c0, 0x5ec0e4);
  m.set32(0x6df3f0, 2);
  load(0x4a5ae0, 0x6a5138, 0x5ec0cc);
  m.set32(0x6a5368, 2);
  load(0x57d310, 0x6f3148, 0x5ec0b8);
  // Unlike the preceding five loads, this sprite result is not tested.
  call(0x57d310, 0x6e2108, {0x5ec0a4, 2});
  call(0x4dba20, 0x657fe8, {0x6df1c0});
  call(0x4db650, 0x657fe8);
  call(0x4dc0f0, 0x657fe8, {0});
  call(0x4d8630, 0x6ddf78, {0x6a5138});
  call(0x4d85f0, 0x6ddf78);
  call(0x4d82f0, 0x6ddf78);
  call(0x49c280, 0x689498);
  if (m.get32(0x642c64)) {
    call(0x4a30b0, 0);
    call(0x534aa0, self + 0x12, {20, 5});
    call(0x534ad0, self + 0x12, {0x4eaaf0});
  }
  if (!call(0x502b20, 0x5ec084, {0x6a5f80}, 0x5ec098))
    call(0x49e4e0, 0x5ec084);
  call(0x54d070, 0);
  U table =
      (m.get32(0x6de6d5) ? 0x5e1720u : 0x5e1700u) + (m.get32(0x6de6d9) << 4);
  m.set32(0x6c482c, table);
  call(0x4f7860, 0x689930, {0x6c5490, 0x6a49e0, 0x6de558, table, 0});
  call(0x4db4f0, 0x6c5268, {0x656680});
  call(0x4dad50, 0x6c5289);
  call(0x4d9360, 0x6de8a0, {1});
  call(0x554830, 0x689ba1);
  call(0x4d5e50, 0x6a2dd0, {1});
  call(0x4d3690, 0x6c4e61);
  call(0x4da200, 0x6a26c8);
  call(0x4dc830, 0x6c4630);
  U thread = call(0x5c52e8, 0);
  call(0x54e170, 0x6574c8, {thread});
  m.set32(self + 0x26, 0);
  if (!m.get8(0x6de6c5)) {
    m.set32(self + 0x22, 0x657ce0);
    return;
  }
  call(0x4d28b0, 0x6f3470, {0x6b0400});
  call(0x4d2960, 0x6f3470, {0x6a49e0});
  U owner = m.get32(0x6de6c0);
  U text =
      call(0x58a680, 0x5fd7c8, {owner && (m.get8(owner + 4) & 2) ? 14u : 15u});
  call(0x4d28c0, 0x6f3470, {text});
  // The same resolution read supplies both dimensions at this point.
  U high = m.get32(0x6de6d5);
  m.set32(rect, 0);
  m.set32(rect + 4, 0);
  m.set32(rect + 8, high ? 800 : 640);
  m.set32(rect + 12, high ? 600 : 480);
  call(0x4d2970, 0x6f3470, {4});
  for (U at : {0x6f348du, 0x6f3491u, 0x6f3495u})
    m.set32(at, 200);
  call(0x4d2930, 0x6f3470, {rect});
  call(0x4d2800, 0x6f3470);
  call(0x58bf40, 0x6a49e0, {0x6a4fe0, 0, 0});
  for (U i = 0;; ++i) {
    const auto byte = m.get8(0x5ec060 + i);
    m.set8(0x6ddd58 + i, byte);
    if (!byte)
      break;
  }
  call(0x4ca370, 0x6ddd58);
}
} // namespace mnm::reconstruction::rendering
