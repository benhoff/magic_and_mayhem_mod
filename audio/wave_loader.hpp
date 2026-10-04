#pragma once
#include "asset_file.hpp"
#include "buffers.hpp"
#include <stdexcept>

namespace mnm::audio {
inline constexpr std::int64_t WaveInputLimit = 32 * 1024 * 1024;

class AssetInputError : public std::runtime_error {
public:
    explicit AssetInputError(mnm::assets::Error error);
    const mnm::assets::Error& error() const { return error_; }
private:
    mnm::assets::Error error_;
};

// Native input adapter. Rewinds, bounds and owns raw bytes before invoking the
// existing strict parser. Returned PCM is independent of the input handle.
// Asset failures retain structured diagnostics; parser failures remain the
// existing runtime_error behavior. No Qt types or original-game pointers.
Wave loadWave(mnm::assets::AssetFile& input);
}
