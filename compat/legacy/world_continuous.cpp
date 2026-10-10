#include "world_continuous.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::legacy {
namespace {
unsigned word(const std::uint8_t *p, unsigned n) {
  p += n * 4;
  return p[0] | unsigned(p[1]) << 8 | unsigned(p[2]) << 16 |
         unsigned(p[3]) << 24;
}
void put(std::uint8_t *p, unsigned n, unsigned v) {
  p += n * 4;
  for (unsigned i = 0; i < 4; ++i)
    p[i] = std::uint8_t(v >> (i * 8));
}
std::uint64_t wide(const std::uint8_t *p, unsigned n) {
  return word(p, n) | (std::uint64_t(word(p, n + 1)) << 32);
}
unsigned load(const std::uint8_t *p, unsigned n) {
  return __atomic_load_n(reinterpret_cast<const unsigned *>(p + n * 4),
                         __ATOMIC_ACQUIRE);
}
void store(std::uint8_t *p, unsigned n, unsigned v) {
  __atomic_store_n(reinterpret_cast<unsigned *>(p + n * 4), v,
                   __ATOMIC_RELEASE);
}
unsigned fnv(const std::uint8_t *p, std::size_t n) {
  unsigned h = 2166136261u;
  for (std::size_t i = 0; i < n; ++i)
    h = (h ^ p[i]) * 16777619u;
  return h;
}
void require(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
constexpr unsigned requestAt = MNM_CONT_SLOT_HEADER + MNM_CONT_SOURCE_BYTES;
constexpr unsigned replyAt = requestAt + MNM_CONT_REQUEST_BYTES;
} // namespace
void WorldContinuousChannel::create(const QString &path, std::uint64_t nonce,
                                    std::uint64_t target) {
  require(nonce != 0, "Zero continuous session nonce");
  QFile f(path);
  require(f.open(QIODevice::ReadWrite | QIODevice::NewOnly),
          "Existing continuous session");
  require(f.resize(MNM_CONT_BYTES), "Cannot bound continuous channel");
  auto *p = f.map(0, MNM_CONT_BYTES);
  require(p, "Cannot map continuous channel");
  std::memset(p, 0, MNM_CONT_HEADER);
  std::memcpy(p, MNM_CONT_MAGIC, 8);
  put(p, 2, 1);
  put(p, 3, MNM_CONT_HEADER);
  put(p, 4, MNM_CONT_BYTES);
  put(p, 6, unsigned(nonce));
  put(p, 7, unsigned(nonce >> 32));
  put(p, 12, unsigned(target));
  put(p, 13, unsigned(target >> 32));
  require(f.unmap(p), "Cannot release initialized mapping");
}
WorldContinuousChannel::WorldContinuousChannel(const QString &path)
    : file_(path) {
  require(file_.open(QIODevice::ReadWrite) && file_.size() == MNM_CONT_BYTES,
          "Continuous channel extent");
  map_ = file_.map(0, MNM_CONT_BYTES);
  require(map_, "Continuous channel mapping");
  try {
    nonce_ = nonce();
    target_ = target();
    require(!std::memcmp(map_, MNM_CONT_MAGIC, 8) && word(map_, 2) == 1 &&
                word(map_, 3) == MNM_CONT_HEADER &&
                word(map_, 4) == MNM_CONT_BYTES && word(map_, 5) == 0 &&
                nonce() != 0,
            "Continuous channel header");
    for (unsigned i = 14; i < 32; ++i)
      require(word(map_, i) == 0, "Continuous header reserved");
    require(!cancelled() && !finished(), "Terminal continuous session");
    unsigned expected = 0;
    require(__atomic_compare_exchange_n(reinterpret_cast<unsigned *>(map_ + 32),
                                        &expected, 1, false, __ATOMIC_ACQ_REL,
                                        __ATOMIC_ACQUIRE),
            "Continuous native role already owned");
  } catch (...) {
    file_.unmap(map_);
    map_ = nullptr;
    throw;
  }
}
WorldContinuousChannel::~WorldContinuousChannel() {
  if (map_)
    file_.unmap(map_);
}
std::uint8_t *WorldContinuousChannel::slot(std::uint64_t g) const {
  require(g && g == expected_, "Continuous generation identity");
  return map_ + MNM_CONT_HEADER + ((g - 1) & 1) * MNM_CONT_SLOT_BYTES;
}
bool WorldContinuousChannel::cancelled() const { return load(map_, 10) != 0; }
bool WorldContinuousChannel::finished() const { return load(map_, 11) != 0; }
std::uint64_t WorldContinuousChannel::target() const { return wide(map_, 12); }
std::uint64_t WorldContinuousChannel::nonce() const { return wide(map_, 6); }
void WorldContinuousChannel::cancel() { store(map_, 10, 1); }
bool WorldContinuousChannel::poll(Block &b) {
  validate();
  require(!cancelled(), "Continuous session cancelled");
  auto *s = slot(expected_);
  const auto state = load(s, 0);
  require(state <= MNM_CONT_RETURNED, "Unknown continuous state");
  if (state != MNM_CONT_REQUEST)
    return false;
  require(load(map_, 9) == 1, "Missing original producer role");
  require(wide(s, 2) == expected_, "Stale continuous request");
  for (unsigned i : {1u, 9u})
    require(word(s, i) == 0, "Continuous slot reserved");
  for (unsigned i = 16; i < 32; ++i)
    require(word(s, i) == 0, "Continuous slot reserved");
  const unsigned n = word(s, 4), r = word(s, 5);
  require(n >= 64 && n <= MNM_CONT_SOURCE_BYTES && r >= 64 &&
              r <= MNM_CONT_REQUEST_BYTES && !word(s, 8) && !word(s, 10) &&
              !word(s, 11) && !word(s, 14) && !word(s, 15),
          "Continuous request extent or stale completion");
  unsigned expected = MNM_CONT_REQUEST;
  require(__atomic_compare_exchange_n(reinterpret_cast<unsigned *>(s),
                                      &expected, MNM_CONT_PROCESSING, false,
                                      __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE),
          "Continuous request lease changed");
  b = {expected_,
       {s + MNM_CONT_SLOT_HEADER, s + MNM_CONT_SLOT_HEADER + n},
       {s + requestAt, s + requestAt + r}};
  require(fnv(b.source.data(), b.source.size()) == word(s, 6) &&
              fnv(b.request.data(), b.request.size()) == word(s, 7),
          "Continuous owned source checksum");
  return true;
}
void WorldContinuousChannel::reply(std::uint64_t g,
                                   const std::vector<std::uint8_t> &bytes) {
  validate();
  require(!cancelled(), "Continuous session cancelled");
  auto *s = slot(g);
  require(load(s, 0) == MNM_CONT_PROCESSING && wide(s, 2) == g &&
              bytes.size() == MNM_CONT_REPLY_BYTES,
          "Continuous reply lease/extent");
  std::memcpy(s + replyAt, bytes.data(), bytes.size());
  put(s, 10, unsigned(bytes.size()));
  put(s, 11, fnv(bytes.data(), bytes.size()));
  store(s, 0, MNM_CONT_REPLY);
}
bool WorldContinuousChannel::returned(std::uint64_t g,
                                      std::vector<std::uint8_t> &bytes) {
  validate();
  require(!cancelled(), "Continuous session cancelled");
  auto *s = slot(g);
  if (load(s, 0) != MNM_CONT_RETURNED)
    return false;
  require(wide(s, 2) == g && word(s, 8) == word(s, 4) + 192 &&
              word(s, 8) <= MNM_CONT_SOURCE_BYTES,
          "Continuous actual return extent");
  bytes.assign(s + MNM_CONT_SLOT_HEADER, s + MNM_CONT_SLOT_HEADER + word(s, 8));
  return true;
}
void WorldContinuousChannel::release(std::uint64_t g) {
  validate();
  auto *s = slot(g);
  require(load(s, 0) == MNM_CONT_RETURNED && wide(s, 2) == g,
          "Continuous release lease");
  require(expected_ != std::numeric_limits<std::uint64_t>::max(),
          "Continuous generation exhausted");
  store(s, 0, MNM_CONT_FREE);
  ++expected_;
}
} // namespace mnm::legacy

void mnm::legacy::WorldContinuousChannel::validate() const {
  require(!std::memcmp(map_, MNM_CONT_MAGIC, 8) && word(map_, 2) == 1 &&
              word(map_, 3) == MNM_CONT_HEADER &&
              word(map_, 4) == MNM_CONT_BYTES && word(map_, 5) == 0 &&
              nonce() == nonce_ && target() == target_ && load(map_, 8) == 1 &&
              load(map_, 9) <= 1 && load(map_, 10) <= 1 && load(map_, 11) <= 1,
          "Continuous immutable envelope changed");
  for (unsigned i = 14; i < 32; ++i)
    require(word(map_, i) == 0, "Continuous header reserved changed");
}

std::array<std::uint32_t, 4>
mnm::legacy::WorldContinuousChannel::ticks(std::uint64_t g) const {
  validate();
  auto *s = slot(g);
  require(load(s, 0) == MNM_CONT_RETURNED && wide(s, 2) == g,
          "Continuous timing lease");
  return {word(s, 12), word(s, 13), word(s, 14), word(s, 15)};
}
