#include "arena.hpp"
#include "trace.hpp"
#include "persistence/snapshot.hpp"
#include <charconv>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
unsigned ticks(const char *text) {
    const std::string_view s(text);
    unsigned n = 0;
    auto r = std::from_chars(s.data(), s.data() + s.size(), n);
    if (r.ec != std::errc{} || r.ptr != s.data() + s.size() || n > 4096)
        throw std::invalid_argument("Ticks must be 0..4096");
    return n;
}
struct Input {
    unsigned tick;
    mnm::game::SkirmishOrder order;
};
std::vector<Input> script(const char *path) {
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open command script");
    std::vector<Input> result;
    std::string line;
    while (std::getline(file, line)) {
        if (line.size() > 256 || result.size() >= 4096)
            throw std::invalid_argument("Script bounds exceeded");
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream row(line);
        Input input;
        std::string kind;
        unsigned subject = 0;
        if (!(row >> input.tick >> kind >> subject) || input.tick >= 4096)
            throw std::invalid_argument("Invalid command script row");
        auto &o = input.order;
        o.subject = {subject, 1};
        if (kind == "summon" || kind == "move") {
            int x, y;
            if (!(row >> x >> y))
                throw std::invalid_argument("Missing destination");
            o.kind = kind == "summon" ? mnm::game::SkirmishOrderKind::summon
                                      : mnm::game::SkirmishOrderKind::move;
            o.destination = mnm::game::Point{x, y, 1};
        } else if (kind == "bolt" || kind == "attack" || kind == "cure" || kind == "sphere") {
            unsigned target;
            if (!(row >> target))
                throw std::invalid_argument("Missing target");
            o.kind = kind == "sphere" ? mnm::game::SkirmishOrderKind::sphere : kind == "cure" ? mnm::game::SkirmishOrderKind::cure : kind == "bolt" ? mnm::game::SkirmishOrderKind::bolt
                                    : mnm::game::SkirmishOrderKind::attack;
            o.target = mnm::game::Handle{target, 1};
        } else if (kind == "stop")
            o.kind = mnm::game::SkirmishOrderKind::stop;
        else
            throw std::invalid_argument("Unknown script order");
        std::string extra;
        if (row >> extra)
            throw std::invalid_argument("Unexpected script fields");
        if (!result.empty() && input.tick < result.back().tick)
            throw std::invalid_argument("Script must be ordered by tick");
        result.push_back(input);
    }
    return result;
}
void save(const char *p, const mnm::game::SkirmishSession &s) {
    auto r = mnm::game::writeSnapshot(p, s.world().state());
    if (!r.durable)
        throw std::runtime_error(r.detail);
}
} // namespace
int main(int argc, char **argv) try {
    using namespace mnm::game;
    if (argc == 3 && std::string_view(argv[1]) == "create") {
        auto s = mnm::skirmish::newSession();
        save(argv[2], s);
        mnm::skirmish::trace(std::cout, s);
        return 0;
    }
    if (argc == 3 && std::string_view(argv[1]) == "inspect") {
        mnm::skirmish::trace(std::cout, mnm::skirmish::load(argv[2]));
        return 0;
    }
    if ((argc == 5 || argc == 6) && std::string_view(argv[1]) == "resume") {
        auto s = mnm::skirmish::load(argv[2]);
        const auto n = ticks(argv[4]);
        const auto inputs = argc == 6 ? script(argv[5]) : std::vector<Input>{};
        std::size_t cursor = 0;
        while (cursor < inputs.size() && inputs[cursor].tick < s.world().state().tick)
            ++cursor;
        for (unsigned i = 0; i < n && s.state().outcome == Outcome::running; ++i) {
            while (cursor < inputs.size() && inputs[cursor].tick == s.world().state().tick)
                s.enqueue(inputs[cursor++].order);
            const auto r = s.step();
            mnm::skirmish::trace(std::cout, s, r);
        }
        save(argv[3], s);
        return 0;
    }
    throw std::invalid_argument("Usage: mnm-skirmish-cli create OUTPUT | inspect INPUT | resume "
                                "INPUT OUTPUT TICKS [SCRIPT]");
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
