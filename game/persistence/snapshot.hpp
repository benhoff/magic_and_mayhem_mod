#pragma once
#include "simulation/world.hpp"
#include <filesystem>

namespace mnm::game {
// Native v1-v6 lifecycle/movement formats; not original Magic & Mayhem .sav files.
Bytes encodeSnapshot(const State&,const Limits& = {});
State decodeSnapshot(const Bytes&,const Limits& = {});
State readSnapshot(const std::filesystem::path&,const Limits& = {});
struct CommitResult {
    bool durable=true;
    std::string detail; // A directory-sync failure occurs after publication.
};
// POSIX store: temporary sibling, file sync, atomic publish, directory sync.
// Default admission refuses existing destinations. Errors before publication throw.
CommitResult writeSnapshot(const std::filesystem::path&,const State&,bool overwrite=false,const Limits& = {});
void restoreSnapshot(World&,const std::filesystem::path&);
}
