#include "../assets/dat.hpp"
#include "../reconstruction/creature-ai/brain_inference.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#ifdef MNM_BRAIN_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_brain_mapper_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
namespace m = mnm::reconstruction::combat;
namespace {
using Bytes = std::vector<unsigned char>;
void need(bool v, const char *why) {
  if (!v)
    throw std::runtime_error(why);
}
unsigned bits(float f) {
  unsigned v;
  std::memcpy(&v, &f, 4);
  return v;
}
float number(unsigned v) {
  float f;
  std::memcpy(&f, &v, 4);
  return f;
}
Bytes load(const char *path) {
  std::ifstream f(path, std::ios::binary);
  need(bool(f), "brain input");
  return Bytes(std::istreambuf_iterator<char>(f), {});
}
m::BrainInferenceModel decoded(const mnm::assets::BrainModel &a) {
  m::BrainInferenceModel m;
  m.key = a.key;
  m.dimension = a.dimension;
  for (const auto &l : a.layers) {
    m::BrainInferenceLayer out;
    out.observations = number(l.scalarBits);
    out.outside = number(l.tailBits);
    for (const auto &n : l.nodes) {
      std::array<float, 12> v;
      for (unsigned i = 0; i < 12; ++i)
        v[i] = number(n[i]);
      out.nodes.push_back(v);
    }
    for (unsigned v : l.matrix)
      out.matrix.push_back(number(v));
    m.layers.push_back(std::move(out));
  }
  return m;
}
unsigned checks = 0;
std::uint64_t digest = 1469598103934665603ull;
void record(unsigned a, unsigned b, const char *why) {
  if (a != b)
    throw std::runtime_error(std::string(why) + " native=" + std::to_string(a) +
                             " original=" + std::to_string(b));
  ++checks;
  digest = (digest ^ a) * 1099511628211ull;
}
#ifdef MNM_BRAIN_REFERENCE
unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<char *>(p) + off, &v, 4);
}
std::map<void *, unsigned> allocated;
void *__attribute__((cdecl)) allocate(unsigned n) {
  need(n < 1024 * 1024, "original allocation bound");
  void *p = std::malloc(n ? n : 1);
  need(p, "original allocation");
  allocated[p] = n;
  return p;
}
void __attribute__((cdecl)) release(void *p) {
  if (!p)
    return;
  need(allocated.erase(p) == 1, "original unknown allocation");
  std::free(p);
}
void redirect(unsigned va, void *p) {
  auto *out = reinterpret_cast<unsigned char *>(va);
  out[0] = 0xe9;
  put(out, 1, address(p) - va - 5);
}
std::vector<float> original(const m::BrainInferenceModel &m,
                            const std::vector<int> &features) {
  Bytes manager(32), head(64), node(64), nil(64), layers(m.layers.size() * 40),
      input(16);
  std::vector<Bytes> rows(m.layers.size());
  std::vector<std::vector<Bytes>> matrices(m.layers.size());
  put(manager.data(), 4, address(head.data()));
  put(manager.data(), 12, 1);
  put(head.data(), 0, address(node.data()));
  put(head.data(), 4, address(node.data()));
  put(head.data(), 8, address(node.data()));
  put(head.data(), 16, 1);
  put(nil.data(), 16, 1);
  put(node.data(), 0, address(nil.data()));
  put(node.data(), 4, address(head.data()));
  put(node.data(), 8, address(nil.data()));
  put(node.data(), 12, m.key);
  put(node.data(), 16, m.dimension);
  put(node.data(), 28, address(layers.data()));
  put(node.data(), 32, address(layers.data() + layers.size()));
  put(node.data(), 36, address(layers.data() + layers.size()));
  // Exact map nil binding recovered in47ef40.
  global(0x5fce98, address(nil.data()));
  for (unsigned i = 0; i < m.layers.size(); ++i) {
    const auto &l = m.layers[i];
    auto *p = layers.data() + 40 * i;
    put(p, 0, bits(l.observations));
    put(p, 20, bits(l.outside));
    put(p, 8, address(l.nodes.data()));
    put(p, 12, address(l.nodes.data() + l.nodes.size()));
    put(p, 16, address(l.nodes.data() + l.nodes.size()));
    rows[i].assign(m.dimension * 16, 0);
    matrices[i].resize(m.dimension);
    for (unsigned j = 0; j < m.dimension; ++j) {
      matrices[i][j].resize(m.dimension * 4);
      std::memcpy(matrices[i][j].data(), l.matrix.data() + j * m.dimension,
                  m.dimension * 4);
      put(rows[i].data(), 16 * j + 4, address(matrices[i][j].data()));
      put(rows[i].data(), 16 * j + 8,
          address(matrices[i][j].data() + m.dimension * 4));
      put(rows[i].data(), 16 * j + 12,
          address(matrices[i][j].data() + m.dimension * 4));
    }
    put(p, 28, address(rows[i].data()));
    put(p, 32, address(rows[i].data() + rows[i].size()));
    put(p, 36, address(rows[i].data() + rows[i].size()));
  }
  put(input.data(), 4, address(features.data()));
  put(input.data(), 8, address(features.data() + features.size()));
  put(input.data(), 12, address(features.data() + features.size()));
  std::vector<float> result(m.layers.size());
  using Infer =
      void(__attribute__((thiscall)) *)(void *, unsigned, void *, int, float *);
  reinterpret_cast<Infer>(0x4766b0)(manager.data(), m.key, input.data(),
                                    int(m.layers.size()), result.data());
  need(allocated.empty(), "original inference leaked temporary storage");
  return result;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 3, "PE BRAIN");
  const auto pe = load(argv[1]);
  map_image(pe);
  const char *path = argv[2];
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = 1;
  segment.limit_in_pages = 1;
  segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "original exception segment");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "original RX text");
#else
  need(argc == 2, "BRAIN");
  const char *path = argv[1];
#endif
  auto parsed = mnm::assets::decodeBrainDat(load(path));
  need(std::holds_alternative<mnm::assets::BrainDat>(parsed), "brain decode");
  unsigned cases = 0, uniform = 0, zero = 0;
  for (const auto &asset : std::get<mnm::assets::BrainDat>(parsed).models) {
    const auto installed = decoded(asset);
    for (unsigned n = 0; n < 128; ++n) {
      auto model = installed;
      if (n % 17 == 0) {
        for (auto &l : model.layers)
          l.observations = 0;
        ++uniform;
      }
      if (n % 19 == 0) {
        for (auto &l : model.layers)
          std::fill(l.matrix.begin(), l.matrix.end(), 0);
      }
      std::vector<int> features(model.dimension);
      for (unsigned i = 0; i < features.size(); ++i)
        features[i] = int((n * 1031 + i * 137) % 20000) - 10000;
      if (n % 5 == 0)
        for (unsigned i = 0; i < features.size(); ++i)
          features[i] = int(model.layers[0].nodes[i][0] +
                            model.layers[0].nodes[i][1] * float(i % 13) / 10.f);
      auto result = m::inferOwnerTask(model, features);
#ifdef MNM_BRAIN_REFERENCE
      const auto expected = original(model, features);
#else
      const auto &expected = result;
#endif
      for (unsigned i = 0; i < result.size(); ++i) {
        record(bits(result[i]), bits(expected[i]), "brain probability");
        zero += result[i] == 0;
      }
      ++cases;
    }
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "original text guard");
#endif
  need(cases && uniform && zero, "brain inference branch witnesses");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":" << cases << ",\"uniform\":" << uniform
            << ",\"zero\":" << zero << ",\"digest\":\"" << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
