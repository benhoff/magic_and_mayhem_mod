#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>
namespace mnm::audio {
struct MusicTrack {
    std::uint32_t number=0,startSector=0,endSector=0;
    std::uint64_t frames=0;
    std::filesystem::path file;
};
// Native prepared-CD policy, independent of legacy MCI and application widgets.
// Track1 is the data span and has no playable file. Audio tracks start at2.
class MusicCatalog final {
public:
    static MusicCatalog load(const std::filesystem::path& root);
    const std::vector<MusicTrack>& tracks()const{return tracks_;}
    const MusicTrack* track(std::uint32_t number)const;
private:
    std::vector<MusicTrack> tracks_;
};
}
