// Authored state transitions are common inputs, never original observations.
static constexpr U FamilyRoots[] = {0x5468a0, 0x547210, 0x5472c0,
                                    0x548bc0, 0x54a140, 0x54aa30};
static constexpr U FamilyManager = 0x6cbb8a, FamilyGrid = FamilyArea,
                   FamilyGridOld = FamilyArea + 0x400,
                   FamilyShade = FamilyArea + 0x800,
                   FamilyDescriptors = FamilyArea + 0x1000,
                   FamilyFrames = FamilyArea + 0x2000;
static void needFamily(bool v) {
  if (!v)
    throw std::runtime_error("Family ownership assertion");
}
static void initializeFamilyCase(FamilyMemory &m, U seed) {
  m.set32(0x5eb480, 128);
  m.set32(0x5eb484, 96);
  m.set32(0x5eb488, 64);
  m.set32(0x6e1f88, (seed >> 3) & 1);
  m.set32(0x5e1424, seed % 3 == 0 ? 100 : seed % 3 == 1 ? 0 : 40);
  m.set32(0x5e1428, 8 + seed % 5);
  constexpr U dimensions[] = {0, 2, 3, 17};
  m.set32(0x5e142c, dimensions[seed % 4]);
  const float high = 1.3f, low = .7f;
  std::memcpy(m.pointer(0x5e1434, 4), &high, 4);
  std::memcpy(m.pointer(0x5e1438, 4), &low, 4);
  m.set32(0x5e1430, (seed >> 2) & 1);
  m.set32(0x5e1464, 2 + seed % 3);
  m.set32(0x5e1468, 8);
  m.set32(0x5e146c, 2 + seed % 3);
  m.set32(0x5e1470, 1 + seed % 3);
  m.set32(0x5e1474, 25);
  m.set32(0x6e1f5c, seed % 3);
  for (U k = 0; k < 4; ++k)
    m.set32(0x5e149c + k * 4, k == 3 ? 4 : 32 + k * 30);
  for (U p : {0x5e1494u, 0x5e14b0u}) {
    m.set32(p, seed % 3 == 0 ? 0 : seed % 3 == 1 ? 2 : 17);
    m.set32(p + 4, seed % 3 == 0 ? 17 : 2);
  }
  std::memset(m.pointer(FamilyManager, 0x1c08), 0, 0x1c08);
  mnm::reconstruction::Kind8Random random(seed * 37 + 1);
  m.set32(0x6574c8, random.first);
  m.set32(0x6574cc, random.second);
  for (U k = 0; k < 250; ++k)
    m.set32(0x6574d0 + k * 4, random.words[k]);
  for (U k = 0; k < 4; ++k) {
    U actor = Pool + k * 0xe4b;
    m.set32(actor + 0xac, Catalog);
    m.set32(actor + 0xe4, 1);
    m.set32(actor + 0x14, U(-12));
    m.set32(actor + 0x18, 97);
    m.set32(actor + 0x1c, 7);
    m.set32(actor + 0x798, 1);
    m.set32(actor + 0x110, 1);
  }
  m.set32(Catalog + 0x198, 3);
  m.set32(Catalog + 0x19c, 5);
  for (U k = 0; k < 4; ++k)
    m.set32(0x6cb8c2 + k * 4, k * 12);
  for (U k = 0; k < 3; ++k)
    m.set32(0x6cb942 + k * 4, k * 4);
  m.set32(0x6c54dc, FamilyGrid);
  m.set32(0x6c5c5c, FamilyShade);
  m.set32(0x65660c, FamilyDescriptors);
  for (U g : {FamilyGrid, FamilyGridOld})
    for (U k = 0; k < 48; ++k) {
      m.set16(g + k * 12, 1 + k % 2);
      m.set16(g + k * 12 + 4, seed & 16 ? 0 : 0xffff);
    }
  for (U k = 0; k < 48; ++k)
    m.set8(FamilyShade + k, U(int(k % 3) * 63 - 63));
  if (seed & 8)
    std::memset(m.pointer(FamilyDescriptors + 356, 712), 0xff, 712);
  for (U bank = 0; bank < 3; ++bank) {
    U area = FamilyFrames + bank * 0x1000;
    m.set32(area + 12, bank == 2 ? 27 : 18);
    for (U k = 0; k < 64; ++k) {
      m.set32(area + 0x100 + k * 4, k * 40);
      m.set32(area + 0x300 + k * 40 + 4, k + 1);
      m.set32(area + 0x300 + k * 40 + 8, k + 2);
    }
    U base = bank == 0 ? 0x6f1d58 : bank == 1 ? 0x6f2240 : 0x6f2218;
    m.set32(base, area);
    m.set32(base + 8, area + 0x100);
    m.set32(base + 12, area + 0x300);
  }
  m.set32(0x6f2174, 2);
  m.set32(0x6f2148, 2);
  m.set32(0x6f220c, 3);
  m.set32(0x6a5f88, seed & 32 ? 2 : 1);
}
static void checkFamilyOwnership(FamilyMemory &m) {
  U count = m.get32(FamilyManager + 0x1c00);
  needFamily(count <= 0x700);
  std::set<U> owned;
  for (U k = 0; k < count; ++k) {
    U p = m.get32(FamilyManager + k * 4);
    needFamily(p && owned.insert(p).second);
    U table = m.get32(p);
    if (table == 0x5c7494 || table == 0x5c74b0 || table == 0x5c752c) {
      U plane = m.get32(p + 0x2e);
      if (plane)
        needFamily(owned.insert(plane).second);
    }
  }
  for (const auto &b : m.blocks) {
    needFamily(b.alive == (owned.count(b.at) != 0));
    for (U k = 0; k < 16; ++k)
      needFamily(m.get8(b.at - 16 + k) == 0xa7 &&
                 m.get8(b.at + b.size + k) == 0x7a);
  }
}
static U executeFamily(FamilyMemory &m, U root, U seed) {
  auto w = [&](U p, U v) { m.set32(p, v); };
  bool full = seed == 127 && (root == 0x5468a0 || root == 0x54a140);
  U fillers = full ? 1791 : seed % 3;
  for (U k = 0; k < fillers; ++k) {
    const bool low = full && root == 0x5468a0;
    U p = m.invoke(0x597850, 0, {low ? 100u : 84u});
    needFamily(p);
    if (low) {
      m.invoke(0x545650, p, {30, 30, 48, 0, 0, 0});
      w(p, 0x5c7408);
      const U fields[][2] = {{0x1c, 128}, {0x20, 128},
                             {0x24, 32},  {0x28, 5},
                             {0x2c, 5},   {0x30, 10000},
                             {0x34, 2},   {0x38, 2},
                             {0x3c, 16},  {0x40, m.get32(0x6e1f88) ? 16u : 32u},
                             {0x44, 4},   {0x54, 0},
                             {0x58, 0},   {0x5c, 0},
                             {0x60, 0}};
      for (const auto &f : fields)
        w(p + f[0], f[1]);
    } else {
      m.invoke(0x547210, p, {30, 30, 48});
      w(p + 0x10, 0);
    }
    needFamily(m.invoke(0x5457e0, FamilyManager, {p}));
  }
  U n = root == 0x5468a0                       ? 69
        : root == 0x547210 || root == 0x5472c0 ? 84
        : root == 0x548bc0                     ? 65
        : root == 0x54a140                     ? 130
                                               : 50;
  U p = m.invoke(0x597850, 0, {n});
  needFamily(p);
  if (root == 0x5468a0)
    m.invoke(0x546660, p, {31, 31, 48});
  else if (root == 0x547210)
    m.invoke(root, p, {31, 31, 48});
  else if (root == 0x5472c0)
    m.invoke(root, p, {31, 31, 48, 224, 128, 32, 4});
  else if (root == 0x548bc0)
    m.invoke(0x548ac0, p, {31, 31, 48, U(-2), 1, 0});
  else if (root == 0x54a140)
    m.invoke(0x549d70, p, {0, seed % 6, 31, 31, 48, 1, 1, U(-1), 0});
  else
    m.invoke(0x548560, p, {31, 31, 48, 1, U(-1), seed % 3});
  if (root == 0x5468a0) {
    w(p + 0x18, U(-2));
    if (full)
      w(p + 0x2c, 6);
    m.set8(p + 0x44, seed & 1);
  }
  if (root == 0x547210 || root == 0x5472c0) {
    w(p + 0x10, seed % 4);
    w(p + 0x40, seed % 3);
    w(p + 0x2c, U(-3));
    if (seed & 4) {
      w(p + 0x44, Pool + 2 * 0xe4b);
      for (U k = 0; k < 3; ++k)
        w(p + 0x48 + k * 4, (seed >> k) & 1);
    }
  }
  if (root == 0x548bc0) {
    w(p + 0x10, 4 + seed % 3);
    w(p + 0x14, 1 + seed % 3);
    w(p + 0x18, seed & 2 ? U(-1) : 0);
    w(p + 0x38, seed & 1 ? 4 : 0);
  }
  if (root == 0x54a140) {
    w(p + 0x5e, seed % 4);
    w(p + 0x62, 224);
    w(p + 0x66, 128);
    w(p + 0x6a, 32);
    w(p + 0x6e, seed & 1 ? 4 : 0);
  }
  needFamily(m.invoke(0x5457e0, FamilyManager, {p}));
  U before = m.allocations, failBefore = m.allocationFailures;
  if (seed % 4 == 0)
    m.failAllocation = before + 1;
  if (seed % 4 == 1)
    m.failAllocation = before + 2;
  checkFamilyOwnership(m);
  m.snapshotProviders(root, 0, 2);
  for (U frame = 1; frame <= 32; ++frame) {
    if (frame == 2) {
      w(Pool + 2 * 0xe4b + 0x14, 129);
      w(Catalog + 0x198, 5);
      w(0x6c54dc, FamilyGridOld);
    }
    if (frame == 3) {
      w(Pool + 2 * 0xe4b + 0xe4, 0);
      m.set8(FamilyShade + 13, 127);
    }
    if (frame == 4) {
      w(Pool + 2 * 0xe4b + 0xe4, 1);
      w(Pool + 2 * 0xe4b + 0x798, 0);
      w(Pool + 2 * 0xe4b + 0x110, 0);
    }
    // Caller draw-state inputs: actual flag nonraster virtual method, and the
    // additive raster draw's recovered byte flag as input (no raster claim).
    U count = m.get32(FamilyManager + 0x1c00);
    for (U k = 0; k < count; ++k) {
      U token = m.get32(FamilyManager + k * 4), table = m.get32(token);
      if (table == 0x5c74cc && frame % 2 == 0)
        m.invoke(0x54ac00, token, {});
      if (table == 0x5c7420 && frame % 2 == 0)
        m.set8(token + 0x44, 1);
    }
    std::vector<unsigned char> actors(m.pointer(Pool, 4 * 0xe4b),
                                      m.pointer(Pool, 4 * 0xe4b) + 4 * 0xe4b),
        descriptor(m.pointer(Catalog, 0x200),
                   m.pointer(Catalog, 0x200) + 0x200),
        world(m.pointer(FamilyArea, 0x10000),
              m.pointer(FamilyArea, 0x10000) + 0x10000);
    m.invoke(0x545910, FamilyManager, {});
    needFamily(!std::memcmp(actors.data(), m.pointer(Pool, 4 * 0xe4b),
                            actors.size()) &&
               !std::memcmp(descriptor.data(), m.pointer(Catalog, 0x200),
                            descriptor.size()) &&
               !std::memcmp(world.data(), m.pointer(FamilyArea, 0x10000),
                            world.size()));
    for (U k = 0; k < m.get32(FamilyManager + 0x1c00); ++k) {
      U token = m.get32(FamilyManager + k * 4), table = m.get32(token);
      for (U slot : {4u, 8u, 20u})
        m.events.push_back(m.invoke(m.get32(table + slot), token, {}));
    }
    checkFamilyOwnership(m);
    if (frame <= 4)
      m.snapshotProviders(root, frame, frame + 2);
  }
  U evicted = 0;
  if (full)
    for (U k = 0; k < fillers; ++k)
      if (!m.blocks[k].alive)
        ++evicted;
  if (full && root == 0x5468a0)
    needFamily(evicted > 0);
  if (full && root == 0x54a140)
    needFamily(evicted == 0);
  U planes = 0;
  for (const auto &b : m.blocks)
    if (b.size == 24 || b.size == 54 || b.size == 1536)
      ++planes;
  U failed = m.allocationFailures - failBefore;
  std::fprintf(stderr,
               "family_outcome %x %u created %u failed %u full %u planes %u "
               "evicted %u\n",
               root, seed, m.allocations - before - failed, failed, U(full),
               planes, evicted);
  return 0;
}
static void releaseFamilyCase(FamilyMemory &m, U) {
  m.invoke(0x5458b0, FamilyManager, {});
  checkFamilyOwnership(m);
  auto n = m.events.size();
  m.invoke(0x5458b0, FamilyManager, {});
  needFamily(m.events.size() == n);
}
