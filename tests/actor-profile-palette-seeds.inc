// Authored storage/program/endpoint schedules; original execution outputs are
// never seeds.
constexpr U PaletteBuilders[] = {0x582440, 0x582a80, 0x583330, 0x583bb0,
                                 0x586660, 0x5870d0, 0x585d80, 0x587e70,
                                 0x588730, 0x588bf0};
constexpr U ProfilePaletteRoots[] = {
    0x504cc0, 0x504ec0, 0x505160, 0x505840, 0x505920, 0x582440, 0x582a80,
    0x583330, 0x583bb0, 0x586660, 0x5870d0, 0x585d80, 0x587e70, 0x588730,
    0x588bf0, 0x5050d0, 0x505220, 0x5055a0, 0x57d310, 0x57d690};
U executeProviders(ProviderMemory &m, U root, U seed) {
  auto w = [&](U p, U value) { m.set32(p, value); };
  auto record = [&](U bank, U entry) {
    return Program + (bank * 16 + entry) * 44;
  };
  auto scalars = [&]() {
    w(Object + 60, seed & 1);
    w(Object + 68, seed & 2 ? 2 : 0);
    w(Object + 16, seed % 7 * 3);
    for (U k = 0; k < 48; ++k)
      w(Object + 0xd8 + k * 4, 1 + (k * 3 + seed) % 17);
  };
  U type = seed % 5;
  for (U off : {0u, 4u, 8u})
    w(0x6dbf00 + type * 0x114 + off, m.get32(0x6dbf00 + off));
  if (root == 0x504cc0) {
    U bank = seed & 32 ? 176 : 0;
    U badBank = bank + seed % 8, bad = seed % 16;
    if (bad == 0)
      w(record(bank, 0), 6);
    if (bad == 1)
      w(record(badBank, 7), 6);
    if (bad == 2)
      w(record(badBank, 3), 1);
    if (bad == 3)
      w(record(badBank, 13), 0);
    if (bad == 4)
      w(record(badBank, 13) + 4, 4);
    if (bad == 5)
      w(record(badBank, 14), 1);
    if (bad == 6)
      w(record(badBank, 14) + 4, 999);
    if (bad == 7)
      w(record(badBank, 15), 0);
    // Optional event5 immediately before event2 is accepted separately from
    // the event5 inside the first12 sprites.
    if (bad == 8) {
      for (U k = 0; k < 12; ++k) {
        w(record(badBank, k), 0);
        w(record(badBank, k) + 4, badBank * 17 + k);
        w(record(badBank, k) + 8, 100 + k * 3);
      }
      w(record(badBank, 12), 5);
      w(record(badBank, 12) + 4, 5);
    }

    return m.invokeFast(root, type, bank);
  }
  if (root == 0x504ec0) {
    U bank = seed & 32 ? 176 : 0, peer = bank + 2 + (seed % 3) * 2,
      choice = seed % 10;
    if (choice == 0)
      w(record(peer, 3), 1);
    if (choice == 1)
      w(record(peer, 6) + 4, 4);
    if (choice == 2)
      w(record(peer, 15), 0);
    if (choice >= 3 && choice <= 7) {
      for (U k = 0; k < 4; ++k) {
        U p = record(bank + k * 2, 4);
        w(p, choice - 2);
        w(p + 4, 11);
      }
      if (seed & 8)
        w(record(peer, 4) + 4, 12);
    }
    if (choice == 8)
      for (U k = 0; k < 4; ++k)
        w(record(bank + k * 2, 3), 99);
    return m.invokeFast(root, type, bank);
  }
  if (root == 0x505160) {
    scalars();
    m.invoke(root, Object, {});
    return 0;
  }
  if (root == 0x505840) {
    scalars();
    return m.invoke(root, Object,
                    {seed % 7, U(int(seed % 3) - 1), U(int((seed / 3) % 3) - 1),
                     U(int((seed / 9) % 3) - 1)});
  }
  if (root == 0x505920) {
    scalars();
    w(Buffer + 128, seed & 8 ? 900 : seed % 13);
    w(Buffer + 132, 0xdeadbeef);
    m.invoke(root, Object,
             {1 + seed % 7, seed % 5, U(int(seed % 3) - 1),
              U(int((seed / 3) % 3) - 1), U(int((seed / 9) % 3) - 1),
              1 + seed * 7, Buffer + 128, Buffer + 132});
    return 0;
  }
  for (U mode = 0; mode < 10; ++mode)
    if (root == PaletteBuilders[mode]) {
      constexpr U counts[] = {2, 3, 4, 5, 8, 16, 32, 64, 128, 256};
      U count = counts[seed % 10];
      if ((mode == 0 || mode == 8) && count == 5)
        count = 4;
      if (mode == 8 && count == 3)
        count = 4;
      w(Object, Other + 0x1100);
      w(Other + 0x1110, 1 + seed % 4);
      w(Object + 4, Rgb);
      w(0x6e1f88, seed & 16);
      if (seed & 32) {
        U old = m.invoke(0x597880, 0, {32});
        w(Object + 16, old);
      }
      U slot = Object + 16;
      while (m.get32(slot))
        slot = m.get32(slot);
      if (seed % 16 == 15)
        m.failAllocation = m.allocations + 1;
      U result = m.fastcall(root, count, slot, {Rgb});
      w(Other + 0x1b80, m.invoke(0x57d690, Object, {mode, count}));
      return result;
    }
  if (root == 0x5050d0)
    return m.invoke(root, type, {});
  if (root == 0x505220) {
    w(Object + 60, seed & 1);
    w(Object + 0x74, seed & 2 ? 0 : ~0u);
    w(Object + 48, seed % 7);
    m.invoke(root, Object, {type});
    return 0;
  }
  if (root == 0x5055a0) {
    w(0x6dbf00, 0);
    w(0x6dbf04, 0);
    w(0x6dbf08, 0);
    w(0x6e9808, 1);
    w(Object + 0x74, ~0u);
    if (seed % 16 == 15)
      m.failAllocation = 5;
    return m.invoke(root, Object, {0, Buffer});
  }
  if (root == 0x57d310) {
    w(0x6e1f88, seed & 16);
    if (seed % 16 == 15)
      m.failAllocation = 2;
    return m.invoke(root, Object, {Buffer + 32, 16});
  }
  if (root == 0x57d690) {
    w(Object, Other + 0x1100);
    w(Other + 0x1110, seed % 5);
    w(Object + 4, Rgb);
    w(0x6e1f88, seed & 16);
    if (seed % 16 == 15)
      m.failAllocation = 2;
    return m.invoke(root, Object, {seed % 12, 16});
  }
  throw std::runtime_error("Unknown profile/palette case");
}
void releaseProviderChains(ProviderMemory &m, U root) {
  U resource = 0;
  for (U entry : PaletteBuilders)
    if (root == entry)
      resource = Object;
  if (root == 0x57d310 || root == 0x57d690)
    resource = Object;
  if (root == 0x5055a0)
    resource = 0x6de0e8;
  if (resource)
    for (U k = 0; k < 4; ++k)
      if (U chain = m.get32(resource + 16 + k * 4)) {
        m.invoke(0x57d0d0, 0, {chain});
        m.set32(resource + 16 + k * 4, 0);
      }
}
