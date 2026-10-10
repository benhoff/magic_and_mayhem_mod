#pragma once
#include "../../renderer/blit.hpp"
#include "../../protocols/include/mnm/world_rolling_v1.h"
#include <QFile>
#include <optional>
namespace mnm::legacy {
struct RollingInput {std::uint64_t sequence;std::vector<std::uint8_t> bytes;};
struct RollingCompletion {std::uint64_t sequence;render::Image image;};
// SPSC cross-process channel. Calls are nonblocking: false/nullopt means wait
// for the peer, not skip a queue. The orchestrator owns timeout/cancel policy.
class WorldRollingChannel final {
public:
  enum class Role {producer,consumer};
  static void create(const QString& path,std::uint64_t session);
  WorldRollingChannel(const QString& path,std::uint64_t session,Role);
  ~WorldRollingChannel();
  WorldRollingChannel(const WorldRollingChannel&)=delete;
  WorldRollingChannel& operator=(const WorldRollingChannel&)=delete;
  bool publish(const std::vector<std::uint8_t>&);
  std::optional<RollingInput> poll();
  void complete(std::uint64_t sequence,const render::Image&);
  std::optional<RollingCompletion> reap();
  void cancel();
  bool cancelled() const;
  std::uint64_t published() const {return published_;}
  std::uint64_t completed() const {return consumed_;}
  std::uint64_t reclaimed() const {return reclaimed_;}
private:
  void identity() const;
  [[noreturn]] void refuse(const char*);
  std::uint32_t* word(std::size_t at) const;
  std::size_t slot(std::uint64_t sequence) const;
  QFile file_;uchar* mapping_=nullptr;
  std::uint64_t session_,published_=0,consumed_=0,reclaimed_=0,leased_=0;
  Role role_;
  bool attached_=false;
};
}
