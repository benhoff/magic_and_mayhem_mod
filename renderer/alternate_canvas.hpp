#pragma once
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mnm::render {
// Intentional native policy, independent of the original alternate globals.
// Single-threaded. A frame has one mutable lease. No resize can invalidate it.
class AlternateCanvas final {
  struct Storage;
public:
  struct Layout {
    bool highResolution = false;
    std::uint32_t xPadding = 0, yPadding = 0;
    std::uint32_t visibleWidth() const { return highResolution ? 800 : 640; }
    std::uint32_t visibleHeight() const { return highResolution ? 600 : 480; }
    std::uint32_t stride() const { return visibleWidth() + 2 * xPadding; }
    std::uint32_t height() const { return visibleHeight() + 2 * yPadding; }
    // Native choice: copy the centered visible rectangle from the World plane.
    std::size_t copyOffset() const { return std::size_t(yPadding) * stride() + xPadding; }
  };
  class Handle {
    friend class AlternateCanvas;
    std::weak_ptr<Storage> storage_;
    explicit Handle(const std::shared_ptr<Storage>& p) : storage_(p) {}
  public:
    Handle() = default;
    bool expired() const { return storage_.expired(); }
  };
  class Lease {
    friend class AlternateCanvas;
    std::shared_ptr<Storage> storage_;
    std::shared_ptr<unsigned char> token_;
    Lease(std::shared_ptr<Storage> p, std::shared_ptr<unsigned char> token)
        : storage_(std::move(p)), token_(std::move(token)) {}
  public:
    Lease() = default;
    Lease(const Lease&) = delete;
    Lease& operator=(const Lease&) = delete;
    Lease(Lease&&) = default;
    Lease& operator=(Lease&&) = default;
    const Layout& layout() const { return storage().layout; }
    // Scoped borrowing for pixel services, including legacy composition.
    // The borrower must not resize/swap the vector or retain references beyond
    // this lease. Ownership stays here; pixels persist across failed Locks.
    std::vector<std::uint16_t>& words() { return storage().words; }
    const std::vector<std::uint16_t>& words() const { return storage().words; }
  private:
    Storage& storage() const {
      if (!storage_) throw std::logic_error("Empty alternate canvas lease");
      return *storage_;
    }
  };

  AlternateCanvas() = default;
  AlternateCanvas(const AlternateCanvas&) = delete;
  AlternateCanvas& operator=(const AlternateCanvas&) = delete;
  AlternateCanvas(AlternateCanvas&&) = delete;
  AlternateCanvas& operator=(AlternateCanvas&&) = delete;

  Handle publish(Layout layout) {
    // Bound a plane to 4096 x 4096 and prevent overlapping mutable generations.
    // Validate before multiplication/allocation; failed publication keeps the
    // current handle and every pixel intact. Allocation completes before swap.
    if (layout.xPadding > (4096 - layout.visibleWidth()) / 2 ||
        layout.yPadding > (4096 - layout.visibleHeight()) / 2)
      throw std::invalid_argument("Native alternate canvas extent");
    if (activeLease()) throw std::logic_error("Alternate canvas frame is leased");
    auto next = std::make_shared<Storage>(layout);
    current_.swap(next);
    return Handle(current_);
  }
  Lease acquire(const Handle& handle) {
    if (activeLease()) throw std::logic_error("Alternate canvas frame is leased");
    auto p = handle.storage_.lock();
    if (!p || p != current_) throw std::invalid_argument("Stale alternate canvas generation");
    auto token = std::make_shared<unsigned char>(0);
    leased_ = token;
    return Lease(std::move(p), std::move(token));
  }
  // Unpublish now; in-flight work can finish. No new publication is admitted
  // until the old lease ends, so retained generation count stays bounded.
  void retire() { current_.reset(); }
  bool published() const { return bool(current_); }

private:
  struct Storage {
    const Layout layout;
    std::vector<std::uint16_t> words;
    explicit Storage(Layout value) : layout(value), words(std::size_t(value.stride()) * value.height(), 0) {}
  };
  std::shared_ptr<Storage> current_;
  std::weak_ptr<unsigned char> leased_;
  bool activeLease() const { return !leased_.expired(); }
};
} // namespace mnm::render
