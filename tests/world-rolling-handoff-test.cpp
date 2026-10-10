#include "../compat/legacy/world_rolling_handoff.hpp"
#include <QCoreApplication>
#include <QFile>
#include <iostream>
using namespace mnm::legacy;
namespace {
void require(bool okay) {
  if (!okay)
    throw std::runtime_error("Rolling handoff fixture failed");
}
std::vector<std::uint8_t> read(const QString &path) {
  QFile f(path);
  require(f.open(QIODevice::ReadOnly));
  auto b = f.readAll();
  return {b.begin(), b.end()};
}
template <class F> void refuses(F f) {
  bool okay = false;
  try {
    f();
  } catch (const std::invalid_argument &) {
    okay = true;
  }
  require(okay);
}
} // namespace
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  require(argc == 2);
  const QString directory = argv[1];
  auto whole = decodeCanvasProducers(read(directory + "/canvas-producers.bin"));
  CanvasProducerStream prefix{3, {}};
  prefix.queues = 32;
  prefix.version = 3;
  std::size_t cursor = 0;
  unsigned prior = 0, count = 0;
  for (unsigned q = 1; q <= 32; ++q) {
    auto end = cursor;
    while (whole.operations.at(end).fields[2] != 12)
      ++end;
    require(end > cursor && whole.operations[end - 1].fields[2] == 10);
    prefix.operations.assign(whole.operations.begin(),
                             whole.operations.begin() + end - 1);
    auto request =
        read(directory +
             QString("/world-batch-%1.request").arg(q, 4, 10, QChar('0')));
    auto handoff = prepareRollingHandoff(prefix, cursor, request, q, prior);
    auto packet = decodeRollingQueue(handoff.packet);
    require(packet.queues == 1 && packet.operations.back().fields[2] == 12);
    const auto &closed = packet.operations[packet.operations.size() - 2].fields;
    for (unsigned k :
         {4u, 8u, 9u, 10u, 11u, 12u, 13u, 17u, 18u, 19u, 20u, 21u, 22u, 23u})
      require(closed[k] == 0);
    require(packet.operations.back().fields[23] == 0);
    verifyRollingHandoffReturn(handoff, whole.operations[end - 1],
                               whole.operations[end]);
    RollingCompletion reply{
        q, {800, 600, std::vector<std::uint32_t>(480000, 0x1234)}};
    require(rollingHandoffReply(handoff, reply).size() == 960064);
    auto stale = reply;
    stale.sequence++;
    refuses([&] { rollingHandoffReply(handoff, stale); });
    stale = reply;
    stale.image.width = 600;
    stale.image.height = 800;
    refuses([&] { rollingHandoffReply(handoff, stale); });
    stale = reply;
    stale.image.pixels[0] = 65536;
    refuses([&] { rollingHandoffReply(handoff, stale); });
    auto bad = whole.operations[end];
    bad.fields[20] = 0;
    refuses([&] {
      verifyRollingHandoffReturn(handoff, whole.operations[end - 1], bad);
    });
    bad = whole.operations[end - 1];
    bad.fields[14]++;
    refuses([&] {
      verifyRollingHandoffReturn(handoff, bad, whole.operations[end]);
    });
    auto changed = request;
    changed[28] ^= 1;
    refuses([&] { prepareRollingHandoff(prefix, cursor, changed, q, prior); });
    changed = request;
    changed.back() ^= 1;
    refuses([&] { prepareRollingHandoff(prefix, cursor, changed, q, prior); });
    refuses(
        [&] { prepareRollingHandoff(prefix, cursor, request, q, prior + 1); });
    count += handoff.batch.descriptors.size();
    prior = count;
    cursor = end + 1;
  }
  require(count > 0);
  std::cout
      << "32 complete packets,256 stale/extent/guard/return/checksum refusals; "
      << count << " original entry/AX/source descriptors checked\n";
}
