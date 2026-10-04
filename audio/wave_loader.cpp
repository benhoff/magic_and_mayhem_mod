#include "wave_loader.hpp"
#include <utility>

namespace mnm::audio {
AssetInputError::AssetInputError(mnm::assets::Error error)
    : std::runtime_error(error.operation + ": " + error.detail), error_(std::move(error)) {}

Wave loadWave(mnm::assets::AssetFile& input) {
    auto result = mnm::assets::readWhole(input, WaveInputLimit);
    if (const auto* error = std::get_if<mnm::assets::Error>(&result)) throw AssetInputError(*error);
    return readWave(std::get<std::vector<std::uint8_t>>(result));
}
}
