#include "screen_resources.hpp"
#include <charconv>
#include <stdexcept>

namespace mnm::assets {
ScreenResources::ScreenResources(AssetStore store, ScreenResourceSpec spec,
                                 ResourceLimits limits,
                                 ScreenResourceCheckpoint checkpoint)
    : manager_(std::move(store), limits), controls_(std::move(spec.controls)) {
  std::size_t bound = 0, decoded = 0;
  for (const auto &binding : spec.images) {
    manager_.bind(binding.first, binding.second);
    ++bound;
    if (checkpoint)
      checkpoint(bound, decoded);
  }
  for (const auto &binding : spec.images) {
    manager_.load(binding.first);
    ++decoded;
    if (checkpoint)
      checkpoint(bound, decoded);
  }
}
ScreenResourceRequest ScreenResources::request(const ResourceId &id) {
  return {manager_.request(id), lifetime_};
}
const VisualResource &
ScreenResources::adopt(const ScreenResourceRequest &request,
                       PreparedResource &&prepared) {
  const auto lifetime = request.lifetime_.lock();
  if (!lifetime || lifetime != lifetime_)
    throw std::invalid_argument(
        "Screen completion belongs to a retired or different screen");
  if (!(request.request_.id() == prepared.id()))
    throw std::invalid_argument(
        "Screen completion has a different resource ID");
  return manager_.adopt(std::move(prepared));
}
void ScreenResources::retire(const ResourceId &id) { manager_.unload(id); }
void ScreenResourceOwner::replace(AssetStore store, ScreenResourceSpec spec,
                                  ResourceLimits limits,
                                  ScreenResourceCheckpoint checkpoint) {
  auto request = std::make_shared<const int>(0);
  request_ = request;
  auto next = std::make_unique<ScreenResources>(
      std::move(store), std::move(spec), limits, std::move(checkpoint));
  if (request_ != request)
    throw std::invalid_argument(
        "Screen replacement was superseded or cancelled");
  current_ = std::move(next);
}
std::string screenScoreText(std::int32_t score) {
  char text[12];
  const auto result = std::to_chars(text, text + sizeof(text), score);
  if (result.ec != std::errc{})
    throw std::runtime_error("Signed screen score formatting failed");
  return {text, result.ptr};
}
} // namespace mnm::assets
