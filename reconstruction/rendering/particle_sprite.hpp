#pragma once
#include <cstdint>
#include <stdexcept>
#include "kind8_random.hpp"

namespace mnm::reconstruction {
// No-CD 40209ca7, 5492b0/549730/549ba0/54a4a0. Source identities are
// owned resource tokens. Backend names describe exact original entry points.
enum class ParticleSpriteEntry { word, indexed, black, fallback, blend50, blend75, blend25 };
struct ParticleSpriteSource { std::uint32_t token{}; bool words{}; };
struct ParticleSpriteGate {
    bool resource{};
    std::int32_t player{};
    bool playerVisible{};
    std::uint32_t frameCount{};
};
template<class Resolve, class Draw>
std::uint16_t drawRandomParticleSprite(bool chance, const ParticleSpriteGate& gate,
        Kind8Random& random, std::int32_t x, std::int32_t y,
        Resolve&& resolve, Draw&& draw) {
    if (!gate.resource || gate.player < 0 || gate.player >= 8 ||
            !gate.playerVisible || !gate.frameCount) return 0;
    if (chance && (random.next() & 0x7fffffffu) % 100 <= 50) return 0;
    const auto frame = (random.next() & 0x7fffffffu) % gate.frameCount;
    const auto source = resolve(frame);
    if (source.words) draw(ParticleSpriteEntry::word, source.token, x, y, 0, 0);
    else if (std::uint16_t(draw(ParticleSpriteEntry::indexed, source.token, x, y, 0, 0)) == 1)
        draw(ParticleSpriteEntry::fallback, source.token, x, y, 0, 0);
    return 0; // Only AX is a return contract; upper EAX is incidental.
}
template<class Draw>
std::uint16_t drawFadingParticleSprite(bool resource, const ParticleSpriteSource& source,
        std::int32_t category, std::int32_t remaining, std::int32_t duration,
        std::int32_t shade, std::uint32_t palette, std::int32_t x, std::int32_t y,
        Draw&& draw) {
    if (!resource || !source.token) return 0;
    if (!duration) throw std::invalid_argument("Original particle duration divides by zero");
    const auto product = std::uint32_t(remaining) * 100u;
    const auto numerator = product <= 0x7fffffffu ? std::int32_t(product) :
        -1 - std::int32_t(0xffffffffu - product);
    if (numerator == INT32_MIN && duration == -1)
        throw std::invalid_argument("Original particle duration division overflows");
    auto phase = numerator / duration;
    if (category == 13) phase /= 2;
    if (phase <= 75) {
        if (source.words) throw std::invalid_argument("Original blend requires an indexed palette pointer");
        draw(phase > 50 ? ParticleSpriteEntry::blend75 :
             phase > 25 ? ParticleSpriteEntry::blend50 : ParticleSpriteEntry::blend25,
             source.token, x, y, shade, palette);
    } else if (source.words) draw(ParticleSpriteEntry::word, source.token, x, y, shade, palette);
    else if (std::uint16_t(draw(shade == -127 ? ParticleSpriteEntry::black :
            ParticleSpriteEntry::indexed, source.token, x, y, shade, palette)) == 1)
        draw(ParticleSpriteEntry::fallback, source.token, x, y, shade, palette);
    return 0;
}
}
