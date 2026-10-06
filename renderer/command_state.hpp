#pragma once
#include "commands.hpp"
#include <map>
#include <set>

namespace mnm::render::detail {
void validateCommandFormat(PixelFormat format);
struct PaletteDescription {unsigned generation;std::vector<Rgb> colors;};
struct CommandDescription {int width,height;PixelFormat format;ClipperState clipper{};};
// Signed two's-complement coordinates have explicit portable wire conversion.
inline int signedCommandCoordinate(std::uint32_t value){
    return value<=INT32_MAX?int(value):int(std::int64_t(value)-0x100000000LL);
}
inline ClipperState commandClipper(const SurfaceCommand& c){
    return {c.words[1]!=0,c.words[1]==1?std::optional<std::vector<Rect>>(c.regions):std::nullopt};
}
inline SurfaceCopyRequest commandSurfaceCopy(const SurfaceCommand& c){
    const auto& w=c.words;
    return {w[2]?SurfaceCopyApi::BltFast:SurfaceCopyApi::Blt,
        {signedCommandCoordinate(w[5]),signedCommandCoordinate(w[6]),signedCommandCoordinate(w[7]),signedCommandCoordinate(w[8])},
        {signedCommandCoordinate(w[9]),signedCommandCoordinate(w[10]),signedCommandCoordinate(w[11]),signedCommandCoordinate(w[12])},
        w[3],bool(w[4]&1),bool(w[4]&2)};
}
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
    unsigned lastCopySequence=0;
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
