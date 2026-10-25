#pragma once
#include <array>
#include <cstdint>
namespace mnm::reconstruction {
struct GraphicsInitializerInput {
 bool largeVideo=false,windowed=false;
 std::uint32_t configuration=0,paddingX=0,paddingY=0;
};
enum class GraphicsInitializerStep { setupMode,setupWindow,createPrimary,createOffscreen,clearOffscreen,queryPixelFormat };
struct GraphicsInitializerPlan {
 std::uint32_t width=0,height=0,depth=16,paddingX=0,paddingY=0;
 std::array<std::uint32_t,27> primary{},offscreen{};
 std::array<std::uint32_t,25> clearEffects{};
 std::uint32_t clearFlags=0x01000400,pixelFormatBytes=32;
 std::array<GraphicsInitializerStep,5> steps{};
};
// Recovered successful 4e40d0 request construction and boundary ordering, using
// owned descriptor values. No surfaces, pointers, alternate backing or pixels.
// Native policy refuses dimension overflow; original unsafe domains are separate.
GraphicsInitializerPlan planGraphicsInitialization(GraphicsInitializerInput);
}
