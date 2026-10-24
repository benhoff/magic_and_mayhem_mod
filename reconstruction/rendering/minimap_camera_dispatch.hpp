#pragma once
#include <cstdint>
#include <utility>

namespace mnm::reconstruction {
// NoCD 555c3c..555c6e. This is the owner's raw +a4 selector, not an
// orientation supplied to an individual camera drawing kernel. In particular,
// selectors 0 and 2 alias camera0; no value here selects camera3.
constexpr int originalMinimapCamera(std::uint32_t selector) {
  return selector == 0 || selector == 2 ? 0 :
         selector == 1 ? 2 : selector == 3 ? 1 : -1;
}

// The common creature-marker call still runs after the unsigned default path.
// Callbacks supply the owned receiver/storage; this selector owns no pixels.
template<class Camera, class Markers>
void dispatchOriginalMinimapCamera(std::uint32_t selector, Camera&& camera,
                                   Markers&& markers) {
  const int selected = originalMinimapCamera(selector);
  if (selected >= 0) std::forward<Camera>(camera)(unsigned(selected));
  std::forward<Markers>(markers)();
}
}
