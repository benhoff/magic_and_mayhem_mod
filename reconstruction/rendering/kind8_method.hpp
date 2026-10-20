#pragma once
#include "kind8_class.h"
#include <array>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
enum class Kind8Method { additive, inheritedAdditive, ownerAdditive, colour, flagOnly, empty };
struct Kind8Class {
    Kind8Method method;
    unsigned shapeOffset;
};
// Build-specific numeric identities are compared only; never dereferenced.
inline Kind8Class kind8Class(std::uint32_t table, std::uint32_t method) {
    const auto found=mnm_kind8_class(table,method);
    switch(found.method){
    case MNM_K8_ADDITIVE:return {Kind8Method::additive,found.shape_offset};
    case MNM_K8_INHERITED:return {Kind8Method::inheritedAdditive,found.shape_offset};
    case MNM_K8_OWNER:return {Kind8Method::ownerAdditive,found.shape_offset};
    case MNM_K8_COLOUR:return {Kind8Method::colour,found.shape_offset};
    case MNM_K8_FLAG_ONLY:return {Kind8Method::flagOnly,0};
    case MNM_K8_EMPTY:return {Kind8Method::empty,0};
    default:break;
    }
    throw std::invalid_argument("Unrecovered kind8 class/method pair");
}
struct Kind8Owner {
    bool present{};
    std::uint32_t gate{};
    std::array<std::uint32_t, 8> players{};
};
struct Kind8Flags { std::uint8_t at40{}, at44{}; };
// Raster receives a typed class and coordinates and resolves its own owned
// shape. No process pointer, raw diagnostic object, or original pixel seed is
// an input. The caller owns construction/lifetime and raster admission.
template<class Raster>
std::uint16_t drawKind8(Kind8Class type, Kind8Flags& flags, const Kind8Owner& owner,
        std::int32_t player, std::int32_t x, std::int32_t y, Raster&& raster) {
    const unsigned expectedOffset = type.method == Kind8Method::additive ? 0x30 :
        type.method == Kind8Method::ownerAdditive || type.method == Kind8Method::inheritedAdditive ? 0x34 :
        type.method == Kind8Method::colour ? 0x1c : 0;
    if (type.shapeOffset != expectedOffset)
        throw std::invalid_argument("Kind8 class shape does not match method");
    switch (type.method) {
    case Kind8Method::additive:
        flags.at44 = 1; // Before raster execution, including an empty shape.
        return std::uint16_t(raster(type, x, y));
    case Kind8Method::ownerAdditive:
        if (owner.present && owner.gate &&
                (player < 0 || player >= 8 || !owner.players[unsigned(player)])) return 0;
        raster(type, x, y);
        return 0; // Original xor ax,ax; this wrapper writes no drawn flag.
    case Kind8Method::inheritedAdditive:
        return std::uint16_t(raster(type, x, y));
    case Kind8Method::colour:
        return std::uint16_t(raster(type, x, y));
    case Kind8Method::empty:
        return 0; // Exact no-object-write/no-raster draw bodies54ac30/54ac40.
    case Kind8Method::flagOnly:
        flags.at40 = 1;
        return 0;
    }
    throw std::invalid_argument("Unknown kind8 method");
}
}
