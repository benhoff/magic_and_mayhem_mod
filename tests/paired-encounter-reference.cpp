#include "paired-encounter-original.hpp"
#include <filesystem>
namespace {
void writeBytes(const std::filesystem::path &p, const Bytes &b) {
  if (std::filesystem::exists(p))
    throw std::runtime_error("existing artifact");
  std::ofstream f(p, std::ios::binary);
  f.write(reinterpret_cast<const char *>(b.data()), b.size());
  if (!f)
    throw std::runtime_error("artifact write");
}
void checkpoint(const std::filesystem::path &out, const std::string &name,
                const game::State &s, const encounter::paired::Projection &p) {
  game::writeSnapshot(out / (name + ".snapshot"), s);
  writeBytes(out / (name + ".oracle"), encounter::paired::values(p));
}
unsigned number(const char *s) {
  std::string text(s);
  std::size_t end = 0;
  auto n = std::stoul(text, &end);
  if (end != text.size() || n > 0xffffffffu)
    throw std::runtime_error("invalid number");
  return unsigned(n);
}
} // namespace
int main(int argc, char **argv) try {
  if (argc != 8)
    throw std::runtime_error(
        "PE run INSTALL SEED TICKS NEW_OUTPUT FAULT_TICK | PE resume SNAPSHOT "
        "ORACLE TICKS NEW_OUTPUT FAULT_TICK");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  sealText(pe);
  initializeSearch();
  const bool resume = std::string(argv[2]) == "resume";
  if (!resume && std::string(argv[2]) != "run")
    throw std::runtime_error("unknown mode");
  const auto ticks = number(argv[5]), fault = number(argv[7]);
  if (ticks > 8192)
    throw std::runtime_error("tick limit");
  auto profile = encounter::decodeInputs(
      resume ? game::readSnapshot(argv[3]).campaign
             : encounter::loadInputs(argv[3], {0, 0, 12, 12}));
  auto saved =
      resume ? game::readSnapshot(argv[3])
             : encounter::create(profile, number(argv[4])).world().state();
  auto continuation = cs::decode(saved.systems);
  const auto seed = continuation.seed, elapsed = continuation.native.tick;
  if (elapsed > 8192 || ticks > 8192 - elapsed)
    throw std::runtime_error("total tick limit");
  game::World world(0);
  world.restore(saved);
  game::CreatureRuleSession native(std::move(world),
                                   encounter::runtime(profile));
  Original original(profile);
  setup(original,
        cs::decode(encounter::create(profile, seed).world().state().systems),
        seed);
  // Oracle checkpoints rebuild the original prefix from immutable inputs and
  // RNG. Native continuation is restored directly; no native outcome feeds the
  // oracle.
  for (unsigned i = 0; i < elapsed; ++i)
    step(original);
  auto oracle = projectOriginal(original);
  if (resume && std::filesystem::file_size(argv[4]) > 1024 * 1024)
    throw std::runtime_error("oracle checkpoint extent");
  if (resume && encounter::paired::values(oracle) != read(argv[4]))
    throw std::runtime_error("oracle replay checkpoint differs");
  compare(original, cs::decode(native.world().state().systems));
  std::filesystem::path out(argv[6]);
  if (!std::filesystem::create_directory(out))
    throw std::runtime_error("existing output directory");
  checkpoint(out, "initial", native.world().state(), oracle);
  unsigned routeCompletions = 0, stunRecoveries = 0, removals = 0;
  std::ofstream trace(out / "trace.txt");
  std::uint64_t digest = 1469598103934665603ull;
  for (unsigned i = 0; i < ticks; ++i) {
    auto before = native.world().state();
    auto prior = oracle;
    step(original);
    native.step();
    if (fault && original.tick == fault) {
      auto s = native.world().state();
      auto c = cs::decode(s.systems);
      c.world.creatures[0].ai.health ^= 1;
      s.systems = cs::encode(c);
      game::World faulty(0);
      faulty.restore(std::move(s));
      native = game::CreatureRuleSession(std::move(faulty),
                                         encounter::runtime(profile));
    }
    auto now = cs::decode(native.world().state().systems);
    auto prev = cs::decode(before.systems);
    for (unsigned actor = 0; actor < 2; ++actor) {
      if (prev.world.creatures[actor].ai.motion &&
          !now.world.creatures[actor].ai.motion)
        ++routeCompletions;
      if (prev.world.creatures[actor].ai.action == 14 &&
          now.world.creatures[actor].ai.action != 14)
        ++stunRecoveries;
      if (prev.world.creatures[actor].ai.active &&
          !now.world.creatures[actor].ai.active)
        ++removals;
    }
    auto actual = encounter::paired::project(now, profile);
    oracle = projectOriginal(original);
    auto why = encounter::paired::difference(actual, oracle);
    if (!why.empty()) {
      checkpoint(out, "before", before, prior);
      checkpoint(out, "after", native.world().state(), oracle);
      std::ofstream report(out / "divergence.txt");
      report << "tick=" << original.tick << " " << why << '\n';
      checkText();
      destroySearch();
      std::cerr << "tick=" << original.tick << " " << why << '\n';
      return 3;
    }
    digest = encounter::paired::digest(digest, actual);
    trace << original.tick << ' ' << digest << '\n';
  }
  if (!trace)
    throw std::runtime_error("trace write");
  checkpoint(out, "final", native.world().state(), oracle);
  checkText();
  destroySearch();
  auto stats =
      encounter::statistics(cs::decode(native.world().state().systems));
  std::cout << "{\"success\":true,\"ticks\":" << original.tick
            << ",\"digest\":" << digest << ",\"hits\":" << stats.hits
            << ",\"stuns\":" << stats.stuns
            << ",\"knockbacks\":" << stats.knockbacks
            << ",\"route_completions\":" << routeCompletions
            << ",\"stun_recoveries\":" << stunRecoveries
            << ",\"removals\":" << removals << ",\"deaths\":" << stats.deaths
            << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
