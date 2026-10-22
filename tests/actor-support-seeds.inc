// Independent initialized map inputs; never derived from original output.
static constexpr U SupportRoots[] = {0x4f3320, 0x4f4330};
static constexpr U SupportSelf = Base, SupportParameters = Other + 0x1000;
static constexpr U SupportCells = 1024;
static void initializeSupportCase(SupportMemory &m, U) {
  for (U k = 0; k < 0x6600; ++k)
    m.set8(SupportSelf + k, (k * 29 + 13) & 255);
  m.set32(0x6c5494, 6);
  m.set32(0x6c5498, 5);
  m.set32(SupportSelf + 4, 6);
  m.set32(SupportSelf + 8, 5);
  m.set32(SupportSelf + 0x10, 90);
  const U rows[] = {5, 17, 31, 43, 57}, layers[] = {19, 100, 221, 333, 447, 550};
  for (U k = 0; k < 5; ++k)
    m.set32(SupportSelf + 0x64b2 + k * 4, rows[k]);
  for (U k = 0; k < 6; ++k)
    m.set32(SupportSelf + 0x6432 + k * 4, layers[k]);
}
static U typedSupport(SupportMemory &m, U root, U x, U y, U z, U cellBase,
                      U terrainBase, U terrainId) {
  namespace pf = mnm::reconstruction;
  std::vector<pf::OccupancyCell> cells(SupportCells);
  std::memcpy(cells.data(), m.pointer(cellBase, SupportCells * 12), SupportCells * 12);
  std::vector<pf::TerrainValidityRecord> terrain(terrainBase ? terrainId + 1 : 0);
  if (terrainBase)
    std::memcpy(&terrain[terrainId], m.pointer(terrainBase, 0x164), 0x164);
  std::int32_t rows[5], layers[6];
  for (U k = 0; k < 5; ++k)
    rows[k] = int(m.get32(SupportSelf + 0x64b2 + k * 4));
  for (U k = 0; k < 6; ++k)
    layers[k] = int(m.get32(SupportSelf + 0x6432 + k * 4));
  pf::CellValidityMapView view{{{6, 5, 6}, cells.data(), cells.size(), rows, 5,
                              layers, 6, int(m.get32(SupportSelf + 0x10)), 6},
                             terrain.data(), terrain.size()};
  pf::CreatureMovementParameters parameters;
  std::memcpy(&parameters, m.pointer(SupportParameters, 22), 22);
  pf::Coordinates pos{int(std::int32_t(x)), int(std::int32_t(y)), int(std::int32_t(z))};
  return root == 0x4f3320
             ? pf::test_cell_support(pos, parameters, view)
             : pf::query_record(pos, parameters,
                 pf::with_cell_support({{6, 5, 6}, [&] {
                   return int(std::int32_t(m.get32(SupportSelf + 0x7f0)));
                 }, {}}, view));
}
static U executeSupport(SupportMemory &m, U root, U seed) {
  U a = m.invoke(0x597880, 0, {SupportCells * 12}),
    b = m.invoke(0x597880, 0, {SupportCells * 12}),
    record = m.invoke(0x597880, 0, {0x164});
  U terrainId = seed % 128 == 126 ? 32768 : seed % 128 == 127 ? 65535 : 1;
  for (U k = 0; k < SupportCells; ++k) {
    U code = (k * 37 + seed * 13) % 16;
    U flags[] = {0, 0x10, 0x20, 0x30, 0x80, 0x4000, 0x4020, 0x4030};
    for (U base : {a, b}) {
      for (U j = 0; j < 12; ++j)
        m.set8(base + k * 12 + j, k * 17 + j * 13 + seed);
      m.set16(base + k * 12, code & 1 ? terrainId : 0);
      m.set16(base + k * 12 + 10, flags[(code + (base == b ? 3 : 0)) % 8]);
    }
  }
  for (U k = 0; k < 0x164; ++k)
    m.set8(record + k, k * 31 + seed);
  const U classes[] = {0, 15, 16, 17, 0xffffffff};
  m.set32(record + 0x94, classes[(seed / 16) % 5]);
  m.set8(record + 0xb0, seed & 256 ? 8 : seed % 256);
  for (U k = 0; k < 22; ++k)
    m.set8(SupportParameters + k, seed * 7 + k * 3);
  m.set32(SupportParameters + 4, seed % 3 == 0 ? 1 : 2);
  m.set32(SupportParameters + 12, seed & 8 ? 2 : 0);
  m.set8(SupportParameters + 16, seed & 256 ? 0xff : 0);
  m.set32(SupportParameters + 17, seed & 64 ? 22 : 0xfffffffe);
  m.set32(SupportSelf + 0x7f0, (seed / 8) % 5);
  m.set32(0x65660c, record - terrainId * 0x164);
  m.set32(SupportSelf + 0x4c, a);
  U x = seed % 6, y = (seed / 6) % 5, z = seed % 6;
  if (root == 0x4f4330 && m.get32(SupportParameters + 4) != 1) {
    const int xs[] = {-13, -1, 0, 5, 6, 19}, ys[] = {-11, -1, 0, 4, 5, 16};
    x = U(xs[seed % 6]); y = U(ys[(seed / 6) % 6]);
    if (seed % 31 == 30) z = U(-1);
  }
  // Direct current/below rules cover every low flag byte in each special-byte
  // bank. The fallback deliberately uses stride rather than layer[z-1].
  if (root == 0x4f3320) {
    U at = m.get32(SupportSelf + 0x64b2 + y * 4) +
           m.get32(SupportSelf + 0x6432 + z * 4) + x;
    m.set16(a + at * 12, seed & 2 ? terrainId : 0);
    m.set16(a + at * 12 + 10, (seed & 255) | (seed & 512 ? 0x4000 : 0));
    if (z) {
      m.set16(a + (at - 90) * 12, seed & 4 ? terrainId : 0);
      m.set16(a + (at - 90) * 12 + 10, (255 - (seed & 255)) | (seed & 32 ? 0x4000 : 0));
    }
  }
  auto check = [&](U phase, U cells, U terrain) {
    U expected = typedSupport(m, root, x, y, z, cells, terrain, terrainId);
    std::vector<unsigned char> before(m.pointer(SupportSelf, 0x6600),
                                      m.pointer(SupportSelf, 0x6600) + 0x6600);
    std::vector<unsigned char> acopy(m.pointer(a, SupportCells * 12), m.pointer(a, SupportCells * 12) + SupportCells * 12),
      bcopy(m.pointer(b, SupportCells * 12), m.pointer(b, SupportCells * 12) + SupportCells * 12),
      rcopy(m.pointer(record, 0x164), m.pointer(record, 0x164) + 0x164);
    U allocations = m.allocations, events = U(m.events.size());
    U result = m.invoke(root, SupportSelf, {x, y, z, SupportParameters});
    if (result != expected || m.allocations != allocations || m.events.size() != events ||
        std::memcmp(before.data(), m.pointer(SupportSelf, 0x6600), before.size()) ||
        std::memcmp(acopy.data(), m.pointer(a, SupportCells * 12), acopy.size()) ||
        std::memcmp(bcopy.data(), m.pointer(b, SupportCells * 12), bcopy.size()) ||
        std::memcmp(rcopy.data(), m.pointer(record, 0x164), rcopy.size()))
      throw std::runtime_error("Support result/read-only ownership mismatch");
    m.events.insert(m.events.end(), {0x53555031, phase, result, x, y, z, cells});
    return result;
  };
  U result = check(0, a, record);
  // Caller-controlled mutation, rebind, parameter update, no-terrain bypass,
  // then reuse the first backing; each query must preserve all borrowed bytes.
  m.set16(a + 100 * 12 + 10, 0x4000);
  check(1, a, record);
  m.set32(SupportSelf + 0x4c, b); check(2, b, record);
  m.set8(SupportParameters + 16, 1 - (seed & 1)); check(3, b, record);
  for (U k = 0; k < SupportCells; ++k) {
    m.set16(b + k * 12, 0); m.set16(b + k * 12 + 10, seed & 1 ? 0x4000 : 0);
  }
  m.set32(0x65660c, 0); check(4, b, 0);
  m.set32(SupportSelf + 0x4c, a);
  m.set32(0x65660c, record - terrainId * 0x164); check(5, a, record);
  return result;
}
static void releaseSupportCase(SupportMemory &, U) {}
