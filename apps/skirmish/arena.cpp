#include "arena.hpp"
#include "health_rules.hpp"
#include "cure_runtime.hpp"
#include "creature_runtime.hpp"
#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <array>
#include <cstring>

namespace mnm::skirmish {
namespace {
using Buffer = std::vector<std::byte>;
void put(Buffer &b, std::size_t at, std::uint32_t n, unsigned size = 4) {
    for (unsigned i = 0; i < size; ++i)
        b.at(at + i) = std::byte(n >> (8 * i));
}
void append(Buffer &b, std::uint32_t n) {
    for (unsigned i = 0; i < 4; ++i)
        b.push_back(std::byte(n >> (8 * i)));
}
Buffer fixture() {
    std::array<Buffer, 7> blocks{{Buffer(8 * 4), Buffer(3 * 4), Buffer(8 * 8 * 3 * 12),
                                  Buffer(2 * 0x164), Buffer(0xd07), Buffer(0x198), Buffer(0x5c9)}};
    for (unsigned y = 0; y < 8; ++y)
        put(blocks[0], y * 4, y * 8);
    for (unsigned z = 0; z < 3; ++z)
        put(blocks[1], z * 4, z * 64);
    for (unsigned z = 0; z < 3; ++z)
        for (unsigned y = 0; y < 8; ++y)
            for (unsigned x = 0; x < 8; ++x) {
                const auto at = (x + 8 * (y + 8 * z)) * 12;
                put(blocks[2], at + 4, 0xffff, 2);
                if (!z)
                    put(blocks[2], at, 1, 2);
                if (!x || x == 7 || !y || y == 7)
                    put(blocks[2], at + 10, 0x4000, 2);
            }
    put(blocks[3], 0x164 + 0x94, 16);
    blocks[3][0x164 + 0xb0] = std::byte{8};
    for (unsigned at : {8U, 12U, 16U})
        put(blocks[4], at, 1);
    put(blocks[5], 8, 1);
    put(blocks[5], 12, 1);
    put(blocks[5], 16, 500);
    for (unsigned i = 0; i < 48; ++i)
        put(blocks[5], 0xd8 + i * 4, 40U);
    put(blocks[6], 0x589, 720);
    float coefficient = 0.969F;
    std::uint32_t bits = 0;
    std::memcpy(&bits, &coefficient, 4);
    Buffer result;
    for (char c : std::string("MNMWLD01"))
        result.push_back(std::byte(c));
    for (unsigned n :
         {0x1200000U, 8U, 8U, 3U, 64U, 1U, 300U, 0U, 5U, 1U, 1U, 720U, bits, 0x1000000U})
        append(result, n);
    for (const auto &b : blocks)
        append(result, static_cast<unsigned>(b.size()));
    for (const auto &b : blocks)
        result.insert(result.end(), b.begin(), b.end());
    return result;
}
} // namespace
std::shared_ptr<const game::Navigation> arenaNavigation() {
    return sandbox::loadFrozenNavigationBytes(fixture(), {}, true, true);
}
game::SkirmishSession newSession(std::shared_ptr<const game::CombatDriver> driver) { return game::SkirmishSession(arenaNavigation(), ordinaryHealthRules(), battleRuntime(), std::move(driver)); }
game::SkirmishSession load(const std::filesystem::path &path) {
    auto state = game::readSnapshot(path);
    const auto rules = resolveHealthRules(game::decodeCombat(state.systems).healthRulesIdentity);
    auto effects = resolveCureRuntime(state);
    auto driver = resolveCreatureRuntime(state);
    game::World w(0);
    w.restore(std::move(state));
    return {std::move(w), arenaNavigation(), rules, std::move(effects), std::move(driver)};
}
} // namespace mnm::skirmish
