#pragma once
#include <cstdint>
#include <initializer_list>

namespace mnm::reconstruction {
// No-CD 40209ca7. All addresses are original 32-bit wire identities. Storage
// admission and allocator/virtual endpoint effects belong to the caller.
template<class Memory>
void constructOriginalScreenVector(Memory& m, std::uint32_t vector) {
    // 4cab50 pushes ECX and copies its highest byte, not an arbitrary host byte.
    m.byte(vector, vector >> 24);
    for (auto offset : {4u, 8u, 12u, 16u}) m.set(vector + offset, 0);
}

template<class Memory, class Services>
void destroyOriginalScreenVector(Memory& m, Services& p, std::uint32_t vector) {
    const auto begin = m.get(vector + 4), end = m.get(vector + 8);
    if (begin && ((end - begin) & 0xfffffffcu)) m.set(vector + 8, begin);
    m.set(vector + 16, 0);
    // Even a null pointer is passed. Clear fields after the allocator callback.
    p.free(m.get(vector + 4));
    for (auto offset : {4u, 8u, 12u}) m.set(vector + offset, 0);
}

template<class Memory>
void constructOriginalClass11Screen(Memory& m, std::uint32_t screen) {
    m.set(screen + 8, 0); m.byte(screen + 12, 0);
    m.set(screen + 13, 0); m.byte(screen + 17, 1);
    m.set(screen, 0x5c5df4);
    constructOriginalScreenVector(m, screen + 0x1f);
    for (auto offset : {0x13u, 0x17u, 0x1bu, 0x33u, 0x3bu,
                        0x3fu, 0x37u, 0x43u}) m.set(screen + offset, 0);
    m.set(screen, 0x5c6454); m.set(screen + 4, 11);
    m.set(screen + 0x47, 0);
}

template<class Memory, class Services>
void destroyOriginalClass11Screen(Memory& m, Services& p, std::uint32_t screen) {
    m.set(screen, 0x5c6454);
    destroyOriginalScreenVector(m, p, screen + 0x1f);
    // The original body neither releases screen+47 nor clears surface+28.
}

template<class Memory, class Services>
void prepareOriginalClass11Callback(Memory& m, Services& p, std::uint32_t screen) {
    if (m.get(screen + 0x47)) return;
    const auto callback = p.allocate(12);
    if (callback) {
        m.set(callback + 4, 0x4a8550); m.set(callback + 8, screen);
        m.set(callback, 0x5c6484);
    }
    const auto surface = m.get(screen + 0x13); // Reread after allocation.
    m.set(screen + 0x47, callback); m.set(surface + 0x28, callback);
}

template<class Memory, class Services>
void releaseOriginalClass11Callback(Memory& m, Services& p, std::uint32_t screen) {
    const auto callback = m.get(screen + 0x47);
    if (callback) {
        p.dispose(callback, 1);
        m.set(screen + 0x47, 0);
    }
    // The surface's alias is untouched by this caller. Callee retirement and
    // reachability of any later use must be established independently.
}

template<class Memory, class Services>
std::uint32_t disposeOriginalScreenCallback(Memory& m, Services& p,
                                           std::uint32_t callback, std::uint32_t flags) {
    // 4bc980 -> 4bcc00, then the scalar deleting-destructor low-bit test.
    m.set(callback, 0x5c5c40);
    if (flags & 1) p.free(callback);
    return callback;
}

template<class Memory, class Services>
std::uint32_t invokeOriginalScreenCallback(Memory& m, Services& p,
                                          std::uint32_t callback, std::uint32_t argument) {
    return p.invoke(m.get(callback + 4), m.get(callback + 8), argument);
}
} // namespace mnm::reconstruction
