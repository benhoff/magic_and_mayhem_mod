#include "../assets/resources/screen_resources.hpp"
#include "resource-fixtures.hpp"
#include <QCoreApplication>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <thread>

// Exercise C++ binding/staging allocations before decoding (excluding Qt shared
// library internals, whose noexcept allocation policy is separate), including
// partially decoded resources. One injected failure per attempt; diagnostics
// run disarmed.
static thread_local long failAllocation = -1;
extern "C" char __executable_start, etext;
static void *allocateCpp(std::size_t n, void *caller) {
  const auto address = reinterpret_cast<std::uintptr_t>(caller);
  const bool owned =
      address >= reinterpret_cast<std::uintptr_t>(&__executable_start) &&
      address < reinterpret_cast<std::uintptr_t>(&etext);
  if (owned && failAllocation >= 0 && failAllocation-- == 0)
    throw std::bad_alloc();
  if (void *p = std::malloc(n ? n : 1))
    return p;
  throw std::bad_alloc();
}
void *operator new(std::size_t n) {
  return allocateCpp(n, __builtin_return_address(0));
}
void *operator new[](std::size_t n) {
  return allocateCpp(n, __builtin_return_address(0));
}
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
using namespace mnm::assets;
using namespace resource_test;
static ScreenResourceSpec spec() {
  ScreenResourceSpec result;
  result.images = {{{ResourceKind::ui, "selection"}, spr("indexed.spr")},
                   {{ResourceKind::ui, "markers"}, spr()},
                   {{ResourceKind::ui, "background"},
                    {ResourceImageFormat::jpeg, "ui.jpg", {}, {}, {}}},
                   {{ResourceKind::ui, "bmp"},
                    {ResourceImageFormat::bmp, "ui.bmp", {}, {}, {}}},
                   {{ResourceKind::ui, "pcx"},
                    {ResourceImageFormat::pcx, "ui.pcx", {}, {}, {}}}};
  for (unsigned i = 0; i < 21; ++i)
    result.controls.push_back({"Control " + std::to_string(i), {1, 2, 3, 4}});
  return result;
}
int main(int argc, char **argv) try {
  QCoreApplication app(argc, argv);
  QTemporaryDir temporary;
  require(temporary.isValid(), "fixture root");
  const auto root = std::filesystem::path(temporary.path().toStdString());
  populate(root);
  const auto source = store(root);
  const ResourceId selection{ResourceKind::ui, "selection"};
  ScreenResourceOwner owner;
  auto input = spec();
  owner.replace(source, input);
  input.controls[0].text = "caller changed";
  input.images.clear();
  require(owner.current()->controls()[0].text == "Control 0",
          "control text borrowed");
  require(owner.current()->stats().residentResources == 5,
          "concrete providers missing");
  const auto *old = owner.current();
  // Every completed provider prefix is unwound. The already published screen
  // and its control/image references survive each failed replacement.
  for (unsigned fail = 0; fail < 5; ++fail) {
    auto broken = spec();
    broken.images[fail].second.image = "absent";
    rejected([&] { owner.replace(source, broken); });
    require(owner.current() == old && old->stats().residentResources == 5,
            "partial replacement published");
  }
  write(root, "bad.spr", Bytes{0, 1, 2});
  auto malformed = spec();
  malformed.images[1].second.image = "bad.spr";
  rejected([&] { owner.replace(source, malformed); });
  require(owner.current() == old, "decoder failure retired old screen");
  ResourceLimits limit;
  limit.residentResources = 4;
  rejected([&] { owner.replace(source, spec(), limit); });
  require(owner.current() == old, "budget failure retired old screen");
  unsigned allocationFailures = 0;
  for (unsigned at = 0; at < 2000; ++at) {
    auto next = spec();
    bool failed = false;
    failAllocation = at;
    try {
      owner.replace(source, std::move(next), {},
                    [](std::size_t bound, std::size_t decoded) {
                      if (bound == 5 && decoded == 0)
                        failAllocation = -1;
                    });
    } catch (const std::bad_alloc &) {
      failed = true;
    } catch (...) {
      failAllocation = -1;
      throw;
    }
    failAllocation = -1;
    if (!failed)
      break;
    ++allocationFailures;
    require(owner.current() == old && old->stats().residentResources == 5,
            "allocation failure published replacement");
  }
  require(allocationFailures > 5 && allocationFailures < 2000,
          "allocation failure enumeration incomplete");
  old = owner.current();
  for (unsigned prefix = 1; prefix <= 5; ++prefix) {
    rejected([&] {
      owner.replace(source, spec(), {}, [&](std::size_t, std::size_t decoded) {
        if (decoded == prefix)
          throw std::bad_alloc();
      });
    });
    require(owner.current() == old && old->stats().residentResources == 5,
            "decoded prefix rollback failed");
  }
  // Work owns its inputs across owner retirement. Publication is tied to the
  // original lifetime and ResourceManager binding token, even for identical
  // IDs.
  auto late = owner.current()->request(selection);
  auto done = late.prepare();
  owner.reset();
  owner.replace(source, spec());
  rejected([&] { owner.current()->adopt(late, std::move(done)); });
  auto ticket = owner.current()->request(selection);
  auto prepared = ticket.prepare();
  owner.current()->retire(selection);
  rejected([&] { owner.current()->adopt(ticket, std::move(prepared)); });
  auto fresh = owner.current()->request(selection);
  auto ready = fresh.prepare();
  const auto revision =
      owner.current()->adopt(fresh, std::move(ready)).revision;
  ScreenResourceOwner other;
  other.replace(source, spec());
  auto foreign = other.current()->request(selection);
  auto foreignData = foreign.prepare();
  rejected([&] { owner.current()->adopt(foreign, std::move(foreignData)); });
  // Cancellation may throw between file/decoder stages. Retry uses a new owned
  // preparation; no borrowed decoder or control pointer crosses the worker.
  unsigned checkpoints = 0;
  rejected([&] {
    fresh.prepare([&] {
      if (++checkpoints == 2)
        throw std::runtime_error("cancel");
    });
  });
  owner.current()->retire(selection);
  auto workerTicket = owner.current()->request(selection);
  std::unique_ptr<PreparedResource> workerResult;
  std::thread worker([&] {
    workerResult = std::make_unique<PreparedResource>(workerTicket.prepare());
  });
  worker.join();
  require(
      owner.current()->adopt(workerTicket, std::move(*workerResult)).revision >
          revision,
      "fresh completion not adopted");
  rejected([&] {
    owner.replace(source, spec(), {},
                  [&](std::size_t bound, std::size_t decoded) {
                    if (bound == 5 && decoded == 0)
                      owner.reset();
                  });
  });
  require(owner.current() == nullptr, "cancelled staging republished a screen");
  bool nested = false;
  const ScreenResources *nestedScreen = nullptr;
  rejected([&] {
    owner.replace(source, spec(), {},
                  [&](std::size_t bound, std::size_t decoded) {
                    if (!nested && bound == 5 && decoded == 0) {
                      nested = true;
                      owner.replace(source, spec());
                      nestedScreen = owner.current();
                    }
                  });
  });
  require(owner.current() == nestedScreen && nestedScreen,
          "superseded staging replaced newer screen");
  owner.reset();
  other.reset();
  for (const auto value :
       {std::numeric_limits<std::int32_t>::min(), -1000000, -100000, -99999, -1,
        0, 1, 999999, 1000000, std::numeric_limits<std::int32_t>::max()})
    require(screenScoreText(value) == std::to_string(value),
            "full signed score");
  std::cout
      << "{\"providers\":5,\"controls\":21,\"prefix_failures\":5,\"allocation_"
         "failures\":"
      << allocationFailures
      << ",\"late_and_foreign_rejected\":true,\"score_range\":\"int32\"}\n";
  return 0;
} catch (const std::exception &e) {
  failAllocation = -1;
  std::cerr << e.what() << '\n';
  return 1;
}
