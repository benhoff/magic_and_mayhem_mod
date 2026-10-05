#pragma once
#include "commands.hpp"
#include <map>
#include <set>

namespace mnm::render::detail {
void validateCommandFormat(PixelFormat format);
struct CommandDescription {int width,height;PixelFormat format;};
// Shared admission for complete-file decoding and progressive execution.
class CommandState final {
public:
    const CommandDescription& get(unsigned id) const;
    void accept(const SurfaceCommand& command);
    void discardSurfaces(){live.clear();pixels=0;}
    unsigned commands=0;
    std::size_t pixels=0;
    bool ended=false,presented=false;
    std::map<unsigned,CommandDescription> live;
private:
    std::set<unsigned> used;
    std::size_t bytes=16;
};
}
