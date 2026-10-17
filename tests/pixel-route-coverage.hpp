#pragma once

#include <array>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

// Count only calls whose entire physical output and caller state were already
// compared with the original. An entry aggregate cannot prove both of its
// clipped/unclipped interior splices, nor that an admitted call drew anything.
template<std::size_t N> struct PixelRouteCoverage {
    struct Route {
        std::array<std::uint64_t, 2> native{}, changed{};
    };
    std::array<Route, N> routes{};

    void matched(std::size_t route, unsigned format, bool changed) {
        if (route >= N || format >= 2)
            throw std::runtime_error("Invalid pixel route coverage identity");
        ++routes[route].native[format];
        if (changed) ++routes[route].changed[format];
    }

    void require(const std::array<std::string, N>& names) const {
        for (std::size_t i = 0; i < N; ++i)
            for (unsigned format = 0; format < 2; ++format)
                if (!routes[i].native[format] || !routes[i].changed[format])
                    throw std::runtime_error("Missing independently matched drawing for " +
                                             names[i] + " format=" + std::to_string(format));
    }

    void write(std::ostream& out, const std::array<std::string, N>& names) const {
        out << "\"route_coverage\":[";
        for (std::size_t i = 0; i < N; ++i) {
            if (i) out << ',';
            const auto& r = routes[i];
            out << "{\"route\":\"" << names[i] << "\",\"native_by_format\":["
                << r.native[0] << ',' << r.native[1] << "],\"changed_by_format\":["
                << r.changed[0] << ',' << r.changed[1] << "]}";
        }
        out << ']';
    }
};
