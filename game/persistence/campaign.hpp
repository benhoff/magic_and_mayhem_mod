#pragma once
#include "snapshot.hpp"
#include <array>

namespace mnm::game {
// Native campaign data, independent of original .sav layouts and reward rules.
// IDs are caller-owned content identifiers, never pointers or world handles.
struct CampaignIdentity { std::string id, contentRevision; };
struct CampaignWizard {
    std::string id;
    std::uint64_t experience=0;
    std::array<std::int32_t,6> attributes{};
    std::vector<std::string> spells, inventory;
};
struct CampaignRegion {
    std::string id;
    std::optional<std::string> owner;
    bool completed=false;
};
struct CampaignRealm { std::string id; std::vector<CampaignRegion> regions; };
struct CampaignBattle {
    std::string realm, region, map;
    std::vector<std::string> participants;
};
struct CampaignState {
    CampaignIdentity identity;
    std::uint32_t difficulty=0; // Native persisted selection, range 0..3.
    std::uint64_t transition=0; // Caller-owned progression revision; no automatic rewards.
    std::string player, realm, region;
    bool completed=false;
    std::vector<CampaignWizard> wizards;
    std::vector<CampaignRealm> realms;
    std::optional<CampaignBattle> battle;
};
struct CampaignLimits {
    std::uint32_t bytes=4*1024*1024, wizards=80, realms=32, regions=4096;
    std::uint32_t entries=256, stringBytes=255;
};
Bytes encodeCampaign(const CampaignState&,const CampaignLimits& = {});
CampaignState decodeCampaign(const Bytes&,const CampaignLimits& = {});
// The world retains its existing opaque campaign/profile and systems bytes.
struct CampaignCheckpoint { State world; CampaignState campaign; };
// Validates the complete staged world and campaign before replacing progress.
void setCampaign(CampaignCheckpoint&,const CampaignState&,const Limits& = {},const CampaignLimits& = {});
Bytes encodeCampaignSnapshot(const CampaignCheckpoint&,const CampaignIdentity&,const Limits& = {},const CampaignLimits& = {});
// These return staged data. Resource-owning sessions must still run their own
// restore/rebinding validation before publishing a movement/combat world.
CampaignCheckpoint decodeCampaignSnapshot(const Bytes&,const CampaignIdentity&,const Limits& = {},const CampaignLimits& = {});
CampaignCheckpoint readCampaignSnapshot(const std::filesystem::path&,const CampaignIdentity&,const Limits& = {},const CampaignLimits& = {});
CommitResult writeCampaignSnapshot(const std::filesystem::path&,const CampaignCheckpoint&,const CampaignIdentity&,
                                  bool overwrite=false,const Limits& = {},const CampaignLimits& = {});
}
