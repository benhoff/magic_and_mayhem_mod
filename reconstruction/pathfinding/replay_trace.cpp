// Replay wrapper behavior with measured search output; NOT a search validator.
#include "route_request.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace mnm::reconstruction;

std::int32_t signed_bits(std::uint32_t bits) {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

struct MeasuredSearch {
    RouteSnapshot result{};
    std::uint32_t flag_after;
    std::int32_t budget_after;
    std::uint32_t observed[6]{};
};

void measured_search(RouteContextPrefix& context, ObjectPrefix&,
    std::uint32_t unknown_argument, std::int32_t x, std::int32_t y,
    std::int32_t z, std::int32_t& budget, void* data) {
    auto& sample = *static_cast<MeasuredSearch*>(data);
    sample.observed[0] = static_cast<std::uint32_t>(x);
    sample.observed[1] = static_cast<std::uint32_t>(y);
    sample.observed[2] = static_cast<std::uint32_t>(z);
    sample.observed[3] = unknown_argument;
    sample.observed[4] = static_cast<std::uint32_t>(budget);
    sample.observed[5] = context.unknown_route_flag;
    std::memcpy(&context.route, &sample.result, sizeof(sample.result));
    context.unknown_route_flag = static_cast<std::uint8_t>(sample.flag_after);
    budget = sample.budget_after;
}

int main() {
    try {
        std::uint32_t inputs[9];
        std::cin >> std::hex;
        for (auto& input : inputs) {
            if (!(std::cin >> input)) throw std::runtime_error("missing trace header");
        }
        MeasuredSearch sample{};
        sample.flag_after = inputs[7];
        sample.budget_after = signed_bits(inputs[8]);
        std::uint32_t snapshot[131];
        for (auto& word : snapshot) {
            if (!(std::cin >> word)) throw std::runtime_error("missing snapshot word");
        }
        std::memcpy(&sample.result, snapshot, sizeof(snapshot));
        EngineState engine{signed_bits(inputs[5]), signed_bits(inputs[6]), signed_bits(inputs[4])};
        ObjectPrefix object{};
        const bool result = route_request(object, engine,
            signed_bits(inputs[0]), signed_bits(inputs[1]), signed_bits(inputs[2]),
            inputs[3], {measured_search, &sample});
        std::cout << std::hex;
        for (auto word : sample.observed) std::cout << word << ' ';
        std::cout << static_cast<unsigned>(engine.scratch.unknown_route_flag) << ' '
                  << static_cast<std::uint32_t>(sample.budget_after) << ' '
                  << result << ' ' << object.route_present << ' ' << object.unknown_d03 << ' '
                  << static_cast<std::uint32_t>(engine.node_budget) << ' ';
        std::memcpy(snapshot, &object.route, sizeof(snapshot));
        for (auto word : snapshot) std::cout << word << ' ';
        std::cout << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
