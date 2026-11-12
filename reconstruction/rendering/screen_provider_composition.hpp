#pragma once
#include "actor_profile_palette_providers.hpp"
#include "screen_layout_provider.hpp"
#include "screen_palette_providers.hpp"
#include "screen_resource_providers.hpp"
#include "screen_surface_providers.hpp"

namespace mnm::reconstruction {
// Recovered providers used by the screen acquisition/release adapters. The
// platform owns files, profile API, heap primitives, COM and the image codec.
template <class M> class ScreenProviderComposition {
  M &m;

public:
  explicit ScreenProviderComposition(M &memory) : m(memory) {}
  std::optional<std::uint32_t>
  tryRun(std::uint32_t entry, std::uint32_t self,
         std::initializer_list<std::uint32_t> args = {},
         std::uint32_t second = 0) {
    if (auto v = ScreenLayoutProvider<M>(m).tryRun(entry, self, second, args))
      return v;
    if (auto v =
            ScreenResourceProviders<M>(m).tryRun(entry, self, args, second))
      return v;
    if (auto v = ScreenSurfaceProviders<M>(m).tryRun(entry, self, args))
      return v;
    if (auto v = ScreenPaletteProviders<M>(m).tryRun(entry, self, second, args))
      return v;
    switch (entry) {
    case 0x57d310:
    case 0x57d690:
      return actor_services::FrontierProviders<M>(m).run(entry, self, args);
    case 0x582440:
    case 0x582a80:
    case 0x583330:
    case 0x583bb0:
    case 0x586660:
    case 0x5870d0:
    case 0x585d80:
    case 0x587e70:
    case 0x588730:
    case 0x588bf0:
      return actor_services::ProfilePaletteProviders<M>(m).run(entry, self,
                                                               args, second);
    case 0x5977d0:
    case 0x597880:
    case 0x598f70:
      return actor_services::Dependencies<M>(m).run(entry, self, args);
    default:
      return std::nullopt;
    }
  }
};
} // namespace mnm::reconstruction
