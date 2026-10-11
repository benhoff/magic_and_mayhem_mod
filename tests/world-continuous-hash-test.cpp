#include "../compat/legacy/world_raster_batch.hpp"
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include "../protocols/include/mnm/world_raster_batch_v3.h"
#include <cassert>
#include <cstring>
using namespace mnm::legacy;
void put(std::vector<std::uint8_t> &b, unsigned at, unsigned v) {
  for (unsigned i = 0; i < 4; ++i)
    b.at(at + i) = std::uint8_t(v >> (i * 8));
}
unsigned reference(const CanvasProducer &c) {
  std::vector<std::uint8_t> bytes;
  for (auto v : c.fields)
    for (unsigned i = 0; i < 4; ++i)
      bytes.push_back(std::uint8_t(v >> (i * 8)));
  auto hash =
      mnm_cont_crc32c_portable(c.bytes().data(), unsigned(c.bytes().size()));
  for (unsigned i = 0; i < 4; ++i)
    bytes.push_back(std::uint8_t(hash >> (i * 8)));
  return worldBatchHash(bytes);
}
int main() {
  CanvasProducer first{};
  auto &r = first.fields;
  r[0] = 140;
  r[1] = 2;
  r[2] = 9;
  r[3] = 7;
  r[5] = r[7] = 800;
  r[6] = 600;
  r[8] = 799;
  r[12] = 800;
  r[13] = 600;
  r[18] = r[19] = 44;
  r[21] = 1;
  auto source = std::make_shared<std::vector<std::uint8_t>>(44);
  put(*source, 0, 44);
  put(*source, 4, 1);
  put(*source, 8, 1);
  first.ownedPayload = source;
  auto second = first;
  second.fields[1] = 3;
  std::vector<const CanvasProducer *> rows{&first, &second};
  std::vector<std::uint8_t> request(96);
  std::memcpy(request.data(), MNM_WORLD_BATCH_REQUEST, 8);
  const unsigned h[] = {5, 64, 1, 1, 3, 7, 800, 600, 800, 32, 0, 1, 2, 2};
  for (unsigned i = 0; i < 14; ++i)
    put(request, 8 + i * 4, h[i]);
  for (unsigned i = 0; i < 2; ++i) {
    put(request, 64 + i * 16, i + 2);
    put(request, 68 + i * 16, 0x595677);
    put(request, 72 + i * 16, 1);
    put(request, 76 + i * 16, reference(*rows[i]));
  }
  auto checksum = [&](auto &b) {
    put(b, 48, worldBatchHash({b.begin() + 64, b.end()}));
  };
  checksum(request);
  auto parse = [&](const auto &b) {
    return validateWorldRasterBatch(b, 1, 1, 3, 7, 800, 600, 0, rows,
                                    WorldBatchSourcePolicy::ownedV5);
  };
  auto batch = parse(request);
  assert(batch.descriptors.size() == 2 && batch.header[2] == 5);
  unsigned count = 0;
  auto reject = [&](auto body) {
    bool refused = false;
    try {
      parse(body);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    assert(refused);
    ++count;
  };
  for (unsigned at :
       {8u, 16u, 20u, 24u, 28u, 32u, 36u, 40u, 44u, 52u, 56u, 60u}) {
    auto bad = request;
    bad[at] ^= 1;
    reject(bad);
  }
  for (unsigned at : {64u, 68u, 72u, 76u, 80u, 84u, 88u, 92u}) {
    auto bad = request;
    bad[at] ^= 1;
    checksum(bad);
    reject(bad);
  }
  (*source)[40] ^= 1;
  reject(request);
  (*source)[40] ^= 1;
  second.fields[16] ^= 1;
  reject(request);
  second.fields[16] ^= 1;
  bool oldRefused = false;
  try {
    validateWorldRasterBatch(request, 1, 1, 3, 7, 800, 600, 0, rows);
  } catch (const std::invalid_argument &) {
    oldRefused = true;
  }
  assert(oldRefused && count == 22);
  auto reply =
      worldRasterBatchReply(batch, std::vector<std::uint16_t>(480000, 0x1234));
  assert(reply[8] == 5 && reply.size() == 960064);
}
