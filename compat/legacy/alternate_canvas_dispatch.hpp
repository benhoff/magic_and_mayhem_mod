#pragma once
#include "../../renderer/alternate_canvas.hpp"
#include "../../reconstruction/rendering/alternate_canvas_dispatch.hpp"

namespace mnm::compat {
// Synchronous boundary: the native owner supplies one centered-copy generation;
// the recovered dispatcher retains its original callback/Lock/Unlock ordering.
// All ordinary backing and callbacks obey the borrowed dispatch contract.
template<class Lock, class World, class Overlay, class Unlock>
bool dispatchOwnedAlternateCanvas(render::AlternateCanvas& owner,
    const render::AlternateCanvas::Handle& generation,
    reconstruction::DispatchBinding ordinary, reconstruction::DispatchClip clip,
    std::uint32_t overlays, reconstruction::DispatchState& state,
    Lock lock, World world, Overlay overlay, Unlock unlock) {
  auto frame = owner.acquire(generation);
  const auto layout = frame.layout();
  if (ordinary.stride != layout.visibleWidth() || ordinary.height != layout.visibleHeight())
    throw std::invalid_argument("Native alternate copy requires visible ordinary pitch and height");
  reconstruction::AlternateDispatch input;
  input.highResolution = layout.highResolution;
  input.xPadding = layout.xPadding; input.yPadding = layout.yPadding;
  input.overlays = overlays; input.ordinary = ordinary; input.clip = clip;
  input.alternate = {&frame.words(), 0};
  input.copySource = {&frame.words(), layout.copyOffset()};
  // state otherwise outlives frame and could retain a pointer to retired storage.
  // Save only ordinary/incoming binding; no alternate reference escapes on Lock
  // failure or callback unwinding. Clip and pixel effects retain their order.
  const auto incoming = state.binding;
  struct Restore {
    reconstruction::DispatchState& state;
    reconstruction::DispatchBinding incoming;
    std::vector<std::uint16_t>* alternate;
    ~Restore() { if (state.binding.span.words == alternate) state.binding = incoming; }
  } restore{state, incoming, &frame.words()};
  return reconstruction::dispatchAlternateCanvas(input, state, lock, world, overlay, unlock);
}
} // namespace mnm::compat
