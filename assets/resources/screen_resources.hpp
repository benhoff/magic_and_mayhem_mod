#pragma once
#include "resource_manager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace mnm::assets {
// Native screen ownership policy. These values never borrow legacy addresses,
// layout-parser scratch, widgets, or a worker's decoder buffers.
struct ScreenControlResource {
  std::string text;
  std::array<std::int32_t, 4> rectangle{};
};
struct ScreenResourceSpec {
  std::vector<std::pair<ResourceId, ResourceRecipe>> images;
  std::vector<ScreenControlResource> controls;
};
using ScreenResourceCheckpoint =
    std::function<void(std::size_t bound, std::size_t decoded)>;
class ScreenResources;
class ScreenResourceRequest final {
public:
  PreparedResource prepare(const std::function<void()> &checkpoint = {}) const {
    return prepareResource(request_, checkpoint);
  }

private:
  friend class ScreenResources;
  ScreenResourceRequest(ResourcePreparation request,
                        std::weak_ptr<const int> lifetime)
      : request_(std::move(request)), lifetime_(std::move(lifetime)) {}
  ResourcePreparation request_;
  std::weak_ptr<const int> lifetime_;
};
// Thread-confined like ResourceManager. Requests own their CPU inputs and may
// be prepared on workers; adoption and retirement belong to the owning thread.
class ScreenResources final {
public:
  ScreenResources(AssetStore store, ScreenResourceSpec spec,
                  ResourceLimits limits = {},
                  ScreenResourceCheckpoint checkpoint = {});
  ScreenResources(const ScreenResources &) = delete;
  ScreenResources &operator=(const ScreenResources &) = delete;
  const std::vector<ScreenControlResource> &controls() const {
    return controls_;
  }
  const VisualResource &image(const ResourceId &id) const {
    return manager_.resident(id);
  }
  ResourceStats stats() const { return manager_.stats(); }
  ScreenResourceRequest request(const ResourceId &id);
  const VisualResource &adopt(const ScreenResourceRequest &,
                              PreparedResource &&);
  void retire(const ResourceId &id);

private:
  ResourceManager manager_;
  std::vector<ScreenControlResource> controls_;
  std::shared_ptr<const int> lifetime_ = std::make_shared<const int>(0);
};
// Build a complete replacement before publishing it. If any provider,
// allocation or decoder throws, the old screen remains owned and the completed
// prefix dies.
class ScreenResourceOwner final {
public:
  void replace(AssetStore store, ScreenResourceSpec spec,
               ResourceLimits limits = {},
               ScreenResourceCheckpoint checkpoint = {});
  const ScreenResources *current() const { return current_.get(); }
  ScreenResources *current() { return current_.get(); }
  void reset() noexcept {
    request_.reset();
    current_.reset();
  }

private:
  std::unique_ptr<ScreenResources> current_;
  std::shared_ptr<const int> request_;
};
// Full signed DWORD range; the original Report seven-byte scratch is unsafe
// beyond six characters. This deliberate policy does not reproduce that fault.
std::string screenScoreText(std::int32_t score);
} // namespace mnm::assets
