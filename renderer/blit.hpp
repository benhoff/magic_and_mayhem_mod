#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mnm::render {
struct Image {
    int width=0, height=0;
    // One unsigned native pixel per word, including palette indices/unused bits.
    // Row zero is the logical top row. There is no row padding here.
    std::vector<std::uint32_t> pixels;
};
struct Rect {int left=0, top=0, right=0, bottom=0;};
struct Blit {
    Image source, destination;
    unsigned bits=0;
    Rect sourceRect;
    int destinationX=0, destinationY=0;
    std::optional<std::uint32_t> sourceKey;
};
void validate(const Blit& command);
struct Driver {std::string vendor, renderer, version;};

// Qt GUI-thread owner of a dedicated OpenGL 3.3 context. No captured output is
// supplied to draw(): only source pixels, the old destination and the command.
class GlBlitter final {
public:
    GlBlitter();
    ~GlBlitter();
    GlBlitter(const GlBlitter&)=delete;
    GlBlitter& operator=(const GlBlitter&)=delete;
    Image draw(const Blit& command);
    Driver driver() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
