#include "../compat/legacy/world_continuous.hpp"
#include <QTemporaryDir>
#include <cassert>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
using namespace mnm::legacy;
namespace {
void put(unsigned char *p, unsigned n, unsigned v) {
  std::memcpy(p + n * 4, &v, 4);
}
unsigned fnv(const unsigned char *p, unsigned n) {
  unsigned h = 2166136261u;
  while (n--)
    h = (h ^ *p++) * 16777619u;
  return h;
}
void state(unsigned char *p, unsigned v) {
  __atomic_store_n(reinterpret_cast<unsigned *>(p), v, __ATOMIC_RELEASE);
}
unsigned load(unsigned char *p) {
  return __atomic_load_n(reinterpret_cast<unsigned *>(p), __ATOMIC_ACQUIRE);
}
template <class F> void refuses(F f) {
  bool rejected = false;
  try {
    f();
  } catch (const std::exception &) {
    rejected = true;
  }
  assert(rejected);
}
} // namespace
int main() {
  QTemporaryDir dir;
  assert(dir.isValid());
  const auto path = dir.filePath("channel");
  refuses([&] { WorldContinuousChannel::create(path, 0, 0); });
  WorldContinuousChannel::create(path, 101, 64);
  refuses([&] { WorldContinuousChannel::create(path, 102, 64); });
  WorldContinuousChannel native(path);
  assert(native.nonce() == 101 && native.target() == 64);
  refuses([&] { WorldContinuousChannel duplicate(path); });
  QFile file(path);
  assert(file.open(QIODevice::ReadWrite));
  auto *m = file.map(0, MNM_CONT_BYTES);
  assert(m);
  WorldContinuousChannel::Block b;
  assert(!native.poll(b));
  auto *s = m + MNM_CONT_HEADER;
  put(m, 9, 1);
  put(s, 2, 1);
  put(s, 4, 64);
  put(s, 5, 64);
  auto *src = s + MNM_CONT_SLOT_HEADER;
  auto *req = src + MNM_CONT_SOURCE_BYTES;
  std::memset(src, 3, 64);
  std::memset(req, 4, 64);
  put(s, 6, fnv(src, 64));
  put(s, 7, fnv(req, 64));
  state(s, MNM_CONT_REQUEST);
  assert(native.poll(b) && b.generation == 1 && b.source.size() == 64 &&
         b.source[0] == 3);
  // Owned source bytes survive original storage mutation; returned prefix
  // validation belongs to the semantic adapter and is exercised live.
  src[0] = 8;
  assert(b.source[0] == 3);
  src[0] = 3;
  refuses([&] { native.release(1); });
  refuses([&] {
    native.reply(2, std::vector<unsigned char>(MNM_CONT_REPLY_BYTES));
  });
  refuses([&] { native.reply(1, std::vector<unsigned char>(1)); });
  std::vector<unsigned char> reply(MNM_CONT_REPLY_BYTES, 7);
  native.reply(1, reply);
  assert(load(s) == MNM_CONT_REPLY);
  const auto child = fork();
  assert(child >= 0);
  if (!child) {
    assert(load(s) == MNM_CONT_REPLY);
    std::memset(src + 64, 5, 192);
    put(s, 8, 256);
    state(s, MNM_CONT_RETURNED);
    _exit(0);
  }
  int status = 0;
  assert(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
         WEXITSTATUS(status) == 0);
  std::vector<unsigned char> closed;
  assert(native.returned(1, closed) && closed.size() == 256 && closed[64] == 5);
  native.release(1);
  assert(load(s) == MNM_CONT_FREE);
  // The next block uses the second slot; a published stale first-slot request
  // cannot supersede it or be mistaken for the next generation.
  state(s, MNM_CONT_REQUEST);
  assert(!native.poll(b));
  state(s, MNM_CONT_FREE);
  auto *second = s + MNM_CONT_SLOT_BYTES;
  put(second, 2, 1);
  state(second, MNM_CONT_REQUEST);
  refuses([&] { native.poll(b); });
  state(second, MNM_CONT_FREE);
  native.cancel();
  assert(native.cancelled());
  refuses([&] { native.poll(b); });
  assert(file.unmap(m));
  // A fresh session after terminal cancellation requires new storage/nonce.
  const auto fresh = dir.filePath("fresh");
  WorldContinuousChannel::create(fresh, 103, 0);
  WorldContinuousChannel recovered(fresh);
  assert(!recovered.cancelled() && recovered.target() == 0);
  return 0;
}
