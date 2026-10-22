#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction {
struct OriginalWorldOwner {
    std::array<unsigned char, 0x244> bytes;
    std::uint32_t get(unsigned at) const {
        std::uint32_t value; std::memcpy(&value, bytes.data() + at, 4); return value;
    }
    void put(unsigned at, std::uint32_t value) { std::memcpy(bytes.data() + at, &value, 4); }
};
struct WorldMaskGeometry {
    std::uint32_t width{}, height{}, pitch{}, area{}, bytes{};
};
// Allocation and release tokens describe a provider boundary. Row object words
// are borrowed values; neither constructor nor destructor visits their objects.
template<class Provider>
void constructOriginalWorldOwner(OriginalWorldOwner& owner,
                                WorldMaskGeometry& geometry, Provider& provider) {
    owner.put(0, 0x5c5c20); owner.put(4, 0xffffffffu);
    owner.put(8, 0); owner.put(12, 16); owner.bytes[16] = 1;
    for (unsigned at = 0xc5; at < 0xf5; at += 4) owner.put(at, 0);
    owner.put(0x235, 12000);
    owner.put(0x229, provider.allocate(432000));
    geometry = {344, 339, 43, 116616, 14577};
    const auto mask = provider.allocate(geometry.bytes);
    owner.put(0x239, mask);
    // Original unconditionally clears this pointer. A null mask is an observed
    // original fault, outside this successful construction domain.
    if (!mask) throw std::invalid_argument("Original World mask allocation would fault");
    provider.clear(mask, geometry.bytes);
    owner.put(0x231, 0); owner.put(0x22d, owner.get(0x229));
    owner.put(0, 0x5c6a54);
    for (unsigned at : {0x21u,0x25u,0x29u,0x2du,0x31u,0x23du,0x35u,
                        0x51u,0x55u,0x41u,0x3du,0x39u,0x4du,0x49u,0x45u,0x59u,
                        0x1f9u,0x1f5u,0x201u,0x1fdu,0x209u,0x205u,0x20du,
                        0x219u,0x21du,0x99u}) owner.put(at, 0);
    for (unsigned at : {0x61u,0x75u,0x6du}) owner.put(at, 1);
    owner.put(0x65, 3); owner.put(0xc1, 25);
}
struct OriginalOwnerRegistry {
    // Ordinals refer to separately owned registry objects; rows contain no
    // host pointers. The receiver's index must be valid or UINT32_MAX.
    std::vector<std::array<std::uint32_t, 3>> objects;
    std::vector<std::uint32_t> entries;
    std::uint32_t count{};
    std::uint32_t selected{0xffffffffu};
    void remove(std::uint32_t index) {
        if (index == 0xffffffffu) return;
        if (count > entries.size() || index >= count) throw std::invalid_argument("World registry index");
        auto& object = objects.at(entries[index]);
        if (selected == object[1]) selected = 0xffffffffu;
        object[2] = 0;
        for (std::size_t at = index; at + 1 < count; ++at) {
            entries[at] = entries[at + 1]; --objects.at(entries[at])[1];
        }
        // The original decrements count but does not clear the unused last
        // pointer. The fixture retains that physical tail separately.
        --count;
        if (selected == index) selected = 0xffffffffu;
        else if (selected != 0xffffffffu && selected >= index) --selected;
    }
};
template<class Provider, class Unregister>
void destroyOriginalWorldOwner(OriginalWorldOwner& owner, Provider& provider,
                              Unregister&& unregister) {
    provider.release(owner.get(0x229)); // Includes the null row allocation case.
    if (owner.get(0x239)) {
        provider.release(owner.get(0x239)); owner.put(0x239, 0);
    }
    owner.put(0, 0x5c5c20);
    unregister(owner.get(4));
    owner.put(4, 0xffffffffu);
    // The row base, cursor and count deliberately retain their original bytes.
}
} // namespace mnm::reconstruction
