static constexpr U FontRoots[] = {
    0x4a58c0, 0x4a58e0, 0x4a59e0, 0x4a5ae0, 0x4a5ce0, 0x4a6190,
    0x4a6630, 0x4a6970, 0x581ec0, 0x4be870, 0x4be8c0, 0x4be910,
    0x4be960, 0x4be9b0, 0x4bea00, 0x4bea50, 0x4beaa0, 0x4d9f30};
static U fontGet(const std::vector<unsigned char> &b, U p) {
  U n;
  std::memcpy(&n, b.data() + p, 4);
  return n;
}
static void fontPut(std::vector<unsigned char> &b, U p, U n) {
  std::memcpy(b.data() + p, &n, 4);
}
static void initializeFontCase(FontMemory &m, U seed) {
  for (auto range : FontGlobals)
    std::memset(m.pointer(range[0], range[1]), 0, range[1]);
  for (U k = 0; k < 0x8000; ++k)
    m.set16(FontCanvas + k * 2, (k * 1273 + seed * 113) & 65535);
  m.set32(0x658174, FontCanvas);
  m.set32(0x6a2dc8, 128);
  m.set32(0x6def50, 64);
  m.set32(0x6e0008, 0);
  m.set32(0x6cbb6c, 0);
  m.set32(0x6a49b8, 128);
  m.set32(0x656618, 64);
  m.set32(0x6e1f88, (seed & 16) != 0);
  m.set16(0x6e1f7c, seed & 16 ? 0x7c00 : 0xf800);
  m.set16(0x6e1f7e, seed & 16 ? 0x3e0 : 0x7e0);
  m.set16(0x6e1f80, 31);
  m.set32(0x6e1ed4, seed % 4 == 0 ? U(-1) : seed % 4);
  m.set32(0x6f5eac, seed & 8 ? 0x3f800000 : 0);
  for (U k = 0; k < 64; ++k) {
    float f = k / 64.f;
    U bits;
    std::memcpy(&bits, &f, 4);
    m.set32(0x6f5dac + k * 4, bits);
  }
  auto source = FontCorpus[seed % FontCorpusCount];
  m.fontData.assign(source.bytes, source.bytes + source.size);
  const char *messages[] = {"A! b_?", "A\tB\nC", "",
                            "hello",  "a a a a", "!?:;"};
  auto message = messages[(seed / 6) % 6];
  std::memcpy(m.pointer(Buffer, 256), message, std::strlen(message) + 1);
  std::memcpy(m.pointer(Buffer + 256, 9), "font.sft", 9);
  m.set32(FontObject + 0x224, 9 + seed % 4);
  m.set32(FontObject + 0x230, seed % 3);
  m.set32(FontObject + 0x218, 7);
  m.set32(FontObject + 0x21c, 29);
  for (U k = 0; k < 128; ++k)
    m.set32(FontObject + 24 + k * 4, 7 + k % 5);
  for (U k = 0; k < 4; ++k)
    m.set32(Output + k * 4, k == 2 ? 120 : k == 3 ? 64 : 0);
  m.set32(Output + 20, 0xfeedbeef);
}
static U installOwnedFont(FontMemory &m, U p) {
  U raw = m.allocate(U(m.fontData.size()));
  std::memcpy(m.pointer(raw, U(m.fontData.size())), m.fontData.data(),
              m.fontData.size());
  U rows = fontGet(m.fontData, 16), count = fontGet(m.fontData, 12),
    metrics = 40 + (fontGet(m.fontData, 32) ? 768 : 0),
    table = raw + metrics + rows * fontGet(m.fontData, 36) * 8;
  m.set32(p, raw);
  m.set32(p + 4, raw + 40);
  m.set32(p + 8, raw + metrics);
  m.set32(p + 12, table);
  m.set32(p + 16, table + count * 4);
  m.set32(p + 20, 0);
  m.set32(p + 0x220, fontGet(m.fontData, 20) + fontGet(m.fontData, 24));
  return raw;
}
static U executeFonts(FontMemory &m, U root, U seed) {
  U p = FontObject;
  if (root == 0x4a5ae0) {
    if (seed & 32)
      installOwnedFont(m, p); // Reload preserves the earlier raw allocation.
    U choice = seed % 16;
    m.fontFailure = choice < 5 ? choice : 0;
    if (choice == 5)
      fontPut(m.fontData, 4, U(m.fontData.size() + 4));
    if (choice == 6)
      fontPut(m.fontData, 0, 0x12345678); // Original does not check magic.
    if (choice == 7)
      fontPut(m.fontData, 8, 7); // Unknown version retains earlier pointers.
    if (choice == 8)
      fontPut(m.fontData, 32, 0); // Observe no-palette pointer arithmetic only.
    if (choice == 10 || choice == 11) {
      m.fontData.erase(m.fontData.begin() + 36, m.fontData.begin() + 40);
      fontPut(m.fontData, 8, 2);
      fontPut(m.fontData, 4, U(m.fontData.size()));
      if (choice == 11) {
        m.fontData.erase(m.fontData.begin() + 36, m.fontData.begin() + 804);
        fontPut(m.fontData, 32, 0);
        fontPut(m.fontData, 4, U(m.fontData.size()));
      }
    }
    U old = 0;
    if (seed & 32) {
      old = m.allocate(20);
      m.set32(old, 0);
      m.set32(p + 20, old);
    }
    U conversion = (choice == 7 || choice == 8 || choice == 11)
                       ? 0
                       : (seed & 4   ? 4
                          : seed & 8 ? 16
                          : seed & 2 ? 2
                                     : 0);
    if (choice == 3)
      m.failAllocation = m.allocations + 1;
    if (choice == 9)
      m.failAllocation = m.allocations + 2;
    return m.invoke(root, p, {Buffer + 256, conversion});
  }
  if (root == 0x4a58c0) {
    U head = 0;
    for (U k = 0; k < seed % 4; ++k) {
      U node = m.allocate(20);
      m.set32(node, head);
      head = node;
    }
    m.invoke(root, p, {head});
    return 0;
  }
  if (root >= 0x4be870 && root <= 0x4beaa0) {
    bool destroy = root == 0x4be8c0 || root == 0x4be960 || root == 0x4bea00 ||
                   root == 0x4beaa0;
    U global = root <= 0x4be8c0   ? 0x6b0400
               : root <= 0x4be960 ? 0x656680
               : root <= 0x4bea00 ? 0x6df1c0
                                  : 0x6a5138;
    if (destroy && seed % 4) {
      installOwnedFont(m, global);
      U head = 0;
      for (U k = 0; k < seed % 3; ++k) {
        U node = m.allocate(20);
        m.set32(node, head);
        head = node;
      }
      m.set32(global + 20, head);
    }
    m.invoke(root, 0, {});
    return 0;
  }
  if (root == 0x4d9f30) {
    if (!m.invoke(0x4a5ae0, p, {Buffer + 256, 2}))
      throw std::runtime_error("Font actor prerequisite load");
    // Real actor popup surface control and font body are composed; COM stays
    // owned.
    U actor = Object;
    m.set32(actor + 0x16d, Other + 0x1200);
    m.set32(actor + 0x128, Other + 0x1200);
    m.set32(actor + 0x12c, Other + 0x1200);
    m.set32(actor + 0x179, 128);
    m.set32(actor + 0x17d, 64);
    m.set32(actor + 0x181, 256);
    m.set32(actor + 0x169 + 4, Other + 0x1200);
    m.set32(actor + 0x169 + 8, Other + 0x1200);
    m.set32(actor + 0x169 + 16, 256);
    m.set32(actor + 0x169 + 20, 64);
    m.set32(actor + 0x169 + 24, 256);
    m.set32(Other + 0x1200, Base + 0x25000);
    m.set32(Base + 0x25000 + 0x80, 0x1001f680);
    m.invoke(root, actor, {Buffer});
    return 0;
  }
  installOwnedFont(m, p);
  if (root == 0x4a6970)
    return m.invoke(root, p, {});
  if (root == 0x4a58e0) {
    U bytes[] = {0, 9, 10, 31, 32, 33, 39, 63, 95, 97, 0x91, 0xb4, 255};
    return m.invoke(root, p, {bytes[seed % 13], seed & 1});
  }
  if (root == 0x4a59e0) {
    U bytes[] = {33, 39, 63, 95, 97, 9, 10, 32};
    m.invoke(root, p, {bytes[seed % 8], 173, 91, 237});
    return 0;
  }
  if (root == 0x581ec0) {
    U frame = m.get32(p + 16) +
              m.get32(m.get32(p + 12) + (seed % fontGet(m.fontData, 12)) * 4);
    m.fastcall(root, frame, U(int(seed % 5) - 2),
               {U(int(seed % 7) - 2), 173, 91, 237});
    return 0;
  }
  U flags[] = {0, 2, 4, 8, 16, 20, 24, 30};
  U flag = flags[seed % 8];
  // Bounded valid words with a separator avoid the original unterminated
  // long-word branch.
  if ((flag & 16) && !m.get8(Buffer))
    std::memcpy(m.pointer(Buffer, 4), "a a", 4);
  if (root == 0x4a6190) {
    if (!m.get8(Buffer))
      std::memcpy(m.pointer(Buffer, 2), "a", 2);
    return m.invoke(
        root, p, {Buffer, Output, seed % 3, 173, 91, 237, flag, Output + 20});
  }
  if (root == 0x4a6630) {
    if (!m.get8(Buffer))
      std::memcpy(m.pointer(Buffer, 2), "a", 2);
    return m.invoke(root, p, {seed & 32 ? 0 : Buffer, Output, 0, flag});
  }
  return m.invoke(root, p, {Buffer, Output, 173, 91, 237, flag, Output + 20});
}
static void releaseFontCase(FontMemory &m, U) {
  for (U d : {0x4be8c0u, 0x4be960u, 0x4bea00u, 0x4beaa0u})
    m.invoke(d, 0, {});
}
