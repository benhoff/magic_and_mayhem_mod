#pragma once
#include "commands.hpp"
#include <map>
#include <set>

namespace mnm::render::detail {
void validateCommandFormat(PixelFormat format);
struct PaletteDescription {unsigned generation;std::vector<Rgb> colors;};
struct CommandDescription {int width,height;PixelFormat format;};
// Shared admission for complete-file decoding and progressive execution.
class CommandState final {
public:
    explicit CommandState(CommandStreamMode mode=CommandStreamMode::Bounded):mode(mode){}
    const CommandDescription& get(unsigned id) const;
    void accept(const SurfaceCommand& command);
    void discardSurfaces(){live.clear();palettes.clear();bindings.clear();pixels=0;}
    unsigned commands=0;
    std::size_t pixels=0;
    bool ended=false,presented=false;
    std::map<unsigned,CommandDescription> live;
    std::map<unsigned,PaletteDescription> palettes;
    std::map<unsigned,unsigned> bindings;
    const PaletteDescription& palette(unsigned id,unsigned generation) const;
private:
    CommandStreamMode mode;
    unsigned lastCreated=0,lastPaletteCreated=0,version=0;
    std::set<unsigned> used;
    std::size_t bytes=16;
};
}
