#pragma once
#include "wave_loader.hpp"

namespace mnm::audio {
struct SourceWave {
    Wave pcm;
    std::uint32_t durationMs=0;
};
// Read-only source namespace, independent of engine reconstruction and widgets.
// Requests are leaf WAV names in a session-selected Sounds directory. The
// optional quoted-leaf adaptation is explicit, and only retries notFound.
class WaveSource {
public:
    explicit WaveSource(mnm::assets::AssetStore assets,bool dequoteMissingLeaf=false)
        :assets_(std::move(assets)),dequoteMissingLeaf_(dequoteMissingLeaf){}
    SourceWave load(std::string_view leaf) const;
    // Whole file bytes for the selected audio pool budget; does not decode PCM.
    std::uint32_t fileBytes(std::string_view leaf) const;
private:
    std::unique_ptr<mnm::assets::AssetFile> open(std::string_view leaf) const;
    mnm::assets::AssetStore assets_;
    bool dequoteMissingLeaf_;
};
}
