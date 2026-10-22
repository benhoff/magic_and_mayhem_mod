// Independently authored inputs. No original execution output seeds native
// runs.
constexpr U FrontierRoots[] = {0x4644d0, 0x488330, 0x493f20, 0x4eb820,
                               0x4f4330, 0x5050d0, 0x505220, 0x50d060,
                               0x50e400, 0x57d310, 0x57d690, 0x58c8a0,
                               0x414890, 0x4637c0, 0x5055a0};
U executeFrontier(FrontierMemory &m, U root, U seed) {
  constexpr U Effect = Base + 0x8000,
              Cell = Map + ((1 * 36) + (2 * 6) + 2) * 12;
  auto w = [&](U p, U v) { m.set32(p, v); };
  auto h = [&](U p, U v) { m.set16(p, v); };
  auto effects = [&]() {
    w(Manager, Effect);
    w(Manager + 4, 4);
    w(0x689498, Effect);
    w(0x68949c, 4);
    for (U k = 0; k < 4; ++k) {
      U p = Effect + k * 0x22e;
      w(p, k);
      w(p + 36, Manager);
      h(p + 0x1a0, 0xffff);
      w(p + 8, 2);
      w(p + 12, 2);
      w(p + 16, 1);
    }
  };
  auto surfaces = [&](U src, U dst) {
    U table = Other + 0x1800, sourceHandle = Other + 0x1600,
      destHandle = Other + 0x1640;
    w(src + 8, sourceHandle);
    w(dst + 8, destHandle);
    w(sourceHandle, table);
    w(destHandle, table);
    w(table + 20, 0x1001f540);
    w(table + 28, 0x1001f580);
    w(table + 0x6c, 0x1001f5c0);
    w(table + 0x74, 0x1001f600);
    w(0x6f68d8, seed & 1);
    w(0x6f98c0, dst);
    w(0x6f98d8, seed & 2);
    w(0x6de6dd, seed & 4);
    w(0x6f98d0, seed & 32 ? 0 : Other + 0x1680);
    w(Other + 0x1680, Other + 0x1700);
    w(Other + 0x1710, 0x1001f500);
    w(src + 40, seed % 3 == 0 ? 0 : seed % 3 == 1 ? ~0u : Other + 0x1780);
    w(dst + 40, seed % 3 == 2 ? Other + 0x1780 : ~0u);
    w(Other + 0x1780, Other + 0x17c0);
    w(Other + 0x17c0, 0x1001f640);
    h(src + 44, 0x7b);
    h(dst + 44, 0x456);
  };
  switch (root) {
  case 0x4644d0: {
    for (U off : {0u, 4u, 8u})
      w(Object + off, m.invoke(0x597880, 0, {32 + off}));
    w(Object + 12, seed & 8);
    w(Object + 0x110, Other + 0x1100);
    return m.invoke(root, Object, {Buffer, 0});
  }
  case 0x57d310: {
    for (U off : {0u, 32u})
      w(Object + off, m.invoke(0x597880, 0, {48}));
    for (U k = 0; k < 4; ++k) {
      U a = m.invoke(0x597880, 0, {20}), b = m.invoke(0x597880, 0, {20});
      w(Object + 16 + k * 4, a);
      w(a, b);
      w(b, 0);
    }
    if (seed % 16 == 10)
      m.failAllocation = m.allocations + 1;
    return m.invoke(root, Object, {Buffer + 32, seed & 8 ? 0u : 16u});
  }
  case 0x57d690: {
    w(Object, Other + 0x1100);
    w(Other + 0x1110, seed % 5);
    w(Object + 4, Base + 0x31000);
    for (U k = 0; k < 3072; ++k)
      m.set8(Base + 0x31000 + k, k * 13 + seed);
    if (seed & 16) {
      U node = m.invoke(0x597880, 0, {20});
      w(Object + 16, node);
    }
    return m.invoke(root, Object, {seed % 12, seed % 17});
  }
  case 0x488330: {
    effects();
    U self = Effect + 0x22e;
    U kind = seed % 5;
    h(Cell + 2, kind == 0 ? 1 : kind == 1 ? 0 : 0xffff);
    if (kind == 1)
      h(Effect + 0x1a0, 1);
    if (kind == 2)
      h(Cell + 2, 0);
    if (seed & 8)
      h(Cell, 1);
    if (seed & 16)
      m.set8(Cell + 11, 32);
    m.invoke(root, self, {Cell});
    return 0;
  }
  case 0x493f20: {
    effects();
    U self = Effect + 0x22e, source = Buffer + 256;
    constexpr U types[] = {0, 9, 11, 21, 30, 31, 32, 88, 89, ~0u};
    U type = types[seed % 10];
    for (U k = 0; k < 63; ++k)
      w(source + k * 4, 0);
    w(source, seed & 1 ? U(-3) : 67);
    w(source + 4, 81);
    w(source + 8, seed & 2 ? 99 : 21);
    w(source + 12, 98);
    w(source + 16, seed & 4 ? U(-2) : 66);
    w(source + 20, 35);
    w(source + 24, seed & 16 ? 1 : ~0u);
    w(source + 28, seed & 4 ? 1 : ~0u);
    if (seed & 32) {
      h(Cell + 2, 0);
      m.set8(Cell + 11, seed & 4 ? 64 : 0);
    }
    m.invoke(root, self, {type, source});
    return 0;
  }
  case 0x4eb820: {
    h(Cell, seed % 3);
    m.set16(Cell + 10, 0xabff);
    m.invoke(root, Cell, {seed % 11});
    return 0;
  }
  case 0x4f4330:
    w(Other + 4, seed & 1 ? 1 : 2);
    w(Other + 12, seed & 4 ? 2 : 0);
    w(Object + 0x7f0, seed % 3);
    return m.invoke(root, Object,
                    {seed & 32  ? U(-13)
                     : seed & 8 ? U(-1)
                                : 5,
                     seed & 32   ? 19
                     : seed & 16 ? U(-1)
                                 : 5,
                     seed % 4, Other});
  case 0x414890:
    w(Catalog + 8, seed & 1 ? 1 : 2);
    w(Catalog + 12, 2);
    w(Catalog + 0x44, seed & 4 ? 2 : 0);
    w(Object + 0x7f0, seed % 3);
    return m.invoke(root, Object,
                    {seed & 32  ? U(-13)
                     : seed & 8 ? U(-1)
                                : 5,
                     seed & 32   ? 19
                     : seed & 16 ? U(-1)
                                 : 5,
                     seed % 4});
  case 0x5050d0: {
    constexpr U counts[] = {9, 17, 185, 265};
    w(Other + 0x1014, counts[seed % 4]);
    return m.invoke(root, 0, {});
  }
  case 0x505220:
    w(Object + 60, seed & 1);
    w(Object + 0x74, seed & 2 ? 0 : ~0u);
    w(Object + 48, seed % 7);
    m.invoke(root, Object, {0});
    return 0;
  case 0x50d060: {
    effects();
    w(Object + 0x5ec, seed % 17);
    w(Object + 0x5fc, seed & 1 ? 10 : 20);
    w(Object + 0xa0, seed % 7 == 0 ? 3 : seed % 5 == 0 ? 1 : 0);
    w(Object + 0xa8, seed % 16 == 0 ? 27 : 17);
    w(Object + 0xe4, seed % 13 ? 100 : 0);
    w(Object + 0x798, seed % 8 == 1);
    w(Object + 0x108, seed % 8 == 2);
    w(Object + 0x664, seed & 8);
    w(Object + 0x64c, 1);
    w(Object + 0x6a8, 2);
    w(Object + 0x68, seed & 4);
    if (seed & 16) {
      w(Object + 0x6e8, Effect);
      w(Other + 0x908, 1);
      w(Nodes, Nodes + 64);
      w(Nodes + 64, Nodes);
      w(Nodes + 72, 2);
      w(Nodes + 104, Effect + 0x22e);
    }
    m.invoke(root, Object,
             {seed % 4 ? seed % 11 : U(-2), seed % 4, seed & 32 ? 1u : 0u});
    return 0;
  }
  case 0x50e400: {
    effects();
    w(Catalog + 8, seed & 32 ? 1 : 2);
    w(Object + 8, 2);
    w(Object + 12, 2);
    w(Object + 0x60c, seed % 8);
    w(Object + 0x96b, seed % 5);
    w(Object + 0x96f, (seed / 5) % 5);
    w(Object + 0x973, 1);
    w(Other + 0xac, Catalog + 0x600);
    w(Catalog + 0x608, seed & 8 ? 2 : 1);
    w(Other + 8, seed % 5);
    w(Other + 12, (seed / 5) % 5);
    U c = Map + (36 + ((seed / 5) % 5) * 6 + seed % 5) * 12;
    h(c + 2, 0);
    w(Effect + 40, seed & 16 ? 25 : 26);
    w(Object + 0x7cd, seed % 4 == 0 ? 3 : 2);
    w(Object + 0x855, seed & 4);
    w(Object + 0x7d1, seed & 8 ? Effect : 0);
    w(Effect + 40, seed & 16 ? 25 : 52);
    w(Manager + 0x438, Other + 0x1200);
    w(Effect + 68, 1);
    w(Effect + 0x160 + 28, 0x6dbf00);
    // The linked map object remains a distinct effect record.
    h(c + 2, 1);
    w(Effect + 0x22e + 40, seed & 16 ? 25 : 26);

    m.set8(Object + 0x722, seed & 4);
    m.invoke(root, Object, {seed & 1 ? Other : 0u});
    return 0;
  }
  case 0x58c8a0:
    surfaces(Object, Other);
    w(Buffer, 1);
    w(Buffer + 4, 2);
    w(Buffer + 8, 31);
    w(Buffer + 12, 47);
    m.invoke(root, Object, {Other, Buffer, U(-3), 19});
    return 0;
  case 0x4637c0:
    surfaces(Object + 0x21, Object + 0x53);
    w(Object + 0x89, Pool);
    w(Pool + 0xe4, seed % 3 ? 40 : 0);
    w(Pool + 0xac, Catalog);
    w(Catalog + 24, 100);
    w(Object + 0x8e, seed & 4 ? 100 : 0);
    m.invoke(root, Object, {});
    return 0;
  case 0x5055a0:
    w(0x6dbf00, 0);
    w(0x6dbf04, 0);
    w(0x6dbf08, 0);
    w(0x6e9808, seed & 2);
    w(Object + 0x74, ~0u);
    return m.invoke(root, Object, {0, Buffer});
  default:
    throw std::runtime_error("Unknown frontier case");
  }
}
