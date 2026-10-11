#pragma once
#include "../../protocols/include/mnm/world_continuous_v4.h"
#include <QFile>
#include <array>
#include <cstdint>
#include <vector>
namespace mnm::legacy {
class WorldContinuousChannel final {
public:
  struct Block {
    std::uint64_t generation;
    std::vector<std::uint8_t> source, request;
  };
  static void create(const QString &, std::uint64_t nonce,
                     std::uint64_t target, bool resident = false);
  explicit WorldContinuousChannel(const QString &);
  ~WorldContinuousChannel();
  WorldContinuousChannel(const WorldContinuousChannel &) = delete;
  bool resident() const { return resident_; }
  bool cancelled() const;
  bool finished() const;
  std::uint64_t target() const;
  std::uint64_t nonce() const;
  void cancel();
  bool poll(Block &);
  void reply(std::uint64_t, const std::vector<std::uint8_t> &);
  bool returned(std::uint64_t, std::vector<std::uint8_t> &);
  bool returnRecords(std::uint64_t, const std::vector<std::uint8_t> &,
                     std::array<std::uint8_t, 192> &);
  void release(std::uint64_t);
  std::array<std::uint32_t, 10> ticks(std::uint64_t) const;

private:
  QFile file_;
  std::uint8_t *map_ = nullptr;
  std::uint64_t expected_ = 1, nonce_ = 0, target_ = 0;
  bool resident_ = false;
  void validate() const;
  std::uint8_t *slot(std::uint64_t) const;
};
} // namespace mnm::legacy
