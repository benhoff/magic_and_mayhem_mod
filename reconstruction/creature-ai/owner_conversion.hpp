#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
struct CreatureControlLimit {
  std::int32_t current = 0, maximum = 0;
};
struct OwnerConversionState {
  std::vector<std::int32_t> cursor;
  std::vector<CreatureControlLimit> limits;
};
struct OwnerConversionServices {
  std::function<std::optional<unsigned>(int)> subject;
  std::function<void(unsigned)> notify;
  std::function<void(AutonomousCreature &, int)> transfer;
};
// 514c60: bonus124==2 caller. Ten original slots per scan, fixed125 boundary.
// World actor storage remains stable across notification/transfer callbacks.
void scanOwnerConversion(AutonomousCreature &, AutonomousWorld &,
                         OwnerConversionState &,
                         const OwnerConversionServices &);
} // namespace mnm::reconstruction::combat
